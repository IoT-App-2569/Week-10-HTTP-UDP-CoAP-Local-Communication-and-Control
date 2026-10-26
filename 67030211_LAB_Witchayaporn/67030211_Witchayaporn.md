# รายงานผลการทดลอง Week-10: Local Control Protocols (HTTP, UDP, CoAP) & Edge Device Discovery
**รหัสนักศึกษา:** 67030211  
**ชื่อ-นามสกุล:** วิชญาพร เนียมเที่ยง  
**วิชา:** 03376134 APPLICATIONS OF INTERNET OF THINGS  

---

## สรุปรายการปฏิบัติการ
- Labsheet 10.1: mDNS Discovery & HTTP REST Server on ESP-IDF
- Labsheet 10.2: UDP Socket Broadcast & Real-Time Telemetry Streaming
- Labsheet 10.3: CoAP Server & Lightweight Constrained IoT Control
- Labsheet 10.4: Protocol Benchmark & Network Forensics (Performance Analyzer)

---

### 📍 Labsheet 10.1: mDNS Discovery & HTTP REST Server on ESP-IDF

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
  สามารถติดตั้งระบบ mDNS ทำให้เข้าถึงบอร์ด ESP32 ผ่านชื่อ `esp32-node.local` ได้โดยตรง และสร้าง HTTP RESTful Server รับคำขอ GET เพื่ออ่านค่าเซนเซอร์ และ POST เพื่อควบคุมไฟ LED พร้อมประมวลผล JSON Payload ด้วย cJSON ได้อย่างสมบูรณ์

- **คำถามท้ายการทดลอง:**
  1. **นำผลการรัน `curl -i` มาแปะในรายงาน พร้อมวิเคราะห์ว่า HTTP Header มีขนาดกี่ไบต์ และข้อมูล JSON มีขนาดกี่ไบต์**
     - **คำตอบ:**
       - ส่วนหัว HTTP Response Header มีขนาดประมาณ 90 ไบต์
       - ข้อมูลเนื้อหา JSON Body (`{"pot_raw":2048,"free_heap":213848,"led":false}`) มีขนาด 47 ไบต์
       - ขนาดของ Header มีขนาดใหญ่กว่าตัวข้อมูลจริงเกือบ 2 เท่า ส่งผลให้ประสิทธิภาพสัดส่วน Payload ต่อข้อมูลรวมต่ำ
  2. **หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?**
     - **คำตอบ:** คอมพิวเตอร์จะไม่สามารถแปลงชื่อโดเมน `esp32-node.local` เป็นหมายเลข IP ได้ ทำให้เกิดข้อผิดพลาดไม่พบโฮสต์ (Host Not Found) แก้ไขได้โดยเข้าถึงอุปกรณ์ผ่านหมายเลข IP Address โดยตรง (เช่น `http://192.168.1.181/api/status`) หรือกำหนด Static IP ให้กับบอร์ดและเพิ่มการจับคู่ชื่อโฮสต์ในไฟล์ hosts ของระบบปฏิบัติการ
  3. **เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?**
     - **คำตอบ:** เพราะไลบรารี cJSON มีการจัดสรรหน่วยความจำแบบไดนามิกบน Heap เพื่อสร้างโครงสร้างต้นไม้ของออบเจกต์และสร้างสตริงผลลัพธ์ หากไม่เรียกคำสั่งคืนหน่วยความจำ จะเกิดปัญหาหน่วยความจำรั่วไหล (Memory Leak) สะสมจนหน่วยความจำแรมหมด ทำให้ระบบค้างหรือเกิดการรีบูตตัวเอง

---

### 📍 Labsheet 10.2: UDP Socket Broadcast & Real-Time Telemetry Streaming

#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

โค้ดภาษา C บน ESP-IDF (`Lab10-2_UDP_Telemetry_Socket.c`) สำหรับการส่งบรอดแคสต์ข้อมูลโทรมาตรความถี่ 10 Hz (พอร์ต 3334) และเปิด UDP Socket รอรับคำสั่งควบคุม LED (พอร์ต 3333):

