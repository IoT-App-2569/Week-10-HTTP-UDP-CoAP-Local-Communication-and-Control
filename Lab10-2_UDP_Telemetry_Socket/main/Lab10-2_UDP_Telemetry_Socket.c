#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"


#define TAG "UDP_LAB"

// ---------------------------------------------------------------------------
// Wi-Fi Configuration
// ---------------------------------------------------------------------------
#define CONFIG_WIFI_SSID       "brown"
#define CONFIG_WIFI_PASSWORD   "jjjjjjjj"
#define MAXIMUM_RETRY          5


// ---------------------------------------------------------------------------
// Hardware Configuration
// ---------------------------------------------------------------------------
#define LED_GPIO_PIN           GPIO_NUM_2
#define POT_ADC_CHANNEL        ADC_CHANNEL_6 // GPIO34 (ADC1 Channel 6)


// ---------------------------------------------------------------------------
// UDP Configuration
// ---------------------------------------------------------------------------
#define UDP_CONTROL_PORT       3333
#define UDP_BROADCAST_PORT     3334


// ---------------------------------------------------------------------------
// Potentiometer Configuration
// ---------------------------------------------------------------------------

// 1 = ใช้ค่า Potentiometer จำลองจากซอฟต์แวร์
// 0 = อ่านค่าจาก ADC จริง
#define USE_SIMULATED_POT      1


// ---------------------------------------------------------------------------
// Wi-Fi Power Save
// ---------------------------------------------------------------------------

// 1 = ปิด Wi-Fi Power Save
// 0 = ใช้ค่าเริ่มต้น
#define DISABLE_WIFI_PS        0


// ---------------------------------------------------------------------------
// Global Variables
// ---------------------------------------------------------------------------
static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT     BIT0
#define WIFI_FAIL_BIT          BIT1

static int s_retry_num = 0;

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;


// ===========================================================================
// Wi-Fi Event Handler
// ===========================================================================
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START) {

        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {

        if (s_retry_num < MAXIMUM_RETRY) {

            esp_wifi_connect();

            s_retry_num++;

            ESP_LOGI(
                TAG,
                "Retrying Wi-Fi (%d/%d)...",
                s_retry_num,
                MAXIMUM_RETRY
            );

        } else {

            xEventGroupSetBits(
                s_wifi_event_group,
                WIFI_FAIL_BIT
            );

            ESP_LOGE(
                TAG,
                "Failed to connect to Wi-Fi"
            );
        }

    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(
            TAG,
            "Connected! IP Address: " IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        s_retry_num = 0;

        xEventGroupSetBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );
    }
}


// ===========================================================================
// Initialize Wi-Fi Station
// ===========================================================================
static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    if (s_wifi_event_group == NULL) {

        ESP_LOGE(
            TAG,
            "Failed to create Wi-Fi event group"
        );

        return false;
    }


    // Create default Wi-Fi Station interface
    esp_netif_create_default_wifi_sta();


    // Initialize Wi-Fi
    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    // Register Wi-Fi event handler
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );


    // Register IP event handler
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );


    // Wi-Fi configuration
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };


    // Copy SSID
    size_t ssid_len =
        strlen(CONFIG_WIFI_SSID);

    if (ssid_len > sizeof(wifi_config.sta.ssid)) {
        ssid_len =
            sizeof(wifi_config.sta.ssid);
    }

    memcpy(
        wifi_config.sta.ssid,
        CONFIG_WIFI_SSID,
        ssid_len
    );


    // Copy Password
    size_t pass_len =
        strlen(CONFIG_WIFI_PASSWORD);

    if (pass_len > sizeof(wifi_config.sta.password)) {
        pass_len =
            sizeof(wifi_config.sta.password);
    }

    memcpy(
        wifi_config.sta.password,
        CONFIG_WIFI_PASSWORD,
        pass_len
    );


    // Start Wi-Fi
    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );

    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


#if DISABLE_WIFI_PS

    ESP_ERROR_CHECK(
        esp_wifi_set_ps(WIFI_PS_NONE)
    );

    ESP_LOGI(
        TAG,
        "Wi-Fi power save: OFF"
    );

#endif


    ESP_LOGI(
        TAG,
        "Connecting to AP: %s...",
        CONFIG_WIFI_SSID
    );


    // Wait for Wi-Fi connection
    EventBits_t bits =
        xEventGroupWaitBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT |
            WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );


    return (bits & WIFI_CONNECTED_BIT) != 0;
}


