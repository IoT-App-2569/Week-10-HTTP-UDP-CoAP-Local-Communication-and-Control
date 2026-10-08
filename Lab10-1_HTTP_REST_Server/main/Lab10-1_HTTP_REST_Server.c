#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "mdns.h"
#include "esp_adc/adc_oneshot.h"

#define TAG "HTTP_REST_LAB"

// ---------------------------------------------------------------------------
// Wi-Fi Configuration
// ---------------------------------------------------------------------------
#define CONFIG_WIFI_SSID       "brown"
#define CONFIG_WIFI_PASSWORD   "jjjjjjjj"
#define MAXIMUM_RETRY         5

// ---------------------------------------------------------------------------
// Hardware Configuration
// ---------------------------------------------------------------------------
#define LED_GPIO_PIN          GPIO_NUM_2
#define POT_ADC_CHANNEL       ADC_CHANNEL_6 // GPIO34 (ADC1 Channel 6)

// ---------------------------------------------------------------------------
// Wi-Fi Event Group
// ---------------------------------------------------------------------------
static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1

static int s_retry_num = 0;

// ---------------------------------------------------------------------------
// Global Handles
// ---------------------------------------------------------------------------
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
static httpd_handle_t s_http_server = NULL;


// ===========================================================================
// Wi-Fi Event Handler
// ===========================================================================
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {

        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {

        wifi_event_sta_disconnected_t *d =
            (wifi_event_sta_disconnected_t *)event_data;

        ESP_LOGW(TAG, "Disconnected, reason=%d", d->reason);

        if (s_retry_num < MAXIMUM_RETRY) {

            esp_wifi_connect();
            s_retry_num++;

            ESP_LOGI(TAG,
                     "Retrying Wi-Fi connection (%d/%d)...",
                     s_retry_num,
                     MAXIMUM_RETRY);

        } else {

            xEventGroupSetBits(
                s_wifi_event_group,
                WIFI_FAIL_BIT
            );

            ESP_LOGE(TAG, "Failed to connect to Wi-Fi");
        }

    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG,
                 "Wi-Fi Connected! IP Address: " IPSTR,
                 IP2STR(&event->ip_info.ip));

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
        ESP_LOGE(TAG, "Failed to create Wi-Fi event group");
        return false;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            &instance_any_id
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            &instance_got_ip
        )
    );

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    // Copy SSID
    size_t ssid_len = strlen(CONFIG_WIFI_SSID);

    if (ssid_len > sizeof(wifi_config.sta.ssid)) {
        ssid_len = sizeof(wifi_config.sta.ssid);
    }

    memcpy(
        wifi_config.sta.ssid,
        CONFIG_WIFI_SSID,
        ssid_len
    );

    // Copy Password
    size_t pass_len = strlen(CONFIG_WIFI_PASSWORD);

    if (pass_len > sizeof(wifi_config.sta.password)) {
        pass_len = sizeof(wifi_config.sta.password);
    }

    memcpy(
        wifi_config.sta.password,
        CONFIG_WIFI_PASSWORD,
        pass_len
    );

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

    ESP_LOGI(
        TAG,
        "Connecting to SSID: %s ...",
        CONFIG_WIFI_SSID
    );

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    if (bits & WIFI_CONNECTED_BIT) {

        ESP_LOGI(
            TAG,
            "Connected to AP successfully!"
        );

        return true;

    } else {

        ESP_LOGE(
            TAG,
            "Failed to connect to AP"
        );

        return false;
    }
}


// ===========================================================================
// Initialize mDNS
// ===========================================================================
static void initialise_mdns(void)
{
    ESP_ERROR_CHECK(
        mdns_init()
    );

    ESP_ERROR_CHECK(
        mdns_hostname_set("esp32-node")
    );

    ESP_ERROR_CHECK(
        mdns_instance_name_set(
            "ESP32 RESTful Controller"
        )
    );

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"role", "actuator"}
    };

    ESP_ERROR_CHECK(
        mdns_service_add(
            "ESP32-WebControl",
            "_http",
            "_tcp",
            80,
            serviceTxtData,
            2
        )
    );

    ESP_LOGI(
        TAG,
        "mDNS initialized! Hostname: http://esp32-node.local"
    );
}


