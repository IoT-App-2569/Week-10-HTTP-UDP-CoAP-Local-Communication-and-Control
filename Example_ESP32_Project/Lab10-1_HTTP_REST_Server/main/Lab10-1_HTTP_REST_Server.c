
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_http_server.h"
#include "cJSON.h"
#include "mdns.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

// ==========================================
// 1. CONFIGURATION
// ==========================================

#define TAG "HTTP_REST_LAB"

// แก้ไขชื่อและรหัสผ่าน Wi-Fi ตรงนี้
#define CONFIG_WIFI_SSID      "ROOM52_D203_2.4GHz"
#define CONFIG_WIFI_PASSWORD  "0948971563"

#define MAXIMUM_RETRY         5

#define LED_GPIO_PIN          GPIO_NUM_2
#define POT_ADC_CHANNEL       ADC_CHANNEL_6

#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1

#define MAX_JSON_SIZE         128

// ==========================================
// 2. GLOBAL VARIABLES
// ==========================================

static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;

static httpd_handle_t s_http_server = NULL;

static bool s_led_state = false;

// ==========================================
// 3. WIFI EVENT HANDLER
// ==========================================

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START) {

        esp_wifi_connect();

    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {

        if (s_retry_num < MAXIMUM_RETRY) {

            esp_wifi_connect();
            s_retry_num++;

            ESP_LOGI(
                TAG,
                "Retrying Wi-Fi connection (%d/%d)",
                s_retry_num,
                MAXIMUM_RETRY
            );

        }
        else {

            xEventGroupSetBits(
                s_wifi_event_group,
                WIFI_FAIL_BIT
            );

            ESP_LOGE(TAG, "Wi-Fi connection failed");

        }

    }
    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(
            TAG,
            "Connected! IP: " IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        s_retry_num = 0;

        xEventGroupSetBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );

    }
}

// ==========================================
// 4. WIFI INITIALIZATION
// ==========================================

static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Cannot create Wi-Fi event group");
        return false;
    }

    // ตรวจสอบความยาว SSID และ Password
    size_t ssid_len = strlen(CONFIG_WIFI_SSID);
    size_t pass_len = strlen(CONFIG_WIFI_PASSWORD);

    wifi_config_t wifi_config = {0};

    if (ssid_len == 0 ||
        ssid_len > sizeof(wifi_config.sta.ssid) ||
        pass_len > sizeof(wifi_config.sta.password)) {

        ESP_LOGE(TAG, "Invalid Wi-Fi SSID or password length");
        return false;
    }

    memcpy(
        wifi_config.sta.ssid,
        CONFIG_WIFI_SSID,
        ssid_len
    );

    memcpy(
        wifi_config.sta.password,
        CONFIG_WIFI_PASSWORD,
        pass_len
    );

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create Wi-Fi station
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register Wi-Fi events
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

    // Start Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config)
    );

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi: %s", CONFIG_WIFI_SSID);

    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    if (bits & WIFI_CONNECTED_BIT) {

        ESP_LOGI(TAG, "Wi-Fi connected successfully");
        return true;

    }

    ESP_LOGE(TAG, "Unable to connect to Wi-Fi");

    return false;
}

// ==========================================
// 5. mDNS INITIALIZATION
// ==========================================

static void initialise_mdns(void)
{
    ESP_ERROR_CHECK(mdns_init());

    ESP_ERROR_CHECK(
        mdns_hostname_set("esp32-node")
    );

    ESP_ERROR_CHECK(
        mdns_instance_name_set("ESP32 RESTful Controller")
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
        "mDNS initialized: http://esp32-node.local"
    );
}

// ==========================================
// 6. GET /api/status
// ==========================================

static esp_err_t status_get_handler(httpd_req_t *req)
{
    int pot_val = 0;

    // Read potentiometer ADC
    if (s_adc1_handle != NULL) {

        esp_err_t adc_ret = adc_oneshot_read(
            s_adc1_handle,
            POT_ADC_CHANNEL,
            &pot_val
        );

        if (adc_ret != ESP_OK) {
            ESP_LOGE(TAG, "ADC read failed");
            pot_val = 0;
        }

    }

    // Create JSON object
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
        s_led_state
    );

    // Convert JSON object to string
    const char *resp = cJSON_PrintUnformatted(root);

    if (resp == NULL) {
        cJSON_Delete(root);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");

    esp_err_t ret = httpd_resp_send(
        req,
        resp,
        strlen(resp)
    );

    // Free allocated memory
    cJSON_free((void *)resp);
    cJSON_Delete(root);

    return ret;
}