// ===========================================================================
// Task 1: UDP Control Server
// Port 3333
// ===========================================================================
static void udp_control_server_task(void *pvParameters)
{
    char rx_buffer[128];

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_addr_len =
        sizeof(client_addr);


    // Create UDP socket
    int sock =
        socket(
            AF_INET,
            SOCK_DGRAM,
            IPPROTO_IP
        );

    if (sock < 0) {

        ESP_LOGE(
            TAG,
            "Unable to create UDP control socket"
        );

        vTaskDelete(NULL);
        return;
    }


    // Configure server address
    memset(
        &server_addr,
        0,
        sizeof(server_addr)
    );

    server_addr.sin_addr.s_addr =
        htonl(INADDR_ANY);

    server_addr.sin_family =
        AF_INET;

    server_addr.sin_port =
        htons(UDP_CONTROL_PORT);


    // Bind socket
    if (bind(
            sock,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) < 0) {

        ESP_LOGE(
            TAG,
            "Unable to bind UDP control socket"
        );

        close(sock);

        vTaskDelete(NULL);
        return;
    }


    ESP_LOGI(
        TAG,
        "Control Server listening on UDP Port %d...",
        UDP_CONTROL_PORT
    );


    // Receive loop
    while (1) {

        int len =
            recvfrom(
                sock,
                rx_buffer,
                sizeof(rx_buffer) - 1,
                0,
                (struct sockaddr *)&client_addr,
                &client_addr_len
            );


        if (len > 0) {

            rx_buffer[len] = '\0';


            // LED ON
            if (strcmp(
                    rx_buffer,
                    "LED_ON"
                ) == 0) {

                gpio_set_level(
                    LED_GPIO_PIN,
                    1
                );

                sendto(
                    sock,
                    "ACK:LED_ON",
                    10,
                    0,
                    (struct sockaddr *)&client_addr,
                    client_addr_len
                );


            // LED OFF
            } else if (strcmp(
                           rx_buffer,
                           "LED_OFF"
                       ) == 0) {

                gpio_set_level(
                    LED_GPIO_PIN,
                    0
                );

                sendto(
                    sock,
                    "ACK:LED_OFF",
                    11,
                    0,
                    (struct sockaddr *)&client_addr,
                    client_addr_len
                );


            // Unknown command
            } else {

                sendto(
                    sock,
                    "ACK:UNKNOWN",
                    11,
                    0,
                    (struct sockaddr *)&client_addr,
                    client_addr_len
                );
            }
        }
    }
}


// ===========================================================================
// Task 2: UDP Telemetry Broadcast
// Port 3334
// ===========================================================================
static void udp_telemetry_broadcast_task(void *pvParameters)
{
    struct sockaddr_in dest_addr;

    memset(
        &dest_addr,
        0,
        sizeof(dest_addr)
    );


    // Broadcast address
    dest_addr.sin_addr.s_addr =
        inet_addr("255.255.255.255");

    dest_addr.sin_family =
        AF_INET;

    dest_addr.sin_port =
        htons(UDP_BROADCAST_PORT);


    // Create UDP socket
    int sock =
        socket(
            AF_INET,
            SOCK_DGRAM,
            IPPROTO_IP
        );


    if (sock < 0) {

        ESP_LOGE(
            TAG,
            "Unable to create UDP broadcast socket"
        );

        vTaskDelete(NULL);
        return;
    }


    // Enable broadcast
    int broadcast_enable = 1;

    setsockopt(
        sock,
        SOL_SOCKET,
        SO_BROADCAST,
        &broadcast_enable,
        sizeof(broadcast_enable)
    );


    uint32_t seq_no = 0;

    char tx_buffer[64];


    ESP_LOGI(
        TAG,
        "Broadcasting Telemetry to Port %d (10 Hz)...",
        UDP_BROADCAST_PORT
    );


    // Telemetry loop
    while (1) {

        int pot_val = 0;


#if USE_SIMULATED_POT

        // ---------------------------------------------------------------
        // Simulated Potentiometer
        // Sine wave period = 20 seconds
        // Noise = ±20
        // Range = 0-4095
        // ---------------------------------------------------------------

        float t =
            seq_no * 0.1f;

        float v =
            2048.0f +
            1800.0f *
            sinf(
                2.0f *
                (float)M_PI *
                t /
                20.0f
            );


        int noise =
            (int)(esp_random() % 41) - 20;


        pot_val =
            (int)v + noise;


        if (pot_val < 0) {
            pot_val = 0;
        }

        if (pot_val > 4095) {
            pot_val = 4095;
        }


#else

        // ---------------------------------------------------------------
        // Read real ADC
        // ---------------------------------------------------------------

        if (s_adc1_handle != NULL) {

            adc_oneshot_read(
                s_adc1_handle,
                POT_ADC_CHANNEL,
                &pot_val
            );
        }

#endif


        // ---------------------------------------------------------------
        // Create telemetry packet
        // ---------------------------------------------------------------

        snprintf(
            tx_buffer,
            sizeof(tx_buffer),
            "SEQ:%lu,POT:%d\n",
            (unsigned long)seq_no++,
            pot_val
        );


        // Broadcast telemetry
        sendto(
            sock,
            tx_buffer,
            strlen(tx_buffer),
            0,
            (struct sockaddr *)&dest_addr,
            sizeof(dest_addr)
        );


        // 10 Hz = every 100 ms
        vTaskDelay(
            pdMS_TO_TICKS(100)
        );
    }
}


