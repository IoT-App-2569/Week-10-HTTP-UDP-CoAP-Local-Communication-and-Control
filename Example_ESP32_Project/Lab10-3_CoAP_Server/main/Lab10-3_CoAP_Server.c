
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "coap3/coap.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#include "wifi_config.h"

#define TAG "COAP_LAB"

#define MAXIMUM_RETRY         5

#define LED_GPIO_PIN          GPIO_NUM_2
#define POT_ADC_CHANNEL       ADC_CHANNEL_6  // GPIO34 (ADC1 Channel 6)

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1

static int s_retry_num = 0;
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;


// =====================================================
// Wi-Fi Event Handler
// =====================================================

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

            ESP_LOGI(TAG, "Retrying Wi-Fi (%d/%d)...",
                     s_retry_num, MAXIMUM_RETRY);

        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }

    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "Connected! IP Address: " IPSTR,
                 IP2STR(&event->ip_info.ip));

        s_retry_num = 0;

        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


// =====================================================
// Wi-Fi Initialization
// =====================================================

static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create Wi-Fi event group");
        return false;
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    size_t ssid_len = strlen(CONFIG_WIFI_SSID);

    if (ssid_len > sizeof(wifi_config.sta.ssid)) {
        ssid_len = sizeof(wifi_config.sta.ssid);
    }

    memcpy(wifi_config.sta.ssid,
           CONFIG_WIFI_SSID,
           ssid_len);

    size_t pass_len = strlen(CONFIG_WIFI_PASSWORD);

    if (pass_len > sizeof(wifi_config.sta.password)) {
        pass_len = sizeof(wifi_config.sta.password);
    }

    memcpy(wifi_config.sta.password,
           CONFIG_WIFI_PASSWORD,
           pass_len);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config)
    );

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    return (bits & WIFI_CONNECTED_BIT) != 0;
}


// =====================================================
// Handler: GET /sensor/pot
// =====================================================

static void hnd_get_pot(coap_resource_t *resource,
                        coap_session_t *session,
                        const coap_pdu_t *request,
                        const coap_string_t *query,
                        coap_pdu_t *response)
{
    int pot_val = 0;

    if (s_adc1_handle != NULL) {

        esp_err_t ret = adc_oneshot_read(
            s_adc1_handle,
            POT_ADC_CHANNEL,
            &pot_val
        );

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "ADC read failed");
        }
    }

    char pot_str[32];

    snprintf(pot_str, sizeof(pot_str), "%d", pot_val);

    coap_pdu_set_code(
        response,
        COAP_RESPONSE_CODE_CONTENT
    );

    coap_add_data(
        response,
        strlen(pot_str),
        (const uint8_t *)pot_str
    );

    ESP_LOGI(TAG, "GET /sensor/pot -> %s", pot_str);
}


// =====================================================
// Handler: PUT /actuator/led
// =====================================================

static void hnd_put_led(coap_resource_t *resource,
                        coap_session_t *session,
                        const coap_pdu_t *request,
                        const coap_string_t *query,
                        coap_pdu_t *response)
{
    size_t size = 0;
    const uint8_t *data = NULL;

    if (coap_get_data(request, &size, &data) && size > 0) {

        if (data[0] == '1') {

            gpio_set_level(LED_GPIO_PIN, 1);

            ESP_LOGI(TAG, "LED turned ON via CoAP PUT");

        } else if (data[0] == '0') {

            gpio_set_level(LED_GPIO_PIN, 0);

            ESP_LOGI(TAG, "LED turned OFF via CoAP PUT");

        } else {

            ESP_LOGW(TAG, "Invalid LED payload");
            coap_pdu_set_code(
                response,
                COAP_RESPONSE_CODE_BAD_REQUEST
            );
            return;
        }

    } else {

        ESP_LOGW(TAG, "Empty LED payload");

        coap_pdu_set_code(
            response,
            COAP_RESPONSE_CODE_BAD_REQUEST
        );

        return;
    }

    coap_pdu_set_code(
        response,
        COAP_RESPONSE_CODE_CHANGED
    );
}


// =====================================================
// CoAP Server Task
// =====================================================

static void coap_server_task(void *pvParameters)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;

    // Initialize libcoap library
    coap_startup();

    // Initialize server address
    coap_address_init(&serv_addr);

    serv_addr.addr.sin.sin_family = AF_INET;

    serv_addr.addr.sin.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_addr.addr.sin.sin_port = htons(COAP_DEFAULT_PORT);


    // Create CoAP context
    ctx = coap_new_context(NULL);

    if (ctx == NULL) {

        ESP_LOGE(TAG, "coap_new_context() failed");

        vTaskDelete(NULL);
        return;
    }


    // Create CoAP UDP endpoint
    coap_endpoint_t *ep = coap_new_endpoint(
        ctx,
        &serv_addr,
        COAP_PROTO_UDP
    );

    if (ep == NULL) {

        ESP_LOGE(TAG, "coap_new_endpoint() failed");

        coap_free_context(ctx);

        vTaskDelete(NULL);
        return;
    }


    // -------------------------------------------------
    // Resource 1: GET /sensor/pot
    // -------------------------------------------------

    coap_str_const_t *r_pot_uri =
        coap_make_str_const("sensor/pot");

    coap_resource_t *r_pot =
        coap_resource_init(r_pot_uri, 0);

    if (r_pot == NULL) {

        ESP_LOGE(TAG, "Create sensor/pot resource failed");

        coap_free_context(ctx);

        vTaskDelete(NULL);
        return;
    }

    coap_register_handler(
        r_pot,
        COAP_REQUEST_GET,
        hnd_get_pot
    );

    coap_add_resource(ctx, r_pot);


    // -------------------------------------------------
    // Resource 2: PUT /actuator/led
    // -------------------------------------------------

    coap_str_const_t *r_led_uri =
        coap_make_str_const("actuator/led");

    coap_resource_t *r_led =
        coap_resource_init(r_led_uri, 0);

    if (r_led == NULL) {

        ESP_LOGE(TAG, "Create actuator/led resource failed");

        coap_free_context(ctx);

        vTaskDelete(NULL);
        return;
    }

    coap_register_handler(
        r_led,
        COAP_REQUEST_PUT,
        hnd_put_led
    );

    coap_add_resource(ctx, r_led);


    ESP_LOGI(TAG, "CoAP Server listening on port %d...",
             COAP_DEFAULT_PORT);


    // -------------------------------------------------
    // Process CoAP requests continuously
    // -------------------------------------------------

    while (1) {

        int result = coap_io_process(ctx, 100);

        if (result < 0) {

            ESP_LOGE(TAG, "coap_io_process() failed");

            break;
        }
    }


    // Release CoAP context
    coap_free_context(ctx);

    vTaskDelete(NULL);
}


// =====================================================
// Main Application
// =====================================================

void app_main(void)
{
    // 1. Initialize NVS

    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());

        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);


    // 2. Initialize Network Interface and Event Loop

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());


    // 3. Initialize LED GPIO

    gpio_reset_pin(LED_GPIO_PIN);

    gpio_set_direction(
        LED_GPIO_PIN,
        GPIO_MODE_INPUT_OUTPUT
    );


    // 4. Initialize ADC

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

        adc_oneshot_config_channel(
            s_adc1_handle,
            POT_ADC_CHANNEL,
            &chan_config
        );

        ESP_LOGI(TAG, "ADC Initialized on GPIO 34");

    } else {

        ESP_LOGE(TAG, "ADC Initialization failed");
    }


    // 5. Connect Wi-Fi

    if (wifi_init_sta()) {

        // 6. Create CoAP Server Task

        BaseType_t task_result = xTaskCreate(
            coap_server_task,
            "coap_server",
            8192,
            NULL,
            5,
            NULL
        );

        if (task_result == pdPASS) {

            ESP_LOGI(TAG, "Ready! Test CoAP with: python test_coap.py");

        } else {

            ESP_LOGE(TAG, "Failed to create CoAP Server Task");
        }

    } else {

        ESP_LOGE(TAG, "Wi-Fi connection failed.");
    }
}
