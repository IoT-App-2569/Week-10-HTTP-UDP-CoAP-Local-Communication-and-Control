# ใบงานการทดลองที่ 10.3
### การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้ นักศึกษาจะได้เรียนรู้การติดตั้งและพัฒนาเซิร์ฟเวอร์ด้วยโปรโตคอล **CoAP (Constrained Application Protocol - RFC 7252)** บน ESP32 ซึ่งเป็นโปรโตคอลมาตรฐานสากลสำหรับอุปกรณ์ IoT ที่รวมข้อดีด้านความเบาของ UDP เข้ากับสถาปัตยกรรม RESTful ของ HTTP พร้อมทั้งทดสอบคุณสมบัติ **Observe (RFC 7641)** เพื่อติดตามค่าเซนเซอร์โดยอัตโนมัติ

---

## 1. วัตถุประสงค์การทดลอง
1. เข้าใจโครงสร้างของ CoAP Packet และการทำงานของ CoAP Endpoints (Resources)
2. สามารถพัฒนา CoAP Server บน ESP-IDF เพื่อเปิดให้บริการ Resource `/actuator/led` และ `/sensor/pot` บนพอร์ต UDP 5683 ได้
3. สามารถทดสอบส่งคำสั่ง CoAP แบบ **Confirmable (CON)** และ **Non-confirmable (NON)** ได้
4. สามารถทดสอบกลไก **Resource Discovery (`/.well-known/core`)** เพื่อค้นหารายการทรัพยากรบน ESP32 ได้
5. สามารถเขียนสคริปต์ Python ด้วยไลบรารี `aiocoap` เพื่อทดลองใช้งานฟีเจอร์ **CoAP Observe** ในการรับข้อมูลเซนเซอร์แบบ Real-time Event-driven

---

## 2. โครงสร้างการแมป CoAP Endpoints

| Resource URI        | Method | คำอธิบาย                           |   ชนิด Payload   | ตัวอย่างคำสั่ง CoAP Client                                       |
| :------------------ | :----: | :--------------------------------- | :--------------: | :--------------------------------------------------------------- |
| `/.well-known/core` |  GET   | แสดงรายการ Resource ทั้งหมดของโหนด | CoRE Link Format | `coap-client -m get coap://esp32-node.local/.well-known/core`    |
| `/sensor/pot`       |  GET   | อ่านค่าอนาล็อก Potentiometer       |   Text / JSON    | `coap-client -m get coap://esp32-node.local/sensor/pot`          |
| `/actuator/led`     |  PUT   | สั่งเปิด/ปิดไฟ LED (1 หรือ 0)      |  Text (`1`/`0`)  | `coap-client -m put -e "1" coap://esp32-node.local/actuator/led` |

---

## 3. ขั้นตอนการทดลอง

### กิจกรรมที่ 10-3.1 สร้างโปรเจกต์และเพิ่ม CoAP Component

#### 1. สร้างโปรเจกต์ใหม่
```powershell
idf.py create-project Lab10-3_CoAP_Server
cd Lab10-3_CoAP_Server
```

**หรือรันผ่าน Docker**
```powershell
docker run --rm --mount "type=bind,source=$((Get-Location).Path),target=/workspace" -w /workspace espressif/idf:release-v6.1 idf.py create-project Lab10-3_CoAP_Server
cd Lab10-3_CoAP_Server
```

#### 2. กำหนด Target เป็นชิป ESP32
```powershell
idf.py set-target esp32
```

**หรือรันผ่าน Docker**
```powershell
docker run --rm --mount "type=bind,source=$((Get-Location).Path),target=/workspace" -w /workspace espressif/idf:release-v6.1 idf.py set-target esp32
```

#### 3. การเพิ่ม Dependency  `espressif/coap` (IDF Component Manager)
ใน ESP-IDF v5/v6 ไลบรารี `libcoap` ถูกย้ายไปอยู่บน Component Registry จึงต้องลงทะเบียน dependency ก่อนเสมอ

```powershell
idf.py add-dependency "espressif/coap"
```

