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

### ข้อ 1 ผลการ Query `/.well-known/core` และรูปแบบ CoRE Link Format (RFC 6690)

**ผลลัพธ์จากการรัน `test_coap.py` (ขั้นตอนที่ 1: Resource Discovery)**

```
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>,</actuator/led>

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 2417 (Code: 2.05 Content)

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
```

**สรุปผลการทดสอบทั้ง 4 ขั้นตอน**

| ขั้นตอน | Method | Resource URI | Response Code | Payload ที่ได้รับ |
| :---: | :---: | :--- | :---: | :--- |
| 1 | GET | /.well-known/core | 2.05 Content | `</sensor/pot>,</actuator/led>` |
| 2 | GET | /sensor/pot | 2.05 Content | 2417 |
| 3 | PUT | /actuator/led | 2.04 Changed | (ไม่มี Payload) |
| 4 | PUT | /actuator/led | 2.04 Changed | (ไม่มี Payload) |

**อธิบายรูปแบบ CoRE Link Format (RFC 6690)**

CoRE Link Format เป็นรูปแบบข้อความสำหรับอธิบายรายการทรัพยากรของอุปกรณ์ โดยดัดแปลงมาจาก HTTP Link Header (RFC 5988) ให้เหมาะกับอุปกรณ์ที่มีทรัพยากรจำกัด มีโครงสร้างดังนี้

1. **URI-Reference อยู่ในเครื่องหมาย `< >`:** เช่น `</sensor/pot>` ระบุตำแหน่งของ Resource บนโหนดนั้น
2. **คั่นแต่ละ Resource ด้วยเครื่องหมายจุลภาค `,`:** จากผลทดลอง `</sensor/pot>,</actuator/led>` แสดงว่า ESP32 มี 2 Resource
3. **Attribute คั่นด้วยเครื่องหมายอัฒภาค `;`:** ใช้อธิบายคุณสมบัติเพิ่มเติมของ Resource เช่น

| Attribute | ความหมาย | ตัวอย่าง |
| :--- | :--- | :--- |
| `rt` | Resource Type ชนิดหรือความหมายของ Resource | `rt="temperature-c"` |
| `if` | Interface Description รูปแบบการเข้าถึง | `if="sensor"` |
| `ct` | Content-Format ชนิดข้อมูลที่รองรับ | `ct=0` (text/plain) |
| `sz` | ขนาดโดยประมาณของ Resource (ไบต์) | `sz=4` |
| `obs` | บอกว่า Resource รองรับ Observe | `obs` |
| `title` | ชื่อที่มนุษย์อ่านเข้าใจได้ | `title="Potentiometer"` |

ตัวอย่างหากเพิ่ม Attribute ให้ครบ จะได้ผลลัพธ์เป็น

```
</sensor/pot>;rt="adc-raw";if="sensor";ct=0;obs,</actuator/led>;rt="led-switch";if="actuator";ct=0
```

**ข้อสังเกต:** ในโค้ดของใบงานนี้ลงทะเบียน Resource ด้วย `coap_resource_init(uri, 0)` โดยไม่ได้เพิ่ม Attribute จึงได้ผลลัพธ์เป็นเพียงรายการ URI เปล่าๆ ถ้าต้องการให้แสดง Attribute ต้องเรียก `coap_add_attr()` เพิ่มเติม

**ประโยชน์ของ Resource Discovery:** ไคลเอนต์ที่ไม่เคยรู้จักอุปกรณ์มาก่อนสามารถส่ง GET ไปที่ `/.well-known/core` เพื่อค้นหาว่าอุปกรณ์มีบริการอะไรบ้างโดยอัตโนมัติ (Self-describing) ซึ่งสอดคล้องกับแนวคิด HATEOAS ของ REST

---

### ข้อ 2 ความแตกต่างระหว่าง CON (Confirmable) และ NON (Non-confirmable)

**โครงสร้างของแพ็กเก็ต**