```c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define UDP_CONTROL_PORT   3333
#define UDP_BROADCAST_PORT 3334
#define LED_GPIO_PIN       GPIO_NUM_2
#define POT_ADC_CHANNEL    ADC_CHANNEL_6 // GPIO 34

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;

// Task ที่ 1: รับคำสั่งควบคุมไฟ LED และส่ง ACK กลับ (Unicast)
void udp_control_server_task(void *pvParameters)
{
    char rx_buffer[128];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_CONTROL_PORT);

    bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    ESP_LOGI("UDP_SERVER", "Listening on port %d...", UDP_CONTROL_PORT);

    while (1) {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&client_addr, &client_addr_len);
        if (len > 0) {
            rx_buffer[len] = '\0';
            if (strcmp(rx_buffer, "LED_ON") == 0) {
                gpio_set_level(LED_GPIO_PIN, 1);
                sendto(sock, "ACK:LED_ON", 10, 0, (struct sockaddr *)&client_addr, client_addr_len);
            } else if (strcmp(rx_buffer, "LED_OFF") == 0) {
                gpio_set_level(LED_GPIO_PIN, 0);
                sendto(sock, "ACK:LED_OFF", 11, 0, (struct sockaddr *)&client_addr, client_addr_len);
            } else {
                sendto(sock, "ACK:UNKNOWN", 11, 0, (struct sockaddr *)&client_addr, client_addr_len);
            }
        }
    }
}

// Task ที่ 2: ส่งข้อมูลโทรมาตรแบบบรอดแคสต์ 10 Hz
void udp_telemetry_broadcast_task(void *pvParameters)
{
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_BROADCAST_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    int broadcast_enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    uint32_t seq = 0;
    char tx_buffer[64];

    while (1) {
        int pot_val = 0;
        if (s_adc1_handle != NULL) {
            adc_oneshot_read(s_adc1_handle, POT_ADC_CHANNEL, &pot_val);
        }
        snprintf(tx_buffer, sizeof(tx_buffer), "SEQ:%lu,POT:%d\n", seq++, pot_val);
        sendto(sock, tx_buffer, strlen(tx_buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz
    }
}
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ผลการรันสคริปต์ `udp_listener.py` บนเครื่องคอมพิวเตอร์เพื่อดักฟังข้อมูลบรอดแคสต์:
```text
Listening for UDP Broadcast on port 3334...
[192.168.1.181] Seq: 101    | Potentiometer: 2048  | Total Lost: 0
[192.168.1.181] Seq: 102    | Potentiometer: 2050  | Total Lost: 0
[192.168.1.181] Seq: 103    | Potentiometer: 2045  | Total Lost: 0
[192.168.1.181] Seq: 104    | Potentiometer: 2052  | Total Lost: 0
```

ผลการทดสอบส่งคำสั่งควบคุม LED สลับ 10 ครั้ง และวัดค่า RTT ด้วย `udp_controller.py`:
```text
Sending control commands to 192.168.1.181:3333...
Round 01: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT: 3.82 ms
Round 02: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 2.95 ms
Round 03: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT: 3.12 ms
Round 04: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 2.88 ms
Round 05: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT: 3.05 ms
Round 06: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 2.76 ms
Round 07: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT: 2.91 ms
Round 08: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 3.24 ms
Round 09: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT: 2.80 ms
Round 10: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 3.15 ms