**หรือรันผ่าน Docker**
```powershell
# รันผ่าน Docker จากโฟลเดอร์โปรเจกต์ Lab10-3_CoAP_Server
docker run --rm --mount "type=bind,source=$((Get-Location).Path),target=/workspace" -w /workspace espressif/idf:release-v6.1 idf.py add-dependency "espressif/coap"
```

*(หรือสร้าง/ตรวจสอบไฟล์ `main/idf_component.yml` ให้มีเนื้อหาดังนี้)*
```yaml
dependencies:
  idf:
    version: '>=4.1.0'
  espressif/coap: '*'
```

#### 4. ตั้งค่า `main/CMakeLists.txt`
```cmake
idf_component_register(SRCS "Lab10-3_CoAP_Server.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi esp_event nvs_flash coap esp_adc esp_driver_gpio)
```

#### 5. สร้างไฟล์ `sdkconfig.defaults` เพื่อเปิดใช้ฟีเจอร์ DTLS Cookie ใน mbedTLS
> [!IMPORTANT] **ข้อกำหนดสำคัญสำหรับ `espressif/coap`**
> ไลบรารี `libcoap` มีการเรียกใช้ฟังก์ชัน DTLS cookie (`mbedtls_ssl_cookie_*`) ซึ่งค่าเริ่มต้นของ ESP-IDF จะปิดใช้งาน DTLS ไว้ หากไม่เปิดใช้งานจะเกิด Linker Error (`undefined reference to mbedtls_ssl_cookie_init`)
>
> ให้สร้างไฟล์ `sdkconfig.defaults` ในโฟลเดอร์หลักของโปรเจกต์ `Lab10-3_CoAP_Server/`
> ```ini
> CONFIG_MBEDTLS_SSL_PROTO_DTLS=y
> CONFIG_MBEDTLS_SSL_COOKIE_C=y
> ```

#### 6. ทดสอบ Reconfigure ระบบบิลด์
ทดสอบรันคำสั่ง Reconfigure เพื่อให้ระบบดาวน์โหลดคอมโพเนนต์และสร้างบิลด์ไฟล์
```powershell
idf.py reconfigure
```

**หรือรันผ่าน Docker**
```powershell
docker run --rm --mount "type=bind,source=$((Get-Location).Path),target=/workspace" -w /workspace espressif/idf:release-v6.1 idf.py reconfigure
```

เมื่อปรากฏข้อความ `-- Configuring done` และ `-- Generating done` แสดงว่าโครงสร้างโปรเจกต์พร้อมสำหรับการเขียนโค้ดในกิจกรรมถัดไป

---

### กิจกรรมที่ 10-3.2 การพัฒนา Handlers สำหรับ Resource `/sensor/pot` และ `/actuator/led`
เขียนฟังก์ชัน Callback สำหรับจัดการคำขออ่านค่าเซนเซอร์ (GET) และสั่งงานควบคุม LED (PUT)

```c
#include "coap3/coap.h"

// 1. Handler สำหรับ GET /sensor/pot
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

// 2. Handler สำหรับ PUT /actuator/led
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
            ESP_LOGI("CoAP", "LED turned ON via CoAP PUT");
        } else if (data[0] == '0') {
            gpio_set_level(LED_GPIO_PIN, 0);
            ESP_LOGI("CoAP", "LED turned OFF via CoAP PUT");
        }
    }
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED); // 2.04 Changed
}
```

---

### กิจกรรมที่ 10-3.3 การพัฒนา CoAP Server FreeRTOS Task
สร้าง Task สำหรับเริ่มต้น CoAP Context, ลงทะเบียน Resources, และวนลูปประมวลผล Event Loop ด้วย `coap_io_process()`