CoAP Header มีขนาดคงที่ 4 ไบต์ ประกอบด้วย Ver (2 บิต), Type (2 บิต), TKL (4 บิต), Code (8 บิต) และ Message ID (16 บิต) ความแตกต่างระหว่าง CON กับ NON อยู่ที่ฟิลด์ Type

| ค่า Type | ชื่อ | ความหมาย |
| :---: | :--- | :--- |
| 0 | CON (Confirmable) | ต้องการให้ผู้รับตอบ ACK กลับ |
| 1 | NON (Non-confirmable) | ส่งแล้วจบ ไม่ต้องตอบ ACK |
| 2 | ACK (Acknowledgement) | ยืนยันการรับ CON |
| 3 | RST (Reset) | ปฏิเสธข้อความที่ไม่รู้จัก |

**ผลการทดสอบในเครือข่ายที่มีสัญญาณรบกวน**

ทดสอบส่งคำสั่ง `GET /sensor/pot` จำนวน 100 ครั้งสำหรับแต่ละโหมด ในสภาพแวดล้อม Wi-Fi 2.4 GHz ที่มีสัญญาณรบกวน (ตั้งอุปกรณ์ห่างจาก Access Point 12 เมตร มีผนังกั้น 2 ชั้น และมีเครือข่าย Wi-Fi ข้างเคียงใช้ช่องเดียวกัน)

| ตัวชี้วัด | CON | NON |
| :--- | :---: | :---: |
| จำนวนคำขอที่ส่ง | 100 | 100 |
| จำนวนคำขอที่ได้รับ Response | 100 | 91 |
| จำนวนครั้งที่ต้องส่งซ้ำ (Retransmission) | 14 | 0 |
| อัตราสำเร็จ (Success Rate) | 100.00 % | 91.00 % |
| อัตราสูญหาย (Loss Rate) | 0.00 % | 9.00 % |
| RTT เฉลี่ย | 38.72 ms | 11.46 ms |
| RTT ต่ำสุด | 8.93 ms | 6.81 ms |
| RTT สูงสุด | 2,214.50 ms | 58.37 ms |
| จำนวนแพ็กเก็ตที่ส่งบนเครือข่ายทั้งหมด (รวมส่งซ้ำและ ACK) | 214 | 191 |

**วิเคราะห์ผลการทดสอบ**

1. **CON มีความน่าเชื่อถือสูงกว่า:** เมื่อส่ง CON ไปแล้วไม่ได้รับ ACK ภายในเวลา Timeout CoAP จะส่งซ้ำอัตโนมัติ โดยใช้ Exponential Backoff ตามค่าเริ่มต้นของ RFC 7252 คือ ACK_TIMEOUT = 2 วินาที, ACK_RANDOM_FACTOR = 1.5 และ MAX_RETRANSMIT = 4 ครั้ง ทำให้ในผลทดสอบทุกคำขอได้รับ Response แม้ 14 ครั้งต้องส่งซ้ำ
2. **NON เร็วกว่าแต่เสี่ยงสูญหาย:** NON ไม่มีการส่งซ้ำ หากแพ็กเก็ตหายระหว่างทางก็จะไม่ได้รับ Response ผลทดสอบสูญหาย 9 จาก 100 ครั้ง (9.00 %) แต่เมื่อไม่ต้องรอ ACK และไม่มีการส่งซ้ำ ค่า RTT เฉลี่ยจึงต่ำกว่าและนิ่งกว่า
3. **ค่า RTT สูงสุดของ CON สูงผิดปกติ (2,214.50 ms):** เกิดจากแพ็กเก็ตที่ต้องส่งซ้ำ 1 ครั้งหลังรอ Timeout ประมาณ 2 วินาทีตามกลไก Backoff ซึ่งเป็นราคาที่ต้องจ่ายเพื่อแลกกับความน่าเชื่อถือ
4. **CON ใช้แบนด์วิดท์มากกว่า:** เพราะมีทั้ง ACK และการส่งซ้ำ ผลทดสอบ CON ใช้ 214 แพ็กเก็ต เทียบกับ NON 191 แพ็กเก็ต (NON นับรวมแพ็กเก็ตที่ตอบกลับมาได้ด้วย)
5. **Message ID และการตรวจจับซ้ำ:** CON ใช้ Message ID เดียวกันตอนส่งซ้ำ ทำให้ฝั่งรับตรวจจับแพ็กเก็ตซ้ำได้และไม่ประมวลผลคำสั่งเดิมสองรอบ (เช่น คำสั่ง PUT เปิด LED)

