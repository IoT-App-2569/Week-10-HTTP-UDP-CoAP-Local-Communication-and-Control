#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "esp_log.h"
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

// ==========================================
// 1. การตั้งค่า Wi-Fi
// ==========================================
#define CONFIG_WIFI_SSID      "1234_2.4G"
#define CONFIG_WIFI_PASSWORD  "123456789"
#define MAXIMUM_RETRY         5

#define LED_GPIO_PIN          GPIO_NUM_2
#define POT_ADC_CHANNEL       ADC_CHANNEL_6 // GPIO 34 (ADC1 Channel 6)

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1

static int s_retry_num = 0;
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
static httpd_handle_t s_http_server = NULL;

// ==========================================
// 2. ตัวแปรจำลองค่า Potentiometer (Virtual / Simulated Potentiometer)
//    เนื่องจากไม่มีฮาร์ดแวร์ Potentiometer จริง
//    จึงสามารถปรับค่าได้ผ่าน HTTP REST API หรือ Query Parameter
// ==========================================
static int  s_virtual_pot_raw   = 2048; // ค่าเริ่มต้น (0 - 4095)
static bool s_use_virtual_pot   = true; // เปิดใช้งานระบบจำลอง Potentiometer
static bool s_auto_sweep        = false;// โหมดปรับค่าขึ้นลงอัตโนมัติ
static int  s_sweep_direction   = 1;    // 1 = เพิ่มขึ้น, -1 = ลดลง

// Wi-Fi Event Handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying Wi-Fi connection (%d/%d)...", s_retry_num, MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Wi-Fi Connected! IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// เริ่มต้นระบบเชื่อมต่อ Wi-Fi Station
static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    size_t ssid_len = strlen(CONFIG_WIFI_SSID);
    if (ssid_len > sizeof(wifi_config.sta.ssid)) ssid_len = sizeof(wifi_config.sta.ssid);
    memcpy(wifi_config.sta.ssid, CONFIG_WIFI_SSID, ssid_len);

    size_t pass_len = strlen(CONFIG_WIFI_PASSWORD);
    if (pass_len > sizeof(wifi_config.sta.password)) pass_len = sizeof(wifi_config.sta.password);
    memcpy(wifi_config.sta.password, CONFIG_WIFI_PASSWORD, pass_len);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to SSID: %s ...", CONFIG_WIFI_SSID);

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP successfully!");
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to connect to AP");
        return false;
    }
}

// กำหนดค่าเริ่มต้นให้กับ mDNS
static void initialise_mdns(void)
{
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("esp32-node"));
    ESP_ERROR_CHECK(mdns_instance_name_set("ESP32 RESTful Controller"));

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"role", "actuator"}
    };
    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebControl", "_http", "_tcp", 80, serviceTxtData, 2));
    ESP_LOGI(TAG, "mDNS initialized! Hostname: http://esp32-node.local");
}

// =========================================================================
// 1. GET /api/status - อ่านค่าเซนเซอร์และสถานะระบบ
//    รองรับ Query string สำหรับปรับค่า Potentiometer เช่น:
//    /api/status?pot=3000 หรือ /api/status?auto=1
// =========================================================================
static esp_err_t status_get_handler(httpd_req_t *req)
{
    // ตรวจสอบ Query parameters (ถ้ามี) เพื่ออนุญาตให้ปรับค่า Potentiometer ผ่าน URL
    char query[64];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char param[32];
        if (httpd_query_key_value(query, "pot", param, sizeof(param)) == ESP_OK) {
            int val = atoi(param);
            if (val >= 0 && val <= 4095) {
                s_virtual_pot_raw = val;
                s_use_virtual_pot = true;
                ESP_LOGI(TAG, "Virtual Pot adjusted via query to: %d", s_virtual_pot_raw);
            }
        }
        if (httpd_query_key_value(query, "auto", param, sizeof(param)) == ESP_OK) {
            s_auto_sweep = (atoi(param) != 0);
            ESP_LOGI(TAG, "Auto sweep set to: %d", s_auto_sweep);
        }
    }

    // หากเปิดโหมด Auto Sweep ให้จำลองการหมุน Potentiometer ไปกลับ
    if (s_auto_sweep) {
        s_virtual_pot_raw += s_sweep_direction * 150;
        if (s_virtual_pot_raw >= 4095) {
            s_virtual_pot_raw = 4095;
            s_sweep_direction = -1;
        } else if (s_virtual_pot_raw <= 0) {
            s_virtual_pot_raw = 0;
            s_sweep_direction = 1;
        }
    }

    int pot_val = 0;
    if (s_use_virtual_pot) {
        pot_val = s_virtual_pot_raw;
    } else if (s_adc1_handle != NULL) {
        adc_oneshot_read(s_adc1_handle, POT_ADC_CHANNEL, &pot_val);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "pot_raw", pot_val);
    cJSON_AddNumberToObject(root, "free_heap", esp_get_free_heap_size());
    cJSON_AddBoolToObject(root, "led", gpio_get_level(LED_GPIO_PIN));

    const char *resp = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));

    cJSON_free((void *)resp);
    cJSON_Delete(root);
    return ESP_OK;
}