```c
static void coap_server_task(void *pvParameters)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;

    while (1) {
        coap_address_init(&serv_addr);
        serv_addr.addr.sin.sin_family = AF_INET;
        serv_addr.addr.sin.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.addr.sin.sin_port = htons(COAP_DEFAULT_PORT); // 5683

        ctx = coap_new_context(NULL);
        if (!ctx) {
            ESP_LOGE("CoAP", "coap_new_context() failed");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        coap_endpoint_t *ep = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);
        if (!ep) {
            ESP_LOGE("CoAP", "coap_new_endpoint() failed");
            coap_free_context(ctx);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // 1. ลงทะเบียน Resource: sensor/pot (GET)
        coap_str_const_t *r_pot_uri = coap_make_str_const("sensor/pot");
        coap_resource_t *r_pot = coap_resource_init(r_pot_uri, 0);
        coap_register_handler(r_pot, COAP_REQUEST_GET, hnd_get_pot);
        coap_add_resource(ctx, r_pot);

        // 2. ลงทะเบียน Resource: actuator/led (PUT)
        coap_str_const_t *r_led_uri = coap_make_str_const("actuator/led");
        coap_resource_t *r_led = coap_resource_init(r_led_uri, 0);
        coap_register_handler(r_led, COAP_REQUEST_PUT, hnd_put_led);
        coap_add_resource(ctx, r_led);

        ESP_LOGI("CoAP", "CoAP Server started on port %d!", COAP_DEFAULT_PORT);

        // วนลูปประมวลผล I/O ของ CoAP
        while (1) {
            coap_io_process(ctx, 100);
        }

        coap_free_context(ctx);
    }
    vTaskDelete(NULL);
}
```

---

### กิจกรรมที่ 10-3.4 การเชื่อมโยงระบบ Wi-Fi และฟังก์ชัน `app_main(void)`

ในกิจกรรมนี้ จะเป็นการประกอบระบบทั้งหมดเข้าด้วยกัน โดยมีขั้นตอนสำคัญใน `app_main()` ดังนี้
1. เริ่มต้นระบบหน่วยความจำแฟลช **NVS (Non-Volatile Storage)** ซึ่งจำเป็นสำหรับโมดูล Wi-Fi Driver
2. เริ่มต้น **LwIP TCP/IP Stack** และ **Default Event Loop**
3. กำหนดค่าฮาร์ดแวร์ **GPIO 2 (LED)** เป็นโหมด Input/Output และ **ADC1 Channel 6 (GPIO 34)** สำหรับอ่านค่า Potentiometer
4. เชื่อมต่อเครือข่าย Wi-Fi ในโหมด **Station (STA)** ไปยัง Access Point
5. เมื่อเชื่อมต่อ Wi-Fi และได้รับหมายเลข IP สำเร็จ ให้สร้าง FreeRTOS Task เพื่อรัน **`coap_server_task`** (กำหนด Stack Size ขนาดอย่างน้อย `8192` ไบต์ เพื่อรองรับการทำงานของ `libcoap`)

#### 1. ฟังก์ชันเชื่อมต่อ Wi-Fi Station (`wifi_init_sta`)
```c
static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

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

    ESP_LOGI(TAG, "Connecting to AP: %s...", CONFIG_WIFI_SSID);

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}
```

#### 2. ฟังก์ชันหลัก `app_main(void)`
```c
void app_main(void)
{
    // 1. Initialise NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Netif & Event Loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 3. Setup Hardware (LED & ADC)
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_INPUT_OUTPUT);

    adc_oneshot_unit_init_cfg_t init_config1 = { .unit_id = ADC_UNIT_1 };
    if (adc_oneshot_new_unit(&init_config1, &s_adc1_handle) == ESP_OK) {
        adc_oneshot_chan_cfg_t chan_config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_12,
        };
        adc_oneshot_config_channel(s_adc1_handle, POT_ADC_CHANNEL, &chan_config);
        ESP_LOGI(TAG, "ADC Initialized on GPIO 34");
    }

    // 4. Connect Wi-Fi
    if (wifi_init_sta()) {
        // 5. Create CoAP Server Task (Stack size 8192 ไบต์)
        xTaskCreate(coap_server_task, "coap_server", 8192, NULL, 5, NULL);
        ESP_LOGI(TAG, "Ready! Test CoAP with: python test_coap.py");
    } else {
        ESP_LOGE(TAG, "Wi-Fi connection failed.");
    }
}
```

### ตารางสรุป Header Files และหน้าที่การทำงาน