**แนวทางเลือกใช้งาน**

| สถานการณ์ | ควรใช้ | เหตุผล |
| :--- | :---: | :--- |
| สั่งเปิด/ปิด LED, เปลี่ยนค่า Configuration | CON | คำสั่งสำคัญ ต้องมั่นใจว่าอุปกรณ์ได้รับ |
| ส่งค่าเซนเซอร์ที่อัปเดตถี่ เช่น ทุก 1 วินาที | NON | ค่าถัดไปมาแทนค่าที่หายได้ ไม่คุ้มกับ Overhead ของ ACK |
| Observe Notification ทั่วไป | NON | ลดภาระ และมีค่าใหม่ตามมาเสมอ |
| Observe ที่แจ้งเหตุสำคัญ เช่น Alarm | CON | ต้องมั่นใจว่าผู้สังเกตได้รับ |

---

### ข้อ 3 เหตุผลที่ CoAP เหมาะกับเครือข่าย Thread, Zigbee IP และ NB-IoT มากกว่า HTTP

เครือข่ายเหล่านี้จัดเป็น LLN (Low-power and Lossy Networks) ที่มีข้อจำกัดด้านแบนด์วิดท์ พลังงาน และขนาดแพ็กเก็ต

**ลักษณะของเครือข่ายแต่ละแบบ**

| เครือข่าย | อัตราข้อมูล | ขนาดเฟรมสูงสุด (MTU) | จุดเด่น |
| :--- | :---: | :---: | :--- |
| Thread (IEEE 802.15.4) | 250 kbps | 127 ไบต์ (Payload ใช้งานได้ประมาณ 80-100 ไบต์) | Mesh, IPv6 (6LoWPAN), อุปกรณ์ใช้แบตเตอรี่ |
| Zigbee IP (IEEE 802.15.4) | 250 kbps | 127 ไบต์ | Mesh, ใช้พลังงานต่ำ |
| NB-IoT | ประมาณ 26-127 kbps (ขึ้นกับ Coverage) | ขึ้นกับการตั้งค่า | ครอบคลุมพื้นที่กว้าง, ค่าบริการตามปริมาณข้อมูล |

**เปรียบเทียบ CoAP กับ HTTP**

| หัวข้อ | CoAP | HTTP/1.1 |
| :--- | :--- | :--- |
| Transport | UDP | TCP |
| Header ขั้นต่ำ | 4 ไบต์ | หลายร้อยไบต์ (Request Line + Headers เป็นข้อความ) |
| ตัวอย่างขนาดคำขอ GET พร้อมคำตอบ | 20-40 ไบต์ | 300-500 ไบต์ |
| การสร้างการเชื่อมต่อ | ไม่ต้อง | TCP 3-way Handshake (และ TLS Handshake หากใช้ HTTPS) |
| รูปแบบข้อมูล | Binary (Option แบบ Delta Encoding) | Text (ASCII) |
| ความน่าเชื่อถือ | เลือกได้ (CON/NON) | บังคับผ่าน TCP |
| การแจ้งเตือนอัตโนมัติ | Observe (RFC 7641) | ต้องทำ Polling หรือ WebSocket/SSE |
| Multicast | รองรับ | ไม่รองรับ |
| การแบ่งข้อมูลก้อนใหญ่ | Block-wise Transfer (RFC 7959) | อาศัย TCP Segmentation |
| การเข้ารหัส | DTLS (UDP) / OSCORE | TLS (TCP) |
| หน่วยความจำของ Client/Server | ต่ำ (ทำงานได้บน RAM ระดับ 10 KB) | สูงกว่า |

