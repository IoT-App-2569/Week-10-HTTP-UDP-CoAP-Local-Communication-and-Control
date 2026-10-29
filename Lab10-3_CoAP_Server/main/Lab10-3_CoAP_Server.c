
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#include "coap3/coap.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#define TAG "COAP_LAB"

#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

#define LED_GPIO_PIN    GPIO_NUM_2
#define POT_ADC_CHANNEL ADC_CHANNEL_6

#define MAXIMUM_RETRY   5
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static adc_oneshot_unit_handle_t s_adc_handle = NULL;

/* ---------- Wi-Fi ---------- */

static void wifi_event_handler(
    void *arg,
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
            s_retry_num++;
            esp_wifi_connect();
            ESP_LOGW(TAG, "Retry Wi-Fi: %d", s_retry_num);
        } else {
            xEventGroupSetBits(
                s_wifi_event_group, WIFI_FAIL_BIT);
        }

    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "ESP32 IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));

        s_retry_num = 0;

        xEventGroupSetBits(
            s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    if (s_wifi_event_group == NULL) {
        return false;
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {0};

    snprintf((char *)wifi_config.sta.ssid,
             sizeof(wifi_config.sta.ssid),
             "%s", WIFI_SSID);

    snprintf((char *)wifi_config.sta.password,
             sizeof(wifi_config.sta.password),
             "%s", WIFI_PASSWORD);

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(
        WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE, portMAX_DELAY);

    return (bits & WIFI_CONNECTED_BIT) != 0;
}

/* ---------- CoAP GET /sensor/pot ---------- */

static void hnd_get_pot(
    coap_resource_t *resource,
    coap_session_t *session,
    const coap_pdu_t *request,
    const coap_string_t *query,
    coap_pdu_t *response)
{
    int pot_value = 0;

    if (s_adc_handle == NULL ||
        adc_oneshot_read(s_adc_handle,
                         POT_ADC_CHANNEL,
                         &pot_value) != ESP_OK) {
        coap_pdu_set_code(
            response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
        return;
    }

    char payload[16];
    int len = snprintf(payload, sizeof(payload),
                       "%d", pot_value);

    coap_pdu_set_code(
        response, COAP_RESPONSE_CODE_CONTENT);

    if (len > 0) {
        coap_add_data(response, (size_t)len,
                      (const uint8_t *)payload);
    }

    ESP_LOGI(TAG, "GET /sensor/pot = %d", pot_value);
}

/* ---------- CoAP PUT /actuator/led ---------- */

static void hnd_put_led(
    coap_resource_t *resource,
    coap_session_t *session,
    const coap_pdu_t *request,
    const coap_string_t *query,
    coap_pdu_t *response)
{
    size_t size = 0;
    const uint8_t *data = NULL;

    if (!coap_get_data(request, &size, &data) ||
        size != 1 || data == NULL ||
        (data[0] != '0' && data[0] != '1')) {

        coap_pdu_set_code(
            response, COAP_RESPONSE_CODE_BAD_REQUEST);
        return;
    }

    int level = (data[0] == '1') ? 1 : 0;

    esp_err_t err = gpio_set_level(LED_GPIO_PIN, level);

    if (err != ESP_OK) {
        coap_pdu_set_code(
            response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
        return;
    }

    coap_pdu_set_code(
        response, COAP_RESPONSE_CODE_CHANGED);

    ESP_LOGI(TAG, "LED %s",
             level ? "ON" : "OFF");
}

/* ---------- GET /.well-known/core ---------- */

static void hnd_get_core(
    coap_resource_t *resource,
    coap_session_t *session,
    const coap_pdu_t *request,
    const coap_string_t *query,
    coap_pdu_t *response)
{
    static const char links[] =
        "</sensor/pot>;rt=\"sensor\";"
        "title=\"Potentiometer\","
        "</actuator/led>;rt=\"actuator\";"
        "title=\"LED\"";

    uint8_t format_buf[4];

    size_t format_len = coap_encode_var_safe(
        format_buf, sizeof(format_buf),
        COAP_MEDIATYPE_APPLICATION_LINK_FORMAT);

    coap_pdu_set_code(
        response, COAP_RESPONSE_CODE_CONTENT);

    coap_add_option(response,
                    COAP_OPTION_CONTENT_FORMAT,
                    format_len, format_buf);

    coap_add_data(response, strlen(links),
                  (const uint8_t *)links);
}

/* ---------- CoAP Server Task ---------- */

static void coap_server_task(void *pvParameters)
{
    coap_startup();

    coap_context_t *ctx = coap_new_context(NULL);

    if (ctx == NULL) {
        ESP_LOGE(TAG, "Cannot create CoAP context");
        coap_cleanup();
        vTaskDelete(NULL);
        return;
    }

    coap_address_t addr;
    coap_address_init(&addr);

    addr.addr.sin.sin_family = AF_INET;
    addr.addr.sin.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.addr.sin.sin_port = htons(COAP_DEFAULT_PORT);

    if (coap_new_endpoint(ctx, &addr, COAP_PROTO_UDP) == NULL) {
        ESP_LOGE(TAG, "Cannot create CoAP endpoint");
        coap_free_context(ctx);
        coap_cleanup();
        vTaskDelete(NULL);
        return;
    }

    coap_resource_t *r_core =
        coap_resource_init(
            coap_make_str_const(".well-known/core"), 0);

    coap_register_handler(
        r_core, COAP_REQUEST_GET, hnd_get_core);
    coap_add_resource(ctx, r_core);

    coap_resource_t *r_pot =
        coap_resource_init(
            coap_make_str_const("sensor/pot"), 0);

    coap_register_handler(
        r_pot, COAP_REQUEST_GET, hnd_get_pot);
    coap_add_resource(ctx, r_pot);

    coap_resource_t *r_led =
        coap_resource_init(
            coap_make_str_const("actuator/led"), 0);

    coap_register_handler(
        r_led, COAP_REQUEST_PUT, hnd_put_led);
    coap_add_resource(ctx, r_led);

    ESP_LOGI(TAG, "CoAP Server listening on UDP %d",
             COAP_DEFAULT_PORT);

    while (1) {
        int result = coap_io_process(ctx, 100);

        if (result < 0) {
            ESP_LOGW(TAG, "CoAP I/O processing error");
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

/* ---------- app_main ---------- */

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* LED */
    ESP_ERROR_CHECK(gpio_reset_pin(LED_GPIO_PIN));
    ESP_ERROR_CHECK(gpio_set_direction(
        LED_GPIO_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(LED_GPIO_PIN, 0));

    /* Potentiometer: GPIO34 / ADC1 Channel 6 */
    adc_oneshot_unit_init_cfg_t adc_cfg = {
        .unit_id = ADC_UNIT_1
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(
        &adc_cfg, &s_adc_handle));

    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        s_adc_handle, POT_ADC_CHANNEL, &channel_cfg));

    /* Connect Wi-Fi */
    if (!wifi_init_sta()) {
        ESP_LOGE(TAG, "Wi-Fi connection failed");
        return;
    }

    /* Start CoAP */
    BaseType_t created = xTaskCreate(
        coap_server_task,
        "coap_server",
        8192,
        NULL,
        5,
        NULL);

    if (created != pdPASS) {
        ESP_LOGE(TAG, "Cannot create CoAP task");
    }
}