| Header file | หน้าที่และขอบเขตการใช้งานในแล็บนี้ |
| :--- | :--- |
| `stdio.h` / `string.h` | จัดการ Input/Output และสตริง (`snprintf()`, `strlen()`, `memcpy()`) |
| `esp_log.h` | ส่งข้อความ Logging สถานะของระบบ (`ESP_LOGI()`, `ESP_LOGE()`) |
| `nvs_flash.h` | จัดการ Non-Volatile Storage สำหรับระบบ Wi-Fi |
| `esp_netif.h` / `esp_event.h` | จัดการ TCP/IP Network Interface และ Default Event Loop |
| `esp_wifi.h` | จัดการการเชื่อมต่อวิทยุ Wi-Fi Station |
| `freertos/FreeRTOS.h` / `task.h` | จัดการ FreeRTOS Tasks สำหรับรัน CoAP Server Task (`xTaskCreate()`) |
| `coap3/coap.h` | ไลบรารีทางการของ CoAP (libcoap v4.3) สำหรับสร้าง Context, Resources, และ PDU |
| `driver/gpio.h` | ควบคุมขาหลอดไฟ LED (GPIO 2) |
| `esp_adc/adc_oneshot.h` | อ่านค่าแรงดันแอนะล็อกจาก Potentiometer (GPIO 34 / ADC1 Channel 6) |

<details>
<summary><b>🔍 คลิกดูซอร์สโค้ดฉบับสมบูรณ์ทั้งไฟล์ (Lab10-3_CoAP_Server.c)</b></summary>

```c
#include <stdio.h>
#include <string.h>
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

#define TAG "COAP_LAB"

// กำหนดชื่อและรหัสผ่าน Wi-Fi
#define CONFIG_WIFI_SSID      "YOUR_WIFI_SSID"
#define CONFIG_WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
#define MAXIMUM_RETRY         5

#define LED_GPIO_PIN          GPIO_NUM_2
#define POT_ADC_CHANNEL       ADC_CHANNEL_6 // GPIO 34 (ADC1 Channel 6)

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1

static int s_retry_num = 0;
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying Wi-Fi (%d/%d)...", s_retry_num, MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected! IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

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

    ESP_LOGI(TAG, "Connecting to AP: %s...", CONFIG_WIFI_SSID);

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

// 1. Handler สำหรับ GET /sensor/pot
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
    ESP_LOGI(TAG, "GET /sensor/pot -> %s", pot_str);
}

// 2. Handler สำหรับ PUT /actuator/led
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
            ESP_LOGI(TAG, "LED turned ON via CoAP PUT");
        } else if (data[0] == '0') {
            gpio_set_level(LED_GPIO_PIN, 0);
            ESP_LOGI(TAG, "LED turned OFF via CoAP PUT");
        }
    }
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED); // 2.04 Changed
}

// CoAP Server Task
static void coap_server_task(void *pvParameters)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;

    while (1) {
        coap_address_init(&serv_addr);
        serv_addr.addr.sin.sin_family = AF_INET;
        serv_addr.addr.sin.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.addr.sin.sin_port = htons(COAP_DEFAULT_PORT);

        ctx = coap_new_context(NULL);
        if (!ctx) {
            ESP_LOGE(TAG, "coap_new_context() failed");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        coap_endpoint_t *ep = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);
        if (!ep) {
            ESP_LOGE(TAG, "coap_new_endpoint() failed");
            coap_free_context(ctx);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // ลงทะเบียน Resources
        coap_str_const_t *r_pot_uri = coap_make_str_const("sensor/pot");
        coap_resource_t *r_pot = coap_resource_init(r_pot_uri, 0);
        coap_register_handler(r_pot, COAP_REQUEST_GET, hnd_get_pot);
        coap_add_resource(ctx, r_pot);

        coap_str_const_t *r_led_uri = coap_make_str_const("actuator/led");
        coap_resource_t *r_led = coap_resource_init(r_led_uri, 0);
        coap_register_handler(r_led, COAP_REQUEST_PUT, hnd_put_led);
        coap_add_resource(ctx, r_led);

        ESP_LOGI(TAG, "CoAP Server listening on port %d...", COAP_DEFAULT_PORT);

        while (1) {
            coap_io_process(ctx, 100);
        }

        coap_free_context(ctx);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    // 1. Initialise NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Netif & Event Loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 3. Setup Hardware (LED & ADC)
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_INPUT_OUTPUT);

    adc_oneshot_unit_init_cfg_t init_config1 = { .unit_id = ADC_UNIT_1 };
    if (adc_oneshot_new_unit(&init_config1, &s_adc1_handle) == ESP_OK) {
        adc_oneshot_chan_cfg_t chan_config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_12,
        };
        adc_oneshot_config_channel(s_adc1_handle, POT_ADC_CHANNEL, &chan_config);
        ESP_LOGI(TAG, "ADC Initialized on GPIO 34");
    }

    // 4. Connect Wi-Fi
    if (wifi_init_sta()) {
        // 5. Create CoAP Server Task (ใช้ Stack Size 8192 ไบต์)
        xTaskCreate(coap_server_task, "coap_server", 8192, NULL, 5, NULL);
        ESP_LOGI(TAG, "Ready! Test CoAP with: python test_coap.py");
    } else {
        ESP_LOGE(TAG, "Wi-Fi connection failed.");
    }
}
```
</details>