// ===========================================================================
// GET /api/status
// อ่านค่า Potentiometer และสถานะระบบ
// ===========================================================================
static esp_err_t status_get_handler(httpd_req_t *req)
{
    int pot_val = 0;

    if (s_adc1_handle != NULL) {

        adc_oneshot_read(
            s_adc1_handle,
            POT_ADC_CHANNEL,
            &pot_val
        );
    }

    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    cJSON_AddNumberToObject(
        root,
        "pot_raw",
        pot_val
    );

    cJSON_AddNumberToObject(
        root,
        "free_heap",
        esp_get_free_heap_size()
    );

    cJSON_AddBoolToObject(
        root,
        "led",
        gpio_get_level(LED_GPIO_PIN)
    );

    const char *resp =
        cJSON_PrintUnformatted(root);

    if (resp == NULL) {
        cJSON_Delete(root);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(
        req,
        "application/json"
    );

    httpd_resp_send(
        req,
        resp,
        strlen(resp)
    );

    cJSON_free((void *)resp);
    cJSON_Delete(root);

    return ESP_OK;
}


// ===========================================================================
// POST /api/led
// ควบคุม LED
// ===========================================================================
static esp_err_t led_post_handler(httpd_req_t *req)
{
    char buf[128];

    int ret = httpd_req_recv(
        req,
        buf,
        sizeof(buf) - 1
    );

    if (ret <= 0) {

        httpd_resp_send_500(req);

        return ESP_FAIL;
    }

    buf[ret] = '\0';

    cJSON *root = cJSON_Parse(buf);

    if (root != NULL) {

        cJSON *state =
            cJSON_GetObjectItem(
                root,
                "state"
            );

        if (cJSON_IsBool(state)) {

            bool led_on =
                cJSON_IsTrue(state);

            gpio_set_level(
                LED_GPIO_PIN,
                led_on ? 1 : 0
            );

            ESP_LOGI(
                TAG,
                "LED (GPIO %d) switched to: %s",
                LED_GPIO_PIN,
                led_on ? "ON" : "OFF"
            );
        }

        cJSON_Delete(root);

    } else {

        ESP_LOGW(
            TAG,
            "Invalid JSON received"
        );
    }

    const char *resp =
        "{\"result\":\"success\"}";

    httpd_resp_set_type(
        req,
        "application/json"
    );

    httpd_resp_send(
        req,
        resp,
        strlen(resp)
    );

    return ESP_OK;
}


// ===========================================================================
// Start HTTP Web Server
// ===========================================================================
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config =
        HTTPD_DEFAULT_CONFIG();

    config.lru_purge_enable = true;

    httpd_uri_t uri_get_status = {
        .uri      = "/api/status",
        .method   = HTTP_GET,
        .handler  = status_get_handler,
        .user_ctx = NULL
    };

    httpd_uri_t uri_post_led = {
        .uri      = "/api/led",
        .method   = HTTP_POST,
        .handler  = led_post_handler,
        .user_ctx = NULL
    };

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {

        httpd_register_uri_handler(
            server,
            &uri_get_status
        );

        httpd_register_uri_handler(
            server,
            &uri_post_led
        );

        ESP_LOGI(
            TAG,
            "HTTP Server started on port %d",
            config.server_port
        );

        return server;
    }

    ESP_LOGE(
        TAG,
        "Failed to start HTTP server!"
    );

    return NULL;
}


// ===========================================================================
// app_main
// ===========================================================================
void app_main(void)
{
    // -----------------------------------------------------------------------
    // 1. Initialize NVS
    // -----------------------------------------------------------------------
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );

        ret = nvs_flash_init();
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

    esp_netif_create_default_wifi_sta();


    // -----------------------------------------------------------------------
    // 3. Setup GPIO 2 for LED
    // -----------------------------------------------------------------------
    gpio_reset_pin(LED_GPIO_PIN);

    gpio_set_direction(
        LED_GPIO_PIN,
        GPIO_MODE_INPUT_OUTPUT
    );

    gpio_set_level(
        LED_GPIO_PIN,
        0
    );


    // -----------------------------------------------------------------------
    // 4. Setup ADC1 for Potentiometer
    // GPIO34 = ADC1 Channel 6
    // -----------------------------------------------------------------------
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
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
            "ADC Initialized on GPIO 34 (Channel 6)"
        );

    } else {

        ESP_LOGE(
            TAG,
            "Failed to initialize ADC"
        );
    }


    // -----------------------------------------------------------------------
    // 5. Connect Wi-Fi
    // -----------------------------------------------------------------------
    if (wifi_init_sta()) {

        // -------------------------------------------------------------------
        // 6. Measure Baseline Heap
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
        // 7. Initialize mDNS
        // -------------------------------------------------------------------
        initialise_mdns();


        // -------------------------------------------------------------------
        // 8. Start HTTP RESTful Server
        // -------------------------------------------------------------------
        s_http_server =
            start_webserver();

        if (s_http_server != NULL) {

            ESP_LOGI(
                TAG,
                "Ready! Test with: curl http://esp32-node.local/api/status"
            );

            // ---------------------------------------------------------------
            // Wait 3 seconds before measuring again
            // ---------------------------------------------------------------
            vTaskDelay(
                pdMS_TO_TICKS(3000)
            );


            // ---------------------------------------------------------------
            // 9. Measure Heap after HTTP Server
            // ---------------------------------------------------------------
            size_t after_server_heap =
                esp_get_free_heap_size();

            size_t minimum_heap =
                esp_get_minimum_free_heap_size();

            ESP_LOGI(
                "MEM",
                "After HTTP server free heap: %lu bytes (%lu KB)",
                (unsigned long)after_server_heap,
                (unsigned long)(after_server_heap / 1024)
            );

            ESP_LOGI(
                "MEM",
                "Minimum free heap ever: %lu bytes",
                (unsigned long)minimum_heap
            );


            // ---------------------------------------------------------------
            // 10. Calculate approximate heap usage
            // ---------------------------------------------------------------
            if (baseline_heap > after_server_heap) {

                size_t heap_used =
                    baseline_heap - after_server_heap;

                ESP_LOGI(
                    "MEM",
                    "Approx. heap used by HTTP + mDNS: %lu bytes (%lu KB)",
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
                "HTTP Server could not be started"
            );
        }

    } else {

        ESP_LOGE(
            TAG,
            "Cannot start server due to Wi-Fi connection failure."
        );
    }
}