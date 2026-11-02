# Labsheet 10.1: mDNS Discovery & HTTP REST Server on ESP-IDF


#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

โค้ดภาษา C บนเฟรมเวิร์ก ESP-IDF (`Lab10-1_HTTP_REST_Server.c`) สำหรับการตั้งค่า mDNS Service Discovery และสร้าง HTTP RESTful Server (`esp_http_server`) เพื่อควบคุมหลอดไฟ LED (GPIO 2) และอ่านค่าตัวต้านทานปรับค่าได้ (ADC1 GPIO 34):

```c
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "mdns.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define TAG "HTTP_REST_LAB"
#define LED_GPIO_PIN GPIO_NUM_2
#define POT_ADC_CHANNEL ADC_CHANNEL_6 // GPIO 34

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
static httpd_handle_t s_http_server = NULL;

// 1. ฟังก์ชันเริ่มต้น mDNS Service Discovery
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
    ESP_LOGI(TAG, "mDNS initialized! Hostname: esp32-node.local");
}

// 2. Handler สำหรับ GET /api/status
static esp_err_t get_status_handler(httpd_req_t *req)
{
    int pot_raw = 0;
    if (s_adc1_handle != NULL) {
        adc_oneshot_read(s_adc1_handle, POT_ADC_CHANNEL, &pot_raw);
    }
    int led_state = gpio_get_level(LED_GPIO_PIN);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "pot_raw", pot_raw);
    cJSON_AddNumberToObject(root, "free_heap", (double)esp_get_free_heap_size());
    cJSON_AddBoolToObject(root, "led", led_state ? true : false);

    const char *resp_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp_str, strlen(resp_str));

    cJSON_free((void *)resp_str);
    cJSON_Delete(root);
    return ESP_OK;
}

// 3. Handler สำหรับ POST /api/led
static esp_err_t post_led_handler(httpd_req_t *req)
{
    char buf[128];
    int total_len = req->content_len;
    if (total_len >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }

    int cur_len = 0;
    while (cur_len < total_len) {
        int received = httpd_req_recv(req, buf + cur_len, total_len - cur_len);
        if (received <= 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive post data");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *json = cJSON_Parse(buf);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *state_item = cJSON_GetObjectItem(json, "state");
    if (cJSON_IsBool(state_item)) {
        int new_state = cJSON_IsTrue(state_item) ? 1 : 0;
        gpio_set_level(LED_GPIO_PIN, new_state);
        ESP_LOGI(TAG, "LED State set to %d", new_state);

        const char *resp = "{\"result\":\"success\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, resp, strlen(resp));
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Field 'state' must be boolean");
    }

    cJSON_Delete(json);
    return ESP_OK;
}

// เริ่มต้น Web Server
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 8;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_get_status = {
            .uri = "/api/status",
            .method = HTTP_GET,
            .handler = get_status_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_get_status);

        httpd_uri_t uri_post_led = {
            .uri = "/api/led",
            .method = HTTP_POST,
            .handler = post_led_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_post_led);
    }
    return server;
}
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ผลลัพธ์บน Serial Monitor ขณะเริ่มต้นระบบ:
```text
I (1820) esp_netif_handlers: sta ip: 192.168.1.181, mask: 255.255.255.0, gw: 192.168.1.1
I (1830) HTTP_REST_LAB: mDNS initialized! Hostname: esp32-node.local
I (1840) HTTP_REST_LAB: Ready! Test with: curl http://esp32-node.local/api/status
```

ผลการทดสอบคำสั่ง Ping ตรวจสอบ mDNS จากเครื่องคอมพิวเตอร์:
```text
Pinging esp32-node.local [192.168.1.181] with 32 bytes of data:
Reply from 192.168.1.181: bytes=32 time=4ms TTL=255
Reply from 192.168.1.181: bytes=32 time=3ms TTL=255
Reply from 192.168.1.181: bytes=32 time=3ms TTL=255
Reply from 192.168.1.181: bytes=32 time=4ms TTL=255
```

ผลการทดสอบ `curl.exe -i http://esp32-node.local/api/status`:
```http
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 47
Server: esp_http_server

{"pot_raw":2048,"free_heap":213848,"led":false}
```

ผลการทดสอบสั่งเปิดไฟ LED ผ่าน cURL:
```powershell
curl.exe -s -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d "{\"state\": true}"
```
ผลลัพธ์ตอบกลับ:
```json
{"result":"success"}
```
หลอดไฟ LED บนขา GPIO 2 ติดสว่างขึ้นตามคำสั่ง

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
สามารถติดตั้งและกำหนดค่าระบบค้นหาบริการเครือข่าย mDNS (Multicast DNS) บน ESP32 ภายใต้ชื่อโฮสต์ esp32-node.local ได้สำเร็จ ทำให้คอมพิวเตอร์ในเครือข่ายแลนเดียวกันสามารถค้นหาและสื่อสารกับอุปกรณ์ได้โดยตรงโดยไม่ต้องทราบหมายเลข IP Address ล่วงหน้า นอกจากนี้ยังสร้าง HTTP RESTful Server ด้วยคอมโพเนนต์ esp_http_server และประมวลผลข้อมูลด้วยไลบรารี cJSON ได้อย่างมีประสิทธิภาพ โดยรองรับทั้งคำขอ GET /api/status เพื่ออ่านค่าเซนเซอร์แอนะล็อก ปริมาณแรมคงเหลือ และสถานะของหลอดไฟ ตลอดจนรองรับ POST /api/led เพื่อรับเพย์โหลด JSON สั่งเปิด-ปิดไฟ LED บนขา GPIO 2 และตอบกลับสถานะตามรูปแบบสถาปัตยกรรม RESTful API ได้อย่างถูกต้อง

- **คำถามท้ายการทดลอง:**
1. **นำผลการรัน `curl -i` มาแปะในรายงาน พร้อมวิเคราะห์ว่า HTTP Header มีขนาดกี่ไบต์ และข้อมูล JSON มีขนาดกี่ไบต์**
     - **คำตอบ:** HTTP Header: $96\text{ ไบต์} 
     ขนาดรวมของข้อมูล JSON: $47\text{ ไบต์}$


2. **หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?**
     - **คำตอบ:** ผลกระทบ:เครื่องคอมพิวเตอร์ดังกล่าวจะไม่สามารถแปลงชื่อโดเมน esp32-node.local กลับมาเป็นหมายเลข IP Address ได้ เนื่องจากโปรโตคอล mDNS ส่งและรับแพ็กเก็ต Multicast ผ่านพอร์ต UDP 5353
     แนวทางการแก้ไข:กำหนด Static DHCP Reservation บนเราเตอร์: ตั้งค่าเราเตอร์ให้ผูกหมายเลข MAC Address ของ ESP32 เข้ากับ IP Address ถาวร เพื่อป้องกันไม่ให้ IP เปลี่ยนแปลงเมื่อต่อใหม่

3. **เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?**
     - **คำตอบ:** พราะทั้งฟังก์ชันสร้างออบเจกต์ (เช่น cJSON_CreateObject(), cJSON_Parse()) และฟังก์ชันแปลงเป็นข้อความ (cJSON_PrintUnformatted()) มีการจองหน่วยความจำแบบพลวัตลงใน Heap Memory ผ่านคำสั่ง malloc() ภายในระบบ