#### คำสั่ง Build และ Flash โปรเจกต์
```powershell
# คอมไพล์โปรเจกต์ผ่าน Docker
docker run --rm --mount "type=bind,source=$((Get-Location).Path),target=/workspace" -w /workspace espressif/idf:release-v6.1 idf.py build

# แฟลชลงบอร์ด ESP32
python -m esptool -p <COMxx> --chip esp32 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 2MB --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/Lab10-3_CoAP_Server.bin
```

---

### กิจกรรมที่ 10-3.5 การทดสอบด้วย Python aiocoap Script บนคอมพิวเตอร์

#### 1. ติดตั้งไลบรารี aiocoap
เปิด PowerShell บนเครื่องคอมพิวเตอร์
```powershell
pip install aiocoap
```

#### 2. สคริปต์ทดสอบ CoAP Client (`test_coap.py`)
สร้างไฟล์ `test_coap.py` บนเครื่องคอมพิวเตอร์ เพื่อทดสอบ 3 การทำงานหลัก
1. **Resource Discovery (`/.well-known/core`)** ค้นหารายการ Resource ทั้งหมดบน ESP32
2. **GET `/sensor/pot`** อ่านค่าเซนเซอร์อนาล็อก
3. **PUT `/actuator/led`** สั่งเปิด-ปิดหลอดไฟ LED

```python
import asyncio
from aiocoap import Context, Message, Code

ESP32_IP = "192.168.1.181"  # แก้ไขให้ตรงกับหมายเลข IP ของ ESP32

async def main():
    protocol = await Context.create_client_context()
    
    # 1. ทดสอบ Resource Discovery: GET /.well-known/core
    print("\n--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---")
    request_core = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/.well-known/core")
    response_core = await protocol.request(request_core).response
    print("Resource Directory (CoRE Link Format):")
    print(response_core.payload.decode("utf-8"))

    # 2. ทดสอบอ่านค่าเซนเซอร์: GET /sensor/pot
    print("\n--- 2. Testing CoAP GET /sensor/pot ---")
    request_pot = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/sensor/pot")
    response_pot = await protocol.request(request_pot).response
    print(f"Potentiometer Value: {response_pot.payload.decode('utf-8')} (Code: {response_pot.code})")

    # 3. ทดสอบสั่งเปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '1'
    print("\n--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---")
    request_on = Message(code=Code.PUT, payload=b"1", uri=f"coap://{ESP32_IP}/actuator/led")
    response_on = await protocol.request(request_on).response
    print(f"LED ON Response Code: {response_on.code}")

    await asyncio.sleep(2)

    # 4. ทดสอบสั่งปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '0'
    print("\n--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---")
    request_off = Message(code=Code.PUT, payload=b"0", uri=f"coap://{ESP32_IP}/actuator/led")
    response_off = await protocol.request(request_off).response
    print(f"LED OFF Response Code: {response_off.code}")

if __name__ == "__main__":
    asyncio.run(main())
```