// ==========================================
// 7. POST /api/led
// ==========================================

static esp_err_t led_post_handler(httpd_req_t *req)
{
    char buf[MAX_JSON_SIZE];

    // Check request body size
    if (req->content_len <= 0 ||
        req->content_len >= sizeof(buf)) {

        httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Invalid request body size"
        );

        return ESP_FAIL;
    }

    // Receive the complete HTTP body
    size_t received = 0;

    while (received < req->content_len) {

        int ret = httpd_req_recv(
            req,
            buf + received,
            req->content_len - received
        );

        if (ret <= 0) {

            httpd_resp_send_500(req);

            return ESP_FAIL;
        }

        received += (size_t)ret;
    }

    buf[received] = '\0';

    // Parse JSON
    cJSON *root = cJSON_Parse(buf);

    if (root == NULL || !cJSON_IsObject(root)) {

        cJSON_Delete(root);

        httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Invalid JSON"
        );

        return ESP_FAIL;
    }

    cJSON *state = cJSON_GetObjectItemCaseSensitive(
        root,
        "state"
    );

    if (!cJSON_IsBool(state)) {

        cJSON_Delete(root);

        httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "state must be true or false"
        );

        return ESP_FAIL;
    }

    bool led_on = cJSON_IsTrue(state);

    // Control LED GPIO 2
    gpio_set_level(
        LED_GPIO_PIN,
        led_on ? 1 : 0
    );

    s_led_state = led_on;

    ESP_LOGI(
        TAG,
        "LED switched to: %s",
        led_on ? "ON" : "OFF"
    );

    cJSON_Delete(root);

    // Send response
    const char *resp = "{\"result\":\"success\"}";

    httpd_resp_set_type(req, "application/json");

    return httpd_resp_send(
        req,
        resp,
        strlen(resp)
    );
}

// ==========================================
// 8. START HTTP WEB SERVER
// ==========================================

static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.lru_purge_enable = true;

    // GET URI
    httpd_uri_t uri_get_status = {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = status_get_handler,
        .user_ctx = NULL
    };

    // POST URI
    httpd_uri_t uri_post_led = {
        .uri = "/api/led",
        .method = HTTP_POST,
        .handler = led_post_handler,
        .user_ctx = NULL
    };

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {

        if (httpd_register_uri_handler(
                server,
                &uri_get_status
            ) != ESP_OK ||
            httpd_register_uri_handler(
                server,
                &uri_post_led
            ) != ESP_OK) {

            httpd_stop(server);

            ESP_LOGE(TAG, "Cannot register HTTP URI");

            return NULL;
        }

        ESP_LOGI(
            TAG,
            "HTTP Server started on port %d",
            config.server_port
        );

        return server;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server");

    return NULL;
}

// ==========================================
// 9. MAIN APPLICATION
// ==========================================

void app_main(void)
{
    ESP_LOGI(TAG, "Starting LAB 10-1");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());

        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    // Configure LED GPIO 2
    ESP_ERROR_CHECK(gpio_reset_pin(LED_GPIO_PIN));

    ESP_ERROR_CHECK(
        gpio_set_direction(
            LED_GPIO_PIN,
            GPIO_MODE_OUTPUT
        )
    );

    ESP_ERROR_CHECK(
        gpio_set_level(LED_GPIO_PIN, 0)
    );

    s_led_state = false;

    // Initialize ADC1
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_config1,
            &s_adc1_handle
        )
    );

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11
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
        "ADC initialized on GPIO 34 (Channel 6)"
    );

    // Connect Wi-Fi
    if (wifi_init_sta()) {

        // Initialize mDNS
        initialise_mdns();

        // Start HTTP Server
        s_http_server = start_webserver();

        if (s_http_server != NULL) {

            ESP_LOGI(
                TAG,
                "System ready!"
            );

            ESP_LOGI(
                TAG,
                "Test: curl http://esp32-node.local/api/status"
            );

        }

    }
    else {

        ESP_LOGE(
            TAG,
            "Cannot start server: Wi-Fi connection failed"
        );

    }
}