Average UDP RTT Latency: 3.07 ms (Min: 2.76 ms, Max: 3.82 ms)
```

สถิติการรับส่งบรอดแคสต์ต่อเนื่อง 1 นาที (600 แพ็กเก็ต):
- ได้รับ: 588 แพ็กเก็ต
- สูญหาย: 12 แพ็กเก็ต
- Packet Loss Rate: 2.0%

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
  สามารถสร้างการสื่อสารด้วย UDP Socket ทั้งรูปแบบ Broadcast สำหรับกระจายข้อมูลโทรมาตรความถี่สูง และ Unicast สำหรับส่งคำสั่งควบคุมไฟ LED ได้สำเร็จ โดยมีความหน่วงเวลาในการตอบสนองเฉลี่ยต่ำเพียง 3.07 มิลลิวินาที

- **คำถามท้ายการทดลอง:**
  1. **นำผลการวัดค่า RTT Latency ของ UDP มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง**
     - **คำตอบ:** UDP มีความหน่วงเวลาเฉลี่ยอยู่ที่ประมาณ 3.07 ms ในขณะที่ HTTP RESTful มีความหน่วงเวลาเฉลี่ยอยู่ที่ประมาณ 42.8 ms ความแตกต่างเกิดจาก HTTP ทำงานบน TCP ซึ่งต้องมีกระบวนการจับมือ 3 ขั้นตอน (3-Way Handshake) และมีภาระส่วนหัวขนาดใหญ่เกือบ 200 ไบต์ ส่วน UDP ไม่ต้องสร้างการเชื่อมต่อและส่งข้อมูลดิบขนาดเล็กได้ทันที จึงมีความหน่วงเวลาต่ำกว่ามาก
  2. **รันสคริปต์ `udp_listener.py` เป็นเวลา 1 นาที จงบันทึกค่าและคำนวณอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) พร้อมวิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายบนเครือข่าย Wi-Fi**
     - **คำตอบ:** ตรวจพบการสูญหาย 12 แพ็กเก็ต จากทั้งหมด 600 แพ็กเก็ต คิดเป็นอัตราการสูญหาย 2.0% สาเหตุเกิดจากโพรโทคอล UDP ไม่มีการตรวจสอบความสมบูรณ์และไม่มีการส่งซ้ำเมื่อข้อมูลสูญหาย ประกอบกับสัญญาณ Wi-Fi อาจมีสัญญาณรบกวนหรือการชนกันของคลื่นสัญญาณในเครือข่ายไร้สาย
  3. **อธิบายข้อดีและข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) ในระบบ IoT และในสถานการณ์ใดที่ควรเปลี่ยนไปใช้ UDP Multicast หรือ Unicast แทน?**
     - **คำตอบ:**
       - ข้อดี: ส่งข้อมูลเพียงครั้งเดียว ทุกโหนดในเครือข่ายวงแลนเดียวกันสามารถรับข้อมูลได้พร้อมกันโดยไม่ต้องทราบ IP ผู้รับล่วงหน้า
       - ข้อจำกัด: สิ้นเปลืองทรัพยากรแบนด์วิดท์เครือข่ายเนื่องจากทุกเครื่องต้องรับและประมวลผลแพ็กเก็ต และไม่สามารถส่งข้ามเราเตอร์ได้
       - สถานการณ์ที่ควรเปลี่ยน: ควรเปลี่ยนเป็น UDP Multicast เมื่อต้องการกระจายข้อมูลเฉพาะกลุ่มอุปกรณ์ที่สนใจ และควรเปลี่ยนเป็น UDP Unicast เมื่อต้องการส่งข้อมูลหรือคำสั่งควบคุมไปยังอุปกรณ์เป้าหมายตัวใดตัวหนึ่งโดยเฉพาะ

---

### 📍 Labsheet 10.3: CoAP Server & Lightweight Constrained IoT Control

#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

โค้ดภาษา C บน ESP-IDF (`Lab10-3_CoAP_Server.c`) พัฒนา CoAP Server บนพอร์ต UDP 5683 เพื่อเปิด Resource `/sensor/pot` (GET) และ `/actuator/led` (PUT) รองรับการควบคุมแบบ RESTful ผ่าน UDP:

```c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "coap3/coap.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define LED_GPIO_PIN    GPIO_NUM_2
#define POT_ADC_CHANNEL ADC_CHANNEL_6

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;

// Handler สำหรับ GET /sensor/pot
static void hnd_get_pot(coap_resource_t *resource,
                        coap_session_t *session,
                        const coap_pdu_t *request,
                        const coap_string_t *query,
                        coap_pdu_t *response)
{
    int pot_val = 0;
    if (s_adc1_handle != NULL) {
        adc_oneshot_read(s_adc1_handle, POT_ADC_CHANNEL, &pot_val);
    }
    char pot_str[32];
    snprintf(pot_str, sizeof(pot_str), "%d", pot_val);

    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT); // 2.05 Content
    coap_add_data(response, strlen(pot_str), (const uint8_t *)pot_str);
    ESP_LOGI("CoAP", "Responded GET /sensor/pot: %s", pot_str);
}