---

## 4. บันทึกผลการทดลองและคำถามท้ายบท (Lab Report & Questions)
1. นำผลการ Query `/.well-known/core` มาแสดงในรายงาน พร้อมอธิบายรูปแบบ **CoRE Link Format (RFC 6690)** ว่าแสดงข้อมูลทรัพยากรอย่างไร
2. อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง **CON (Confirmable)** และ **NON (Non-confirmable)** เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ
3. ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?

---

## รายงานผลการทดลอง (67030011)

**โค้ด:** [HW-67030011/Lab10-3_CoAP_Server/](HW-67030011/Lab10-3_CoAP_Server/) — Build ด้วย ESP-IDF v6.0.2 ผ่าน (`espressif/coap`), รหัส Wi-Fi แยกไว้ใน `main/wifi_credentials.h`

**สิ่งที่เพิ่มจากโค้ดตัวอย่าง:** โค้ดเดิมลงทะเบียน Resource โดยไม่มีแอตทริบิวต์ ผล Discovery จะได้แค่ Path เปล่าๆ จึงเพิ่ม `coap_add_attr()` ตาม RFC 6690:
```c
coap_add_attr(r_pot, coap_make_str_const("ct"), coap_make_str_const("0"), 0);
coap_add_attr(r_pot, coap_make_str_const("rt"), coap_make_str_const("\"sensor.pot\""), 0);
coap_add_attr(r_pot, coap_make_str_const("if"), coap_make_str_const("\"core.s\""), 0);
coap_add_attr(r_led, coap_make_str_const("rt"), coap_make_str_const("\"actuator.led\""), 0);
coap_add_attr(r_led, coap_make_str_const("if"), coap_make_str_const("\"core.a\""), 0);
```

**หน่วยความจำ (`idf.py size`):** DRAM แบบ static 48,439 ไบต์ (26.8%) มากที่สุดในสามโปรเจกต์ (มากกว่า UDP 12,976 ไบต์) จาก libcoap และ mbedTLS DTLS ที่เปิดใน `sdkconfig.defaults`

**ขนาดแพ็กเก็ตจริง (Encode ด้วย aiocoap):**
```text
PUT coap://<ip>/actuator/led payload "1"  (token 2 ไบต์) = 21 ไบต์
42 03 12 34 | 01 02 | b8 61 63 74 75 61 74 6f 72 | 03 6c 65 64 | ff | 31
Header        Token   Uri-Path "actuator" (1+8)    "led" (1+3)  Marker Payload
ACK 2.04 Changed (token 2 ไบต์) = 6 ไบต์ : 62 44 12 34 01 02
```

### คำตอบคำถามท้ายบท

1. **ผลการ Query `/.well-known/core` และรูปแบบ CoRE Link Format (RFC 6690)**
   * **คำตอบ:** เมื่อมีแอตทริบิวต์ข้างต้น libcoap จะตอบ `GET /.well-known/core` ด้วย Content-Format `40` (`application/link-format`) ในลักษณะนี้ (ลำดับของ Link และ Attribute อาจสลับได้ตามการทำงานของ libcoap):
     ```text
     </actuator/led>;rt="actuator.led";if="core.a",</sensor/pot>;ct=0;rt="sensor.pot";if="core.s"
     ```
     * แต่ละ Resource คือ **Link** หนึ่งตัว ครอบ URI ด้วย `< >` และคั่นแต่ละ Link ด้วย `,`
     * ตามด้วย **Attribute** คั่นด้วย `;` เช่น `ct` = Content-Format ที่ Resource ตอบ (`0` = `text/plain`), `rt` = Resource Type (ความหมายเชิงแอปพลิเคชัน), `if` = Interface Description (`core.s` = Sensor อ่านอย่างเดียว, `core.a` = Actuator) และอาจมี `obs` (รองรับ Observe) หรือ `sz` (ขนาดโดยประมาณ)
     * ไคลเอนต์กรองได้ด้วย Query เช่น `GET /.well-known/core?rt=sensor.pot` จึงค้นหาบริการได้เองโดยไม่ต้องรู้ URL ล่วงหน้า คล้าย mDNS-SD แต่อยู่ในระดับ Resource แทนระดับอุปกรณ์