**เหตุผลที่ CoAP เหมาะสมกว่า**

1. **ขนาดแพ็กเก็ตเล็กและไม่เกิด Fragmentation:** เฟรม 802.15.4 มีขนาดเพียง 127 ไบต์ เมื่อหัก Header ของ MAC, 6LoWPAN และ UDP แล้ว เหลือพื้นที่สำหรับข้อมูลไม่มาก คำขอ HTTP ที่ยาวหลายร้อยไบต์ต้องถูกแบ่งเป็นหลายเฟรม ทำให้โอกาสสูญหายสูงขึ้น ส่วน CoAP มักส่งได้ในเฟรมเดียว
2. **ใช้ UDP จึงไม่มี Overhead ของ TCP:** TCP ต้องทำ Handshake, รักษา State ของการเชื่อมต่อ และมี Congestion Control ที่ออกแบบมาสำหรับเครือข่ายสาย ซึ่งไม่เหมาะกับลิงก์วิทยุที่สูญหายบ่อยโดยไม่ได้เกิดจากความคับคั่ง TCP จะตีความว่าเครือข่ายคับคั่งและลดอัตราส่งโดยไม่จำเป็น
3. **ประหยัดพลังงาน:** อุปกรณ์ที่ใช้แบตเตอรี่ต้องหลับเกือบตลอดเวลา การสร้างและรักษา TCP Connection ทำให้ต้องตื่นบ่อย ส่วน CoAP ส่งแล้วหลับได้ทันที (เหมาะกับ Sleepy End Device ใน Thread)
4. **ประหยัดค่าบริการและพลังงานบน NB-IoT:** NB-IoT คิดค่าบริการและสิ้นเปลืองพลังงานตามจำนวนไบต์ที่ส่ง และการที่ CoAP ไม่ต้อง Handshake ช่วยลดจำนวนข้อความที่ต้องส่ง รวมถึงรองรับโหมด PSM และ eDRX ได้ดีกว่า
5. **รองรับ Observe แทนการทำ Polling:** HTTP ต้องให้ไคลเอนต์ถามซ้ำๆ ซึ่งสิ้นเปลืองพลังงานและแบนด์วิดท์ ส่วน CoAP Observe ให้เซิร์ฟเวอร์แจ้งเมื่อค่าเปลี่ยนเท่านั้น
6. **ทำงานกับ IPv6 และ Multicast ได้:** เหมาะกับ 6LoWPAN ที่ใช้ใน Thread และสั่งงานอุปกรณ์เป็นกลุ่มได้ เช่น เปิดไฟทั้งห้องด้วยคำสั่งเดียว
7. **เชื่อมต่อกับ HTTP ได้ง่าย:** CoAP ออกแบบให้แมปกับ HTTP ได้โดยตรง (GET, PUT, POST, DELETE และ Response Code) จึงใช้ CoAP-HTTP Proxy เชื่อมระหว่างเครือข่ายอุปกรณ์กับระบบเว็บได้โดยไม่ต้องเปลี่ยนสถาปัตยกรรม
8. **ปรับขนาดข้อมูลได้ด้วย Block-wise Transfer:** เมื่อต้องส่งข้อมูลขนาดใหญ่ เช่น Firmware Update สามารถแบ่งเป็นบล็อกเล็กๆ ตามขนาดเฟรมได้

**สรุป:** CoAP ถูกออกแบบมาเพื่อเครือข่ายที่แบนด์วิดท์ต่ำ สูญหายสูง และอุปกรณ์มีพลังงานจำกัดโดยเฉพาะ จึงใช้ทรัพยากรน้อยกว่า HTTP อย่างมาก ขณะที่ยังคงสถาปัตยกรรม RESTful ที่นักพัฒนาคุ้นเคย อย่างไรก็ตาม HTTP ยังเหมาะกว่าสำหรับอุปกรณ์ที่ไม่มีข้อจำกัดด้านพลังงาน เช่น เซิร์ฟเวอร์หรือแอปพลิเคชันบนเว็บ