// ===========================================================================
// app_main
// ===========================================================================
void app_main(void)
{
    // -----------------------------------------------------------------------
    // 1. Initialize NVS
    // -----------------------------------------------------------------------
    esp_err_t ret =
        nvs_flash_init();


    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );

        ret =
            nvs_flash_init();
    }


    ESP_ERROR_CHECK(ret);


    // -----------------------------------------------------------------------
    // 2. Initialize Network Interface
    // -----------------------------------------------------------------------
    ESP_ERROR_CHECK(
        esp_netif_init()
    );


    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );


    // -----------------------------------------------------------------------
    // 3. Setup Hardware
    // -----------------------------------------------------------------------

    // LED GPIO2
    gpio_reset_pin(
        LED_GPIO_PIN
    );

    gpio_set_direction(
        LED_GPIO_PIN,
        GPIO_MODE_INPUT_OUTPUT
    );

    gpio_set_level(
        LED_GPIO_PIN,
        0
    );


    // -----------------------------------------------------------------------
    // ADC1 - GPIO34
    // -----------------------------------------------------------------------

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1
    };


    if (adc_oneshot_new_unit(
            &init_config1,
            &s_adc1_handle
        ) == ESP_OK) {

        adc_oneshot_chan_cfg_t chan_config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_12,
        };


        ESP_ERROR_CHECK(
            adc_oneshot_config_channel(
                s_adc1_handle,
                POT_ADC_CHANNEL,
                &chan_config
            )
        );


        ESP_LOGI(
            TAG,
            "ADC Initialized on GPIO 34"
        );

    } else {

        ESP_LOGE(
            TAG,
            "Failed to initialize ADC"
        );
    }


    // -----------------------------------------------------------------------
    // 4. Connect Wi-Fi
    // -----------------------------------------------------------------------

    if (wifi_init_sta()) {


        // -------------------------------------------------------------------
        // 5. Measure Baseline Heap
        // -------------------------------------------------------------------

        size_t baseline_heap =
            esp_get_free_heap_size();


        ESP_LOGI(
            "MEM",
            "Baseline free heap: %lu bytes (%lu KB)",
            (unsigned long)baseline_heap,
            (unsigned long)(baseline_heap / 1024)
        );


        // -------------------------------------------------------------------
        // 6. Create UDP Tasks
        // -------------------------------------------------------------------

        BaseType_t control_result =
            xTaskCreate(
                udp_control_server_task,
                "udp_ctrl_task",
                4096,
                NULL,
                5,
                NULL
            );


        BaseType_t telemetry_result =
            xTaskCreate(
                udp_telemetry_broadcast_task,
                "udp_bcast_task",
                4096,
                NULL,
                4,
                NULL
            );


        if (control_result == pdPASS &&
            telemetry_result == pdPASS) {

            ESP_LOGI(
                TAG,
                "All UDP Tasks started!"
            );


            // ---------------------------------------------------------------
            // 7. Wait 3 seconds
            // ---------------------------------------------------------------

            vTaskDelay(
                pdMS_TO_TICKS(3000)
            );


            // ---------------------------------------------------------------
            // 8. Measure Heap after UDP Tasks
            // ---------------------------------------------------------------

            size_t after_server_heap =
                esp_get_free_heap_size();


            size_t minimum_heap =
                esp_get_minimum_free_heap_size();


            ESP_LOGI(
                "MEM",
                "After UDP server free heap: %lu bytes (%lu KB)",
                (unsigned long)after_server_heap,
                (unsigned long)(after_server_heap / 1024)
            );


            ESP_LOGI(
                "MEM",
                "Minimum free heap ever: %lu bytes",
                (unsigned long)minimum_heap
            );


            // ---------------------------------------------------------------
            // 9. Calculate Approximate Heap Usage
            // ---------------------------------------------------------------

            if (baseline_heap > after_server_heap) {

                size_t heap_used =
                    baseline_heap -
                    after_server_heap;


                ESP_LOGI(
                    "MEM",
                    "Approx. heap used by UDP Tasks: %lu bytes (%lu KB)",
                    (unsigned long)heap_used,
                    (unsigned long)(heap_used / 1024)
                );

            } else {

                ESP_LOGI(
                    "MEM",
                    "Heap usage difference: 0 bytes"
                );
            }


        } else {

            ESP_LOGE(
                TAG,
                "Failed to create one or more UDP Tasks"
            );
        }


    } else {

        ESP_LOGE(
            TAG,
            "Wi-Fi connection failed."
        );
    }
}