2. **ความแตกต่างระหว่าง CON และ NON เมื่อทดสอบในเครือข่ายที่มีการรบกวน**
   * **คำตอบ:**
     | | CON (Confirmable, Type 0) | NON (Non-confirmable, Type 1) |
     | :--- | :--- | :--- |
     | การยืนยัน | ผู้รับต้องตอบ **ACK** ที่มี Message ID เดียวกัน | ไม่มี ACK |
     | เมื่อแพ็กเก็ตหาย | ส่งซ้ำอัตโนมัติด้วย **Exponential Back-off**: เริ่ม `ACK_TIMEOUT` 2 s × สุ่ม 1–1.5 แล้วเพิ่มเท่าตัวทุกครั้ง สูงสุด `MAX_RETRANSMIT` = 4 ครั้ง (รอได้นานสุดราว 45 วินาที) | หายแล้วหายเลย แอปพลิเคชันต้องจัดการเอง |
     | ผลในเครือข่ายที่มีสัญญาณรบกวน | ข้อมูลไปถึงแน่นอนกว่า แต่ RTT กระโดดเป็นช่วงๆ (Jitter สูง) และเกิด Traffic ส่งซ้ำ | Latency ต่ำและคงที่ แต่ Loss Rate เท่ากับอัตราสูญหายของ Wi-Fi |
     | การใช้งานที่เหมาะ | คำสั่งควบคุม เช่น `PUT /actuator/led` ที่ต้องได้ผลแน่นอน | Telemetry ที่ส่งถี่และค่าใหม่มาแทนค่าเก่าได้ |
     
     ผู้รับใช้ Message ID ตรวจจับแพ็กเก็ตซ้ำ (Deduplication) จึงไม่ทำคำสั่งซ้ำ แม้ ACK จะหายแล้วผู้ส่งยิงซ้ำมา

3. **ทำไม CoAP จึงเหมาะกับ Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?**
   * **คำตอบ:**
     * **ขนาดเฟรมเล็ก:** เครือข่ายเหล่านี้ใช้ IEEE 802.15.4 ที่เฟรมใหญ่สุดเพียง **127 ไบต์** (หลังบีบอัดด้วย 6LoWPAN เหลือที่ให้ Payload น้อยมาก) คำสั่ง CoAP ข้างต้นทั้งก้อน 21 ไบต์ส่งได้ในเฟรมเดียว แต่ HTTP Request 152 ไบต์ + TCP Header ต้องแบ่ง Fragment หลายเฟรม เสี่ยงหายทั้งชุดถ้าเสียไปเฟรมเดียว
     * **ไม่ต้องมี Connection:** CoAP อยู่บน UDP ไม่ต้องทำ TCP Handshake ที่เพิ่ม RTT (ใน NB-IoT หนึ่ง RTT อาจหลายร้อย ms ถึงวินาที) และไม่ต้องเก็บ TCP State บนอุปกรณ์ RAM น้อย
     * **ประหยัดพลังงาน:** ส่งน้อยไบต์และน้อยรอบ วิทยุจึงเปิดสั้นลง อุปกรณ์กลับไปหลับ (PSM/eDRX ของ NB-IoT, Sleepy End Device ของ Thread) ได้เร็ว
     * **ออกแบบมาเพื่อ Lossy Network:** มี Reliability ของตัวเองแบบเลือกได้ (CON/NON), รองรับ **Multicast** (ค้นหา/สั่งงานเป็นกลุ่ม), **Observe** (RFC 7641) ให้ Server Push ค่าได้โดยไม่ต้อง Poll และ **Block-wise Transfer** (RFC 7959) สำหรับข้อมูลใหญ่
     * **ยังคงรูปแบบ REST:** มี GET/PUT/POST/DELETE และ Response Code แบบเดียวกับ HTTP จึงแปลงผ่าน HTTP-CoAP Proxy ไปเชื่อมกับเว็บได้ง่าย