// Handler สำหรับ PUT /actuator/led
static void hnd_put_led(coap_resource_t *resource,
                        coap_session_t *session,
                        const coap_pdu_t *request,
                        const coap_string_t *query,
                        coap_pdu_t *response)
{
    size_t size;
    const uint8_t *data;
    coap_get_data(request, &size, &data);

    if (size > 0) {
        if (data[0] == '1') {
            gpio_set_level(LED_GPIO_PIN, 1);
            ESP_LOGI("CoAP", "LED turned ON");
        } else if (data[0] == '0') {
            gpio_set_level(LED_GPIO_PIN, 0);
            ESP_LOGI("CoAP", "LED turned OFF");
        }
    }
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED); // 2.04 Changed
}

// CoAP Server Task
void coap_server_task(void *pvParameters)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;

    coap_startup();
    coap_address_init(&serv_addr);
    serv_addr.addr.sin.sin_family = AF_INET;
    serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
    serv_addr.addr.sin.sin_port = htons(5683);

    ctx = coap_new_context(NULL);
    coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);

    // ลงทะเบียน Resource /sensor/pot
    coap_resource_t *r_pot = coap_resource_init(coap_make_str_const("sensor/pot"), 0);
    coap_register_handler(r_pot, COAP_REQUEST_GET, hnd_get_pot);
    coap_add_resource(ctx, r_pot);

    // ลงทะเบียน Resource /actuator/led
    coap_resource_t *r_led = coap_resource_init(coap_make_str_const("actuator/led"), 0);
    coap_register_handler(r_led, COAP_REQUEST_PUT, hnd_put_led);
    coap_add_resource(ctx, r_led);

    ESP_LOGI("CoAP", "CoAP Server Ready on UDP 5683");

    while (1) {
        coap_io_process(ctx, 100);
    }
    coap_free_context(ctx);
    coap_cleanup();
    vTaskDelete(NULL);
}
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ผลการรันสคริปต์ `test_coap.py` ผ่านไลบรารี `aiocoap` บนคอมพิวเตอร์:

```text
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>,</actuator/led>

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 2048 (Code: 2.05 Content)

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
```

ผลการทำงานบน Serial Monitor ฝั่ง ESP32:
```text
I (2450) CoAP: CoAP Server Ready on UDP 5683
I (5610) CoAP: Responded GET /sensor/pot: 2048
I (7820) CoAP: LED turned ON
I (9840) CoAP: LED turned OFF
```

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
  สามารถติดตั้ง CoAP Server บน ESP-IDF และเปิดให้บริการตามมาตรฐาน CoRE Link Format ได้สำเร็จ สามารถอ่านค่าเซนเซอร์ด้วยเมธอด GET (ตอบกลับรหัส `2.05 Content`) และสั่งงานควบคุม LED ด้วยเมธอด PUT (ตอบกลับรหัส `2.04 Changed`) โดยใช้โครงสร้างข้อมูลแบบไบนารีที่มีขนาดกะทัดรัด

- **คำถามท้ายการทดลอง:**
  1. **นำผลการ Query `/.well-known/core` มาแสดงในรายงาน พร้อมอธิบายรูปแบบ CoRE Link Format (RFC 6690) ว่าแสดงข้อมูลทรัพยากรอย่างไร**
     - **คำตอบ:**
       - ผลลัพธ์ที่ได้: `</sensor/pot>,</actuator/led>`
       - อธิบาย: CoRE Link Format แสดงรายการทรัพยากรที่โหนดให้บริการ โดยคั่นแต่ละ Resource ด้วยเครื่องหมายจุลภาค และระบุพาธของ Resource ไว้ภายในเครื่องหมาย `< >` ช่วยให้ไคลเอนต์สามารถค้นหาและทราบทรัพยากรที่มีอยู่บนอุปกรณ์ได้โดยอัตโนมัติ
  2. **อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ**
     - **คำตอบ:**
       - **CON (Confirmable):** เป็นแพ็กเก็ตที่ต้องการการยืนยันการรับข้อมูล (Acknowledge: ACK) หากผู้รับไม่ตอบกลับภายในเวลาที่กำหนด ผู้ส่งจะทำการส่งแพ็กเก็ตซ้ำ เหมาะสำหรับคำสั่งควบคุมที่สำคัญ
       - **NON (Non-confirmable):** เป็นแพ็กเก็ตที่ไม่ต้องการการตอบรับ (Fire-and-forget) ไม่มีการส่งซ้ำ เหมาะสำหรับข้อมูลเซนเซอร์ที่ส่งสม่ำเสมอเป็นระยะ
  3. **ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?**
     - **คำตอบ:** เพราะ CoAP มีขนาดส่วนหัวคงที่เพียง 4 ไบต์ และทำงานบน UDP ทำให้ไม่ต้องมีภาระการจับมือเชื่อมต่อ TCP และไม่ต้องส่งข้อความส่วนหัวขนาดใหญ่เหมือน HTTP ช่วยประหยัดแบนด์วิดท์ ลดเวลาการเปิดใช้วิทยุสื่อสาร ส่งผลให้ประหยัดพลังงานแบตเตอรี่ได้อย่างมีนัยสำคัญ

---

### 📍 Labsheet 10.4: Protocol Benchmark & Network Forensics (Performance Analyzer)

#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

การทดสอบสมรรถนะเปรียบเทียบระหว่าง HTTP RESTful, UDP Socket และ CoAP จำนวน 50 รอบ ผ่านสคริปต์ `benchmark_protocols.py`:

```powershell
python benchmark_protocols.py --target 192.168.1.181 --protocol all --rounds 50
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ตารางสรุปผลการทดสอบ RTT Latency และสถิติเชิงวิศวกรรม (จำนวน 50 รอบ):

| โปรโตคอล | จำนวนรอบสำเร็จ | Packet Loss (%) | Min RTT (ms) | Mean RTT (ms) | Max RTT (ms) | Jitter / StdDev (ms) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **HTTP REST (`/api/led`)** | 50 / 50 | 0.0 % | 28.52 ms | 42.84 ms | 85.20 ms | 11.45 ms |
| **UDP Socket (`พอร์ต 3333`)** | 49 / 50 | 2.0 % | 2.15 ms | 3.07 ms | 12.80 ms | 1.82 ms |
| **CoAP PUT (`/actuator/led`)** | 50 / 50 | 0.0 % | 7.85 ms | 11.64 ms | 45.20 ms | 6.25 ms |

ตารางการตรวจสอบหน่วยความจำบน ESP32 (RAM Footprint):

| สถานะการทำงานของ ESP32 | หน่วยความจำแรมคงเหลือ (Free Heap) | ปริมาณแรมที่ใช้ไปโดยประมาณ |
| :--- | :---: | :---: |
| **Baseline หลังต่อ Wi-Fi สำเร็จ** | 248 KB | อ้างอิง (0 KB) |
| **HTTP REST Server (`esp_http_server`)** | 215 KB | 33 KB |
| **UDP Socket Server (LwIP Raw Socket)** | 242 KB | **6 KB** |
| **CoAP Server (`libcoap`)** | 205 KB | 43 KB |

ตารางสัดส่วนประสิทธิภาพข้อมูล (Payload Efficiency Ratio Analysis):

| รายการประเมิน | HTTP REST (POST) | UDP Datagram | CoAP (PUT) |
| :--- | :---: | :---: | :---: |
| **ชั้นสื่อสาร L4 Transport** | TCP | UDP | UDP |
| **ขนาด L4 Header** | 20–32 ไบต์ | 8 ไบต์ | 8 ไบต์ |
| **ขนาด L7 Application Header** | ~180–220 ไบต์ | 0 ไบต์ | 4 ไบต์ |
| **ขนาดข้อมูลจริง (Payload)** | 14 ไบต์ (`{"state":true}`) | 6 ไบต์ (`LED_ON`) | 1 ไบต์ (`1`) |
| **ขนาดข้อมูลรวมต่อคำขอ** | ~224 ไบต์ | 14 ไบต์ | 13 ไบต์ |
| **สัดส่วน Payload ต่อข้อมูลรวม (%)** | **~6.25 %** | **~42.85 %** | **~7.69 %** |

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
  จากการทดสอบเปรียบเทียบเชิงวิศวกรรมพบว่า UDP Socket มีความหน่วงเวลาต่ำที่สุด (เฉลี่ย 3.07 ms) และใช้แรมน้อยที่สุด (6 KB) แต่มีอัตราแพ็กเก็ตสูญหาย 2.0% ส่วน CoAP สามารถรวมข้อดีด้านความเร็วระดับ 11.64 ms และความน่าเชื่อถือแบบ Confirmable เข้าด้วยกันได้ดีที่สุด ในขณะที่ HTTP REST มีความสะดวกในการเชื่อมต่อกับเว็บเบราว์เซอร์แต่มีภาระส่วนหัวสูงและความหน่วงเวลามากที่สุด

- **วิเคราะห์ผลและตอบคำถามเชิงวิศวกรรม:**
  1. **เปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix):**
     - **HTTP:** ข้อดีคือทำงานร่วมกับเว็บเบราว์เซอร์ได้ทันทีและมีเครื่องมือรองรับหลากหลาย ข้อจำกัดคือมีภาระส่วนหัวสูง ความหน่วงเวลามาก และกินทรัพยากรแรม
     - **UDP:** ข้อดีคือความหน่วงเวลาต่ำที่สุด ประหยัดแบนด์วิดท์ และใช้แรมน้อยที่สุด ข้อจำกัดคือไม่มีกลไกยืนยันการรับส่ง ข้อมูลอาจสูญหายได้
     - **CoAP:** ข้อดีคือมีสถาปัตยกรรมแบบ RESTful แต่ใช้ส่วนหัวขนาดเล็กเพียง 4 ไบต์บน UDP และรองรับการยืนยันข้อมูล (CON) ข้อจำกัดคือเว็บเบราว์เซอร์ทั่วไปไม่รองรับโดยตรง ต้องมีตัวแปลงโพรโทคอล
  2. **กรณีศึกษาการตัดสินใจเลือกใช้โปรโตคอล (Engineering Case Studies):**
     - **กรณีศึกษาที่ 1: ระบบเซนเซอร์ความชื้นในดินและวาล์วน้ำ ทำงานด้วยแบตเตอรี่โซลาร์เซลล์ กลางแจ้งห่างไกล**
       - **โปรโตคอลที่เลือก:** CoAP
       - **เหตุผลทางวิศวกรรม:** ใช้พลังงานต่ำ มีขนาดแพ็กเก็ตเล็กมากช่วยประหยัดแบนด์วิดท์และอายุการใช้งานแบตเตอรี่ อีกทั้งรองรับแพ็กเก็ตแบบ CON เพื่อยืนยันว่าคำสั่งเปิด-ปิดวาล์วน้ำสำเร็จอย่างแน่นอน
     - **กรณีศึกษาที่ 2: เว็บแดชบอร์ดสำหรับฝ่ายซ่อมบำรุงเปิดดูสถานะและตั้งค่าผ่านเบราว์เซอร์บนแท็บเล็ต/สมาร์ตโฟน**
       - **โปรโตคอลที่เลือก:** HTTP REST
       - **เหตุผลทางวิศวกรรม:** สามารถแสดงผลและทำงานร่วมกับเว็บเบราว์เซอร์บนอุปกรณ์พกพาทุกชนิดได้ทันทีโดยไม่ต้องติดตั้งแอปพลิเคชันหรือโปรแกรมเสริมเพิ่มเติม
     - **กรณีศึกษาที่ 3: ระบบควบคุมแขนกลอุตสาหกรรมความเร็วสูง ส่งพิกัด 100 Hz ภายในเครือข่าย LAN ปิด**
       - **โปรโตคอลที่เลือก:** UDP Socket
       - **เหตุผลทางวิศวกรรม:** ต้องการความเร็วในการตอบสนองสูงสุดและความหน่วงเวลาต่ำที่สุดระดับมิลลิวินาที โดยระบบทำงานในเครือข่ายแลนปิดที่มีความเสถียร หากมีแพ็กเก็ตหล่นหายบางส่วน ข้อมูลตำแหน่งชุดถัดไปจะเข้ามาทดแทนทันทีโดยไม่ต้องเสียเวลารอการส่งซ้ำ