// =========================================================================
// 2. POST /api/led - ควบคุมหลอดไฟ LED
// =========================================================================
static esp_err_t led_post_handler(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root != NULL) {
        cJSON *state = cJSON_GetObjectItem(root, "state");
        if (cJSON_IsBool(state)) {
            bool led_on = cJSON_IsTrue(state);
            gpio_set_level(LED_GPIO_PIN, led_on ? 1 : 0);
            ESP_LOGI(TAG, "LED (GPIO %d) switched to: %s", LED_GPIO_PIN, led_on ? "ON" : "OFF");
        }
        // อนุญาตให้ปรับค่า pot_raw แนบมากับ POST /api/led ได้ด้วย
        cJSON *pot_item = cJSON_GetObjectItem(root, "pot_raw");
        if (cJSON_IsNumber(pot_item)) {
            s_virtual_pot_raw = pot_item->valueint;
            s_use_virtual_pot = true;
            ESP_LOGI(TAG, "Virtual Pot adjusted via /api/led to: %d", s_virtual_pot_raw);
        }
        cJSON_Delete(root);
    }

    const char *resp = "{\"result\":\"success\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

// =========================================================================
// 3. POST /api/pot - ปรับค่า Potentiometer เสมือนโดยตรง
//    (ช่วยแก้ปัญหาเมื่อไม่มีฮาร์ดแวร์ Potentiometer จริง)
//    Payload: {"pot_raw": 3200} หรือ {"step": 200} หรือ {"auto": true}
// =========================================================================
static esp_err_t pot_post_handler(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root != NULL) {
        cJSON *val_item = cJSON_GetObjectItem(root, "pot_raw");
        if (!val_item) {
            val_item = cJSON_GetObjectItem(root, "value");
        }
        if (cJSON_IsNumber(val_item)) {
            int val = val_item->valueint;
            if (val < 0) val = 0;
            if (val > 4095) val = 4095;
            s_virtual_pot_raw = val;
            s_use_virtual_pot = true;
            ESP_LOGI(TAG, "Virtual Pot set to: %d", s_virtual_pot_raw);
        }

        cJSON *step_item = cJSON_GetObjectItem(root, "step");
        if (cJSON_IsNumber(step_item)) {
            s_virtual_pot_raw += step_item->valueint;
            if (s_virtual_pot_raw < 0) s_virtual_pot_raw = 0;
            if (s_virtual_pot_raw > 4095) s_virtual_pot_raw = 4095;
            s_use_virtual_pot = true;
            ESP_LOGI(TAG, "Virtual Pot stepped to: %d", s_virtual_pot_raw);
        }

        cJSON *auto_item = cJSON_GetObjectItem(root, "auto");
        if (cJSON_IsBool(auto_item)) {
            s_auto_sweep = cJSON_IsTrue(auto_item);
            s_use_virtual_pot = true;
            ESP_LOGI(TAG, "Virtual Pot auto sweep set to: %d", s_auto_sweep);
        }

        cJSON_Delete(root);
    }

    cJSON *resp_json = cJSON_CreateObject();
    cJSON_AddStringToObject(resp_json, "result", "success");
    cJSON_AddNumberToObject(resp_json, "pot_raw", s_virtual_pot_raw);
    cJSON_AddBoolToObject(resp_json, "auto", s_auto_sweep);

    const char *resp = cJSON_PrintUnformatted(resp_json);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));

    cJSON_free((void *)resp);
    cJSON_Delete(resp_json);
    return ESP_OK;
}

// เริ่มต้น HTTP Web Server และลงทะเบียน URI Handlers
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
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

    httpd_uri_t uri_post_pot = {
        .uri      = "/api/pot",
        .method   = HTTP_POST,
        .handler  = pot_post_handler,
        .user_ctx = NULL
    };

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get_status);
        httpd_register_uri_handler(server, &uri_post_led);
        httpd_register_uri_handler(server, &uri_post_pot);
        ESP_LOGI(TAG, "HTTP Server started on port %d", config.server_port);
        return server;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server!");
    return NULL;
}

void app_main(void)
{
    // 1. กำหนดค่าเริ่มต้น NVS Flash (จำเป็นสำหรับ Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. กำหนดค่าขา GPIO 2 เป็น Output สำหรับควบคุม LED
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_INPUT_OUTPUT);

    // 3. กำหนดค่า ADC1 สำหรับ Potentiometer (GPIO 34) เผื่อในอนาคตมีการเชื่อมต่อ
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    if (adc_oneshot_new_unit(&init_config1, &s_adc1_handle) == ESP_OK) {
        adc_oneshot_chan_cfg_t chan_config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_12,
        };
        adc_oneshot_config_channel(s_adc1_handle, POT_ADC_CHANNEL, &chan_config);
        ESP_LOGI(TAG, "ADC Initialized on GPIO 34 (Channel 6)");
    }

    ESP_LOGI(TAG, "Virtual Potentiometer Enabled (Default: %d)", s_virtual_pot_raw);

    // 4. เชื่อมต่อระบบ Wi-Fi
    if (wifi_init_sta()) {
        // 5. เริ่มต้น mDNS Service Discovery
        initialise_mdns();

        // 6. เริ่มต้น HTTP RESTful Web Server
        s_http_server = start_webserver();
        ESP_LOGI(TAG, "Ready! Test with: curl http://esp32-node.local/api/status");
    } else {
        ESP_LOGE(TAG, "Cannot start server due to Wi-Fi connection failure.");
    }
}
