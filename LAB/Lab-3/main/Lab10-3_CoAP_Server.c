#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "coap3/coap.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "nvs_flash.h"

#define TAG "COAP_LAB"

/* Wi-Fi settings from Lab 1. */
#define CONFIG_WIFI_SSID "Pornprom"
#define CONFIG_WIFI_PASSWORD "12345678"
#define MAXIMUM_RETRY 5

#define LED_GPIO_PIN GPIO_NUM_2
#define POT_ADC_CHANNEL ADC_CHANNEL_6 /* GPIO 34, ADC1 Channel 6 */

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;
static adc_oneshot_unit_handle_t s_adc1_handle;
static int s_retry_num;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying Wi-Fi (%d/%d)...", s_retry_num,
                     MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected! IP Address: " IPSTR,
                 IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Unable to create Wi-Fi event group");
        return false;
    }

    if (esp_netif_create_default_wifi_sta() == NULL) {
        ESP_LOGE(TAG, "Unable to create Wi-Fi station interface");
        return false;
    }

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    size_t ssid_length = strlen(CONFIG_WIFI_SSID);
    if (ssid_length > sizeof(wifi_config.sta.ssid)) {
        ssid_length = sizeof(wifi_config.sta.ssid);
    }
    memcpy(wifi_config.sta.ssid, CONFIG_WIFI_SSID, ssid_length);

    size_t password_length = strlen(CONFIG_WIFI_PASSWORD);
    if (password_length > sizeof(wifi_config.sta.password)) {
        password_length = sizeof(wifi_config.sta.password);
    }
    memcpy(wifi_config.sta.password, CONFIG_WIFI_PASSWORD, password_length);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Connecting to AP: %s...", CONFIG_WIFI_SSID);

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
        pdFALSE, portMAX_DELAY);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

static void hnd_get_core(coap_resource_t *resource, coap_session_t *session,
                         const coap_pdu_t *request, const coap_string_t *query,
                         coap_pdu_t *response)
{
    static const char core_payload[] =
        "</sensor/pot>;rt=\"sensor\",</actuator/led>;rt=\"actuator\"";

    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data(response, sizeof(core_payload) - 1,
                  (const uint8_t *)core_payload);
    ESP_LOGI(TAG, "Responded GET /.well-known/core");
}

static void hnd_get_pot(coap_resource_t *resource, coap_session_t *session,
                        const coap_pdu_t *request, const coap_string_t *query,
                        coap_pdu_t *response)
{
    int potentiometer_value = 0;
    if (s_adc1_handle != NULL) {
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc1_handle, POT_ADC_CHANNEL,
                                         &potentiometer_value));
    }

    char payload[32];
    int payload_length =
        snprintf(payload, sizeof(payload), "%d", potentiometer_value);
    if (payload_length < 0 || payload_length >= sizeof(payload)) {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
        return;
    }

    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data(response, payload_length, (const uint8_t *)payload);
    ESP_LOGI(TAG, "Responded GET /sensor/pot: %s", payload);
}

static void hnd_put_led(coap_resource_t *resource, coap_session_t *session,
                        const coap_pdu_t *request, const coap_string_t *query,
                        coap_pdu_t *response)
{
    size_t size = 0;
    const uint8_t *data = NULL;
    coap_get_data(request, &size, &data);

    if (size == 0 || data == NULL ||
        (data[0] != '0' && data[0] != '1')) {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
        return;
    }

    int level = data[0] == '1' ? 1 : 0;
    ESP_ERROR_CHECK(gpio_set_level(LED_GPIO_PIN, level));
    ESP_LOGI(TAG, "LED turned %s via CoAP PUT", level ? "ON" : "OFF");
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED);
}

static void coap_server_task(void *pv_parameters)
{
    coap_address_t server_address;
    coap_address_init(&server_address);
    server_address.addr.sin.sin_family = AF_INET;
    server_address.addr.sin.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.addr.sin.sin_port = htons(COAP_DEFAULT_PORT);

    coap_context_t *context = coap_new_context(NULL);
    if (context == NULL) {
        ESP_LOGE(TAG, "coap_new_context() failed");
        vTaskDelete(NULL);
        return;
    }

    coap_endpoint_t *endpoint =
        coap_new_endpoint(context, &server_address, COAP_PROTO_UDP);
    if (endpoint == NULL) {
        ESP_LOGE(TAG, "coap_new_endpoint() failed");
        coap_free_context(context);
        vTaskDelete(NULL);
        return;
    }

    coap_resource_t *core_resource = coap_resource_init(
        coap_make_str_const(".well-known/core"), 0);
    coap_register_handler(core_resource, COAP_REQUEST_GET, hnd_get_core);
    coap_add_resource(context, core_resource);

    coap_resource_t *pot_resource =
        coap_resource_init(coap_make_str_const("sensor/pot"), 0);
    coap_register_handler(pot_resource, COAP_REQUEST_GET, hnd_get_pot);
    coap_add_resource(context, pot_resource);

    coap_resource_t *led_resource =
        coap_resource_init(coap_make_str_const("actuator/led"), 0);
    coap_register_handler(led_resource, COAP_REQUEST_PUT, hnd_put_led);
    coap_add_resource(context, led_resource);

    ESP_LOGI(TAG, "CoAP Server listening on port %d...", COAP_DEFAULT_PORT);

    while (true) {
        int result = coap_io_process(context, 100);
        if (result < 0) {
            ESP_LOGE(TAG, "coap_io_process() failed");
            break;
        }
    }

    coap_free_context(context);
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES ||
        result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    gpio_reset_pin(LED_GPIO_PIN);
    ESP_ERROR_CHECK(gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_INPUT_OUTPUT));

    adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&adc_init_config, &s_adc1_handle));

    adc_oneshot_chan_cfg_t adc_channel_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        s_adc1_handle, POT_ADC_CHANNEL, &adc_channel_config));
    ESP_LOGI(TAG, "ADC initialized on GPIO 34");

    if (wifi_init_sta()) {
        xTaskCreate(coap_server_task, "coap_server", 8192, NULL, 5, NULL);
        ESP_LOGI(TAG, "Ready! Test CoAP with: python test_coap.py");
    } else {
        ESP_LOGE(TAG, "Wi-Fi connection failed");
    }
}
