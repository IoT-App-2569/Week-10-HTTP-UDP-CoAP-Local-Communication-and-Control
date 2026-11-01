# ใบงานการทดลองที่ 10.2
### การสื่อสารความหน่วงต่ำด้วย UDP Socket และการถ่ายทอดข้อมูล Real-time Telemetry

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้ นักศึกษาจะได้เรียนรู้การเขียนโปรแกรมเครือข่ายระดับล่างด้วย **BSD Socket API** บนสแตก LwIP ของ ESP32 เพื่อสร้างบริการส่งข้อมูลเซนเซอร์แบบต่อเนื่องความถี่สูง (Real-time Telemetry Streaming) และการควบคุมอุปกรณ์ด้วยแพ็กเก็ต UDP ที่มีขนาด Header เล็กและตอบสนองได้รวดเร็วระดับมิลลิวินาที

---

## 1. วัตถุประสงค์การทดลอง
1. สามารถเขียนโปรแกรม Socket แบบ Connectionless (UDP Datagram) ด้วยคำสั่ง `socket()`, `bind()`, `recvfrom()`, และ `sendto()` บน ESP-IDF ได้
2. สามารถพัฒนา FreeRTOS Task เพื่อส่งข้อมูลแอนะล็อกเซนเซอร์แบบบรอดแคสต์ (UDP Broadcast) สู่เครือข่ายได้ด้วยความถี่ 10-50 Hz
3. สามารถพัฒนาสคริปต์ภาษา Python บนเครื่องคอมพิวเตอร์เพื่อดักฟังข้อมูลบรอดแคสต์ และส่งคำสั่งควบคุม LED กลับมายัง ESP32 ได้
4. สามารถวัดค่าความหน่วงเวลาเฉลี่ย (Round-Trip Latency) และอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) ได้

---

## 2. โครงสร้างระบบและการทำงาน

```
   [ESP32 Node]                                              [PC Client / Python]
         |                                                             |
         | --- (UDP Broadcast: pot_raw, seq_no) : Port 3334 ---------> | (รับค่าแสดงผลกราฟ)
         |                                                             |
         | <--- (UDP Unicast Command: "LED_ON" / "LED_OFF") : Port 3333| (ส่งคำสั่งควบคุม)
         | --- (UDP Unicast ACK: "STATUS:OK") ------------------------>| (วัด RTT Latency)
```

---

## 3. ขั้นตอนการทดลอง

### กิจกรรมที่ 10-2.1  การสร้างโปรเจกต์ใหม่และตั้งค่าโครงสร้าง

#### 1. สร้างโปรเจกต์ใหม่
```powershell
idf.py create-project Lab10-2_UDP_Telemetry_Socket
cd Lab10-2_UDP_Telemetry_Socket
```

**หรือรันผ่าน Docker**
```powershell
docker run --rm --mount "type=bind,source=$((Get-Location).Path),target=/workspace" -w /workspace espressif/idf:release-v6.1 idf.py create-project Lab10-2_UDP_Telemetry_Socket
cd Lab10-2_UDP_Telemetry_Socket
```

#### 2. กำหนด Target เป็นชิป ESP32
```powershell
idf.py set-target esp32
```

**หรือรันผ่าน Docker**
```powershell
docker run --rm --mount "type=bind,source=$((Get-Location).Path),target=/workspace" -w /workspace espressif/idf:release-v6.1 idf.py set-target esp32
```

#### 3. ตั้งค่า `main/CMakeLists.txt`
เปิดไฟล์ `main/CMakeLists.txt` และระบุคอมโพเนนต์ที่ต้องใช้งาน

```cmake
idf_component_register(SRCS "Lab10-2_UDP_Telemetry_Socket.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi esp_event nvs_flash lwip esp_adc esp_driver_gpio)
```

#### 4. ทดสอบ Reconfigure ระบบบิลด์
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

### กิจกรรมที่ 10-2.2 พัฒนา Task รับคำสั่ง UDP Control Server (Port 3333)
ฟังก์ชันนี้ทำหน้าที่เปิด UDP Socket ผูกเข้ากับพอร์ต 3333 (`INADDR_ANY`) เพื่อคอยรับคำสั่งควบคุม LED แบบ Unicast และตอบรับกลับ (ACK) เพื่อให้ไคลเอนต์นำไปคำนวณ Round-Trip Time (RTT)

```c
#include "lwip/sockets.h"

#define UDP_CONTROL_PORT 3333

void udp_control_server_task(void *pvParameters)
{
    char rx_buffer[128];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE("UDP_SERVER", "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_CONTROL_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE("UDP_SERVER", "Socket unable to bind: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI("UDP_SERVER", "Control Server listening on UDP Port %d...", UDP_CONTROL_PORT);

    while (1) {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&client_addr, &client_addr_len);
        if (len > 0) {
            rx_buffer[len] = '\0';
            ESP_LOGI("UDP_SERVER", "Received command: %s", rx_buffer);

            if (strcmp(rx_buffer, "LED_ON") == 0) {
                gpio_set_level(GPIO_NUM_2, 1);
                sendto(sock, "ACK:LED_ON", 10, 0, (struct sockaddr *)&client_addr, client_addr_len);
            } else if (strcmp(rx_buffer, "LED_OFF") == 0) {
                gpio_set_level(GPIO_NUM_2, 0);
                sendto(sock, "ACK:LED_OFF", 11, 0, (struct sockaddr *)&client_addr, client_addr_len);
            } else {
                sendto(sock, "ACK:UNKNOWN", 11, 0, (struct sockaddr *)&client_addr, client_addr_len);
            }
        }
    }
    close(sock);
    vTaskDelete(NULL);
}
```

---

### กิจกรรมที่ 10-2.3 พัฒนา Task ส่งข้อมูล Telemetry แบบ Broadcast (Port 3334)
ฟังก์ชันนี้ทำหน้าที่อ่านค่าแอนะล็อกเซนเซอร์ Potentiometer จาก ADC1 (GPIO 34) แล้วแพ็กข้อมูลร่วมกับหมายเลขลำดับ (Sequence Number) ส่งบรอดแคสต์ไปยัง `255.255.255.255` พอร์ต 3334 ทุกๆ 100 ms (10 Hz)

```c
#define UDP_BROADCAST_PORT 3334

void udp_telemetry_broadcast_task(void *pvParameters)
{
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_BROADCAST_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE("UDP_BCAST", "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    // เปิดใช้งานตัวเลือก SO_BROADCAST เพื่อให้อนุญาตส่งแพ็กเก็ตบรอดแคสต์
    int broadcast_enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    uint32_t seq_no = 0;
    char tx_buffer[64];

    ESP_LOGI("UDP_BCAST", "Starting Telemetry Streaming to port %d (10 Hz)...", UDP_BROADCAST_PORT);

    while (1) {
        int pot_val = 0;
        if (s_adc1_handle != NULL) {
            adc_oneshot_read(s_adc1_handle, ADC_CHANNEL_6, &pot_val);
        }

        snprintf(tx_buffer, sizeof(tx_buffer), "SEQ:%lu,POT:%d\n", (unsigned long)seq_no++, pot_val);

        int err = sendto(sock, tx_buffer, strlen(tx_buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE("UDP_BCAST", "Error occurred during sendto: errno %d", errno);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // หน่วงเวลา 100ms (10 Hz)
    }
    close(sock);
    vTaskDelete(NULL);
}
```

---

### กิจกรรมที่ 10-2.4 การเชื่อมโยงระบบ Wi-Fi และฟังก์ชัน `app_main()`

ในกิจกรรมนี้ จะเป็นการประกอบระบบทั้งหมดเข้าด้วยกัน โดยมีขั้นตอนสำคัญใน `app_main()` ดังนี้
1. เริ่มต้นระบบหน่วยความจำแฟลช **NVS (Non-Volatile Storage)** ซึ่งจำเป็นสำหรับโมดูล Wi-Fi Driver
2. เริ่มต้น **LwIP TCP/IP Stack** และ **Default Event Loop**
3. กำหนดค่าฮาร์ดแวร์ **GPIO 2 (LED)** เป็นโหมด Input/Output และ **ADC1 Channel 6 (GPIO 34)** สำหรับอ่านค่า Potentiometer
4. เชื่อมต่อเครือข่าย Wi-Fi ในโหมด **Station (STA)** ไปยัง Access Point
5. เมื่อเชื่อมต่อ Wi-Fi สำเร็จ ให้สร้าง FreeRTOS Tasks สำหรับรัน **`udp_control_server_task`** (รับคำสั่งพอร์ต 3333) และ **`udp_telemetry_broadcast_task`** (บรอดแคสต์ข้อมูลเซนเซอร์พอร์ต 3334 อัตรา 10 Hz)

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
        // 5. Create FreeRTOS Tasks for UDP
        xTaskCreate(udp_control_server_task, "udp_ctrl_task", 4096, NULL, 5, NULL);
        xTaskCreate(udp_telemetry_broadcast_task, "udp_telemetry_task", 4096, NULL, 5, NULL);
        ESP_LOGI(TAG, "Ready! Test UDP Telemetry with: python udp_listener.py");
    } else {
        ESP_LOGE(TAG, "Wi-Fi connection failed.");
    }
}
```

### ตารางสรุป Header Files และหน้าที่การทำงาน

| Header file                       | หน้าที่และขอบเขตการใช้งานในแล็บนี้                                                      |
| :-------------------------------- | :-------------------------------------------------------------------------------------- |
| `stdio.h` / `string.h`            | จัดการ Input/Output และฟังก์ชันจัดรูปแบบสตริง (`snprintf()`, `strlen()`, `memcpy()`)    |
| `esp_log.h`                       | ส่งข้อความแจ้งสถานะและตรวจแก้ข้อผิดพลาด (`ESP_LOGI()`, `ESP_LOGE()`)                    |
| `nvs_flash.h`                     | จัดการ Non-Volatile Storage สำหรับระบบ Wi-Fi                                            |
| `esp_netif.h` / `esp_event.h`     | จัดการ Network Interface Adapter และ Event Loop                                         |
| `esp_wifi.h`                      | จัดการการเชื่อมต่อวิทยุ Wi-Fi Station                                                   |
| `freertos/FreeRTOS.h` / `task.h`  | โครงสร้างระบบ FreeRTOS สำหรับสร้าง Task แบบมัลติทาสก์กิ้ง (`xTaskCreate()`)             |
| `lwip/sockets.h` / `lwip/netdb.h` | BSD Socket API บน LwIP (`socket()`, `bind()`, `sendto()`, `recvfrom()`, `setsockopt()`) |
| `driver/gpio.h`                   | ควบคุมระดับสัญญาณดิจิทัลเปิด-ปิดหลอดไฟ LED (GPIO 2)                                     |
| `esp_adc/adc_oneshot.h`           | อ่านค่าแรงดันแอนะล็อกจาก Potentiometer (GPIO 34 / ADC1 Channel 6)                       |

<details>
<summary><b>🔍 คลิกดูซอร์สโค้ดฉบับสมบูรณ์ทั้งไฟล์ (Lab10-2_UDP_Telemetry_Socket.c)</b></summary>

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
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define TAG "UDP_LAB"

// กำหนดชื่อและรหัสผ่าน Wi-Fi
#define CONFIG_WIFI_SSID      "YOUR_WIFI_SSID"
#define CONFIG_WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
#define MAXIMUM_RETRY         5

#define LED_GPIO_PIN          GPIO_NUM_2
#define POT_ADC_CHANNEL       ADC_CHANNEL_6 // GPIO 34 (ADC1 Channel 6)

#define UDP_CONTROL_PORT      3333
#define UDP_BROADCAST_PORT    3334

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

// Task 1: UDP Control Server (Port 3333)
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
    ESP_LOGI(TAG, "Control Server listening on UDP Port %d...", UDP_CONTROL_PORT);

    while (1) {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
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

// Task 2: UDP Telemetry Broadcast (Port 3334)
void udp_telemetry_broadcast_task(void *pvParameters)
{
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_BROADCAST_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    int broadcast_enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    uint32_t seq_no = 0;
    char tx_buffer[64];

    ESP_LOGI(TAG, "Broadcasting Telemetry to Port %d (10 Hz)...", UDP_BROADCAST_PORT);

    while (1) {
        int pot_val = 0;
        if (s_adc1_handle != NULL) {
            adc_oneshot_read(s_adc1_handle, POT_ADC_CHANNEL, &pot_val);
        }

        snprintf(tx_buffer, sizeof(tx_buffer), "SEQ:%lu,POT:%d\n", (unsigned long)seq_no++, pot_val);
        sendto(sock, tx_buffer, strlen(tx_buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz
    }
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
        // 5. Create FreeRTOS Tasks for UDP
        xTaskCreate(udp_control_server_task, "udp_ctrl_task", 4096, NULL, 5, NULL);
        xTaskCreate(udp_telemetry_broadcast_task, "udp_bcast_task", 4096, NULL, 4, NULL);
        ESP_LOGI(TAG, "All UDP Tasks started!");
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
python -m esptool -p <COMxx> --chip esp32 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 2MB --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/Lab10-2_UDP_Telemetry_Socket.bin
```

---

### กิจกรรมที่ 10-2.5 การทดสอบด้วยสคริปต์ Python บนคอมพิวเตอร์

#### 1. สคริปต์ดักฟังข้อมูล Telemetry Broadcast (`udp_listener.py`)
สร้างไฟล์ `udp_listener.py` บนเครื่องคอมพิวเตอร์เพื่อดักฟังข้อมูลบรอดแคสต์พอร์ต 3334 และตรวจสอบการสูญหายของแพ็กเก็ต (Packet Loss) จาก Sequence Number:

```python
import socket

UDP_PORT = 3334

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(("", UDP_PORT))

print(f"Listening for UDP Broadcast on port {UDP_PORT}...")

last_seq = None
packet_count = 0
lost_packets = 0

try:
    while True:
        data, addr = sock.recvfrom(128)
        msg = data.decode("utf-8").strip()
        packet_count += 1
        
        # ถอดรหัส SEQ และ POT
        parts = dict(item.split(":") for item in msg.split(","))
        current_seq = int(parts.get("SEQ", 0))
        pot_val = int(parts.get("POT", 0))

        if last_seq is not None:
            diff = current_seq - last_seq
            if diff > 1:
                lost = diff - 1
                lost_packets += lost
                print(f"[PACKET LOSS DETECTED] Lost {lost} packets! (Expected {last_seq + 1}, got {current_seq})")

        last_seq = current_seq
        print(f"[{addr[0]}] Seq: {current_seq:<6} | Potentiometer: {pot_val:<5} | Total Lost: {lost_packets}")

except KeyboardInterrupt:
    if packet_count > 0:
        loss_rate = (lost_packets / (packet_count + lost_packets)) * 100
        print(f"\n--- Statistics ---")
        print(f"Received: {packet_count} packets")
        print(f"Lost: {lost_packets} packets")
        print(f"Packet Loss Rate: {loss_rate:.2f}%")
```

#### 2. สคริปต์ทดสอบคำสั่งควบคุมและวัด RTT Latency (`udp_controller.py`)
สร้างไฟล์ `udp_controller.py` เพื่อส่งคำสั่งเปิด-ปิด LED และวัดเวลา Round-Trip Latency

```python
import socket
import time

ESP32_IP = "192.168.1.181"  # ระบุ IP ของบอร์ด ESP32
CMD_PORT = 3333

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.settimeout(2.0)

print(f"Sending control commands to {ESP32_IP}:{CMD_PORT}...")

commands = [b"LED_ON", b"LED_OFF"] * 5  # ส่งสลับ 10 ครั้ง
rtt_list = []

for i, cmd in enumerate(commands, 1):
    try:
        t_start = time.perf_counter()
        client.sendto(cmd, (ESP32_IP, CMD_PORT))
        resp, _ = client.recvfrom(128)
        t_end = time.perf_counter()
        
        rtt_ms = (t_end - t_start) * 1000
        rtt_list.append(rtt_ms)
        print(f"Round {i:02d}: Sent '{cmd.decode()}' -> Reply '{resp.decode()}' | RTT: {rtt_ms:.2f} ms")
    except socket.timeout:
        print(f"Round {i:02d}: Request timed out!")
    time.sleep(0.5)

if rtt_list:
    avg_rtt = sum(rtt_list) / len(rtt_list)
    print(f"\nAverage UDP RTT Latency: {avg_rtt:.2f} ms (Min: {min(rtt_list):.2f} ms, Max: {max(rtt_list):.2f} ms)")
```

---

## 4. บันทึกผลการทดลองและคำถามท้ายการทดลอง

### ข้อ 1 เปรียบเทียบค่า RTT Latency ระหว่าง UDP (ใบงาน 10.2) กับ HTTP RESTful (ใบงาน 10.1)

**ผลการทดลอง UDP (กิจกรรมที่ 10-2.5)**

| รอบที่ | คำสั่งที่ส่ง | ข้อความตอบกลับ | RTT (ms) |
| :---: | :--- | :--- | :---: |
| 01 | LED_ON | ACK:LED_ON | 8.42 |
| 02 | LED_OFF | ACK:LED_OFF | 5.17 |
| 03 | LED_ON | ACK:LED_ON | 4.89 |
| 04 | LED_OFF | ACK:LED_OFF | 12.35 |
| 05 | LED_ON | ACK:LED_ON | 5.03 |
| 06 | LED_OFF | ACK:LED_OFF | 6.21 |
| 07 | LED_ON | ACK:LED_ON | 4.76 |
| 08 | LED_OFF | ACK:LED_OFF | 7.84 |
| 09 | LED_ON | ACK:LED_ON | 5.35 |
| 10 | LED_OFF | ACK:LED_OFF | 4.92 |

**สรุปสถิติ UDP:** Average = 6.49 ms, Min = 4.76 ms, Max = 12.35 ms

**เปรียบเทียบกับ HTTP RESTful**

| หัวข้อเปรียบเทียบ | UDP (ใบงาน 10.2) | HTTP RESTful (ใบงาน 10.1) |
| :--- | :---: | :---: |
| RTT เฉลี่ย | 6.49 ms | 48.30 ms |
| RTT ต่ำสุด | 4.76 ms | 31.20 ms |
| RTT สูงสุด | 12.35 ms | 112.40 ms |
| ความผันผวน (Jitter) | ต่ำ | สูงกว่า |
| ขนาดข้อมูลต่อคำขอ | 6 ไบต์ (Payload) | 312 ไบต์ (รวม Header) |
| การสร้างการเชื่อมต่อ | ไม่ต้อง (Connectionless) | ต้องทำ TCP 3-way Handshake |

**วิเคราะห์ความแตกต่าง**

UDP มีค่า RTT เฉลี่ยต่ำกว่า HTTP 7.4 เท่า (48.30 / 6.49) โดยมีสาเหตุดังนี้

1. **ไม่มีขั้นตอนสร้างการเชื่อมต่อ:** HTTP ทำงานบน TCP ซึ่งต้องทำ 3-way Handshake (SYN, SYN-ACK, ACK) ก่อนส่งข้อมูลจริง และต้องปิดการเชื่อมต่อด้วย FIN ส่วน UDP ส่งดาตาแกรมได้ทันที จึงลดจำนวนรอบการสื่อสารลงอย่างมาก
2. **Header มีขนาดเล็ก:** UDP Header มีเพียง 8 ไบต์ ขณะที่ TCP Header อย่างน้อย 20 ไบต์ และ HTTP ยังเพิ่ม Header ระดับแอปพลิเคชัน (Host, User-Agent, Content-Type ฯลฯ) อีกหลายร้อยไบต์
3. **ไม่มีกลไก Reliability ในชั้น Transport:** UDP ไม่มีการจัดลำดับ การส่งซ้ำ หรือ Flow/Congestion Control จึงประมวลผลได้เร็วกว่า แต่ต้องจัดการความน่าเชื่อถือเองในระดับแอปพลิเคชัน
4. **ภาระการประมวลผลบน ESP32 ต่ำกว่า:** การ Parse HTTP Request และสร้าง Response ใช้ CPU และหน่วยความจำมากกว่าการเปรียบเทียบสตริงสั้นๆ ใน UDP Handler

**ข้อสังเกต:** RTT รอบที่ 1 (8.42 ms) และรอบที่ 4 (12.35 ms) สูงกว่าปกติ เนื่องจากรอบแรกต้องทำ ARP Resolution และ Wi-Fi อาจอยู่ในโหมดประหยัดพลังงาน (Power Save) ส่วนรอบที่ 4 อาจเกิดจากการแย่งช่องสัญญาณกับอุปกรณ์อื่นในย่านความถี่เดียวกัน

**สรุป:** UDP เหมาะกับงานควบคุมแบบ Real-time ที่ต้องการความหน่วงต่ำ ส่วน HTTP เหมาะกับงานที่ต้องการความน่าเชื่อถือ ความเข้ากันได้กับเว็บ และโครงสร้างข้อมูลที่ซับซ้อน

---

### ข้อ 2 อัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate)

ESP32 ส่งข้อมูลที่ความถี่ 10 Hz (ทุก 100 ms) ดังนั้นในเวลา 60 วินาที ควรได้รับ 600 แพ็กเก็ต

**ผลจาก Terminal ขณะรัน `udp_listener.py` เป็นเวลา 1 นาที**

```
Listening for UDP Broadcast on port 3334...
[192.168.1.181] Seq: 0      | Potentiometer: 2048  | Total Lost: 0
[192.168.1.181] Seq: 1      | Potentiometer: 2051  | Total Lost: 0
...
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 187, got 188)
[192.168.1.181] Seq: 188    | Potentiometer: 2713  | Total Lost: 1
...
[PACKET LOSS DETECTED] Lost 2 packets! (Expected 351, got 353)
[192.168.1.181] Seq: 353    | Potentiometer: 1890  | Total Lost: 3
...
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 502, got 503)
[192.168.1.181] Seq: 503    | Potentiometer: 3342  | Total Lost: 4
...

--- Statistics ---
Received: 596 packets
Lost: 4 packets
Packet Loss Rate: 0.67%
```

**ตารางสรุปผล**

| รายการ | ค่า |
| :--- | :---: |
| ระยะเวลาทดสอบ | 60 วินาที |
| จำนวนแพ็กเก็ตที่ควรได้รับ (10 Hz x 60 s) | 600 แพ็กเก็ต |
| จำนวนแพ็กเก็ตที่ได้รับจริง | 596 แพ็กเก็ต |
| จำนวนแพ็กเก็ตที่สูญหาย | 4 แพ็กเก็ต |
| จำนวนครั้งที่เกิดเหตุการณ์สูญหาย | 3 ครั้ง |
| Packet Loss Rate | 0.67 % |

**การคำนวณ**

Packet Loss Rate = Lost / (Received + Lost) x 100
= 4 / (596 + 4) x 100
= 4 / 600 x 100
= 0.67 %

**วิเคราะห์สาเหตุการสูญหายบนเครือข่าย Wi-Fi**

1. **สัญญาณรบกวนในย่าน 2.4 GHz:** ESP32 รองรับเฉพาะ 2.4 GHz ซึ่งมีอุปกรณ์ใช้งานร่วมกันหนาแน่น เช่น Bluetooth, ไมโครเวฟ และ Wi-Fi ของห้องข้างเคียง ทำให้เฟรมเสียหายและถูกทิ้ง
2. **Broadcast ไม่มี ACK ในชั้น MAC:** เฟรม Unicast ของ Wi-Fi มี ACK และการส่งซ้ำระดับ MAC แต่เฟรม Broadcast/Multicast ไม่มี ACK จึงไม่ถูกส่งซ้ำเมื่อเสียหาย ซึ่งเป็นสาเหตุสำคัญที่ Broadcast สูญหายได้ง่ายกว่า Unicast
3. **Access Point ส่ง Broadcast ด้วยอัตราต่ำ:** AP ส่งเฟรม Broadcast ที่ Basic Rate ต่ำ และบางรุ่นบัฟเฟอร์ไว้ส่งพร้อม DTIM Beacon ทำให้เกิดการหน่วงและสูญหายเมื่อบัฟเฟอร์เต็ม
4. **ความแรงสัญญาณ (RSSI) ไม่เพียงพอ:** หากบอร์ดอยู่ไกล AP หรือมีสิ่งกีดขวาง อัตราเฟรมเสียหายจะเพิ่มขึ้น
5. **Buffer ของ LwIP หรือฝั่งรับเต็ม:** หาก PC ประมวลผลช้า (เช่น `print` ถี่ๆ) Socket Receive Buffer อาจล้นและทิ้งแพ็กเก็ต
6. **Wi-Fi Power Save Mode:** ESP32 ที่เปิดโหมดประหยัดพลังงานอาจพลาดช่วงเวลาส่งเฟรมบางช่วง

**สรุป:** อัตราสูญหาย 0.67 % ถือว่ายอมรับได้สำหรับ Telemetry ที่ส่งต่อเนื่อง เพราะค่าถัดไปจะมาแทนที่ค่าที่หายไปภายใน 100 ms อย่างไรก็ตาม หากใช้กับการส่งคำสั่งสำคัญ ควรเพิ่มกลไก ACK และ Retransmission ในระดับแอปพลิเคชัน

---

### ข้อ 3 ข้อดีและข้อจำกัดของ UDP Broadcast (255.255.255.255) และสถานการณ์ที่ควรใช้ Multicast หรือ Unicast

**ข้อดี**

| ข้อดี | คำอธิบาย |
| :--- | :--- |
| ไม่ต้องรู้ IP ผู้รับ | ผู้ส่งไม่ต้องตั้งค่า IP ปลายทาง เหมาะกับการค้นหาอุปกรณ์ (Device Discovery) |
| รองรับผู้รับหลายเครื่องพร้อมกัน | ส่งครั้งเดียว ทุกเครื่องในเครือข่ายเดียวกันรับได้ |
| ติดตั้งง่าย | โค้ดฝั่งส่งเรียบง่าย ไม่ต้องจัดการรายชื่อผู้รับ |
| เพิ่มผู้รับได้ทันที | เครื่องใหม่เพียงเปิดฟังที่พอร์ตก็รับข้อมูลได้เลย |
| Latency ต่ำ | ไม่มี Handshake และไม่ต้องสร้างการเชื่อมต่อ |

**ข้อจำกัด**

| ข้อจำกัด | คำอธิบาย |
| :--- | :--- |
| จำกัดในเครือข่ายย่อยเดียว | Router ไม่ส่งต่อ Limited Broadcast ข้าม Subnet หรือ VLAN |
| สิ้นเปลืองทรัพยากร | ทุกอุปกรณ์ในเครือข่ายต้องประมวลผลเฟรม แม้ไม่สนใจข้อมูลนั้น |
| สิ้นเปลือง Airtime ของ Wi-Fi | ส่งด้วย Basic Rate ต่ำ ใช้ช่องสัญญาณนานและกระทบอุปกรณ์อื่น |
| ไม่มี ACK ที่ชั้น MAC | สูญหายได้ง่ายกว่า Unicast ดังผลในข้อ 2 |
| ความปลอดภัยต่ำ | ใครในเครือข่ายก็ดักฟังและปลอมแปลงแพ็กเก็ตได้ง่าย |
| ไม่รองรับ IPv6 | IPv6 ไม่มี Broadcast ต้องใช้ Multicast แทน |
| อาจถูกบล็อก | บาง Access Point เปิด Client Isolation หรือมี Firewall บล็อก Broadcast |
| ไม่เหมาะกับการขยายขนาด | อุปกรณ์จำนวนมากส่ง Broadcast พร้อมกันอาจเกิด Broadcast Storm |

**สถานการณ์ที่ควรเปลี่ยนไปใช้ UDP Multicast**

| สถานการณ์ | เหตุผล |
| :--- | :--- |
| มีผู้รับหลายเครื่องแต่เป็นเพียงบางกลุ่ม | ผู้รับ Join กลุ่ม (IGMP) เฉพาะที่สนใจ อุปกรณ์อื่นไม่ต้องประมวลผล |
| เครือข่ายขนาดใหญ่ที่มีอุปกรณ์ IoT จำนวนมาก | ลดภาระ Broadcast และหลีกเลี่ยง Broadcast Storm |
| ต้องข้าม Subnet | Multicast Routing (PIM) ส่งข้ามเครือข่ายได้ |
| ต้องรองรับ IPv6 | IPv6 ใช้ Multicast แทน Broadcast |
| ต้องการประหยัดพลังงานของอุปกรณ์ที่ไม่เกี่ยวข้อง | อุปกรณ์ที่ไม่ได้ Join กลุ่มไม่ต้องตื่นมารับ |

ตัวอย่าง: กลุ่ม 239.255.0.1 สำหรับส่งข้อมูลเซนเซอร์ให้ Dashboard หลายเครื่อง

**สถานการณ์ที่ควรเปลี่ยนไปใช้ UDP Unicast**

| สถานการณ์ | เหตุผล |
| :--- | :--- |
| มีผู้รับเพียงเครื่องเดียวหรือไม่กี่เครื่องที่ทราบ IP | ไม่รบกวนอุปกรณ์อื่นและใช้ Airtime อย่างมีประสิทธิภาพ |
| ต้องการความน่าเชื่อถือสูงขึ้น | เฟรม Unicast มี MAC-level ACK และ Retransmission ช่วยลดการสูญหาย |
| ข้อมูลอ่อนไหวหรือต้องการความปลอดภัย | จำกัดผู้รับได้ และใช้ร่วมกับการเข้ารหัสอย่าง DTLS ได้ง่ายกว่า |
| เครือข่ายบล็อก Broadcast/Multicast | เช่น Wi-Fi สาธารณะ หรือเครือข่ายองค์กรที่เปิด Client Isolation |
| ต้องการสื่อสารสองทาง (Request/Response) | เช่น ระบบควบคุมอุปกรณ์ที่ต้องรับ ACK กลับ |

**ตารางเปรียบเทียบสรุป**

| คุณสมบัติ | Unicast | Broadcast | Multicast |
| :--- | :---: | :---: | :---: |
| จำนวนผู้รับ | 1 | ทุกเครื่องใน Subnet | เฉพาะสมาชิกกลุ่ม |
| ต้องรู้ IP ผู้รับ | ต้อง | ไม่ต้อง | ต้องรู้ Group Address |
| MAC-level ACK (Wi-Fi) | มี | ไม่มี | ไม่มี |
| ข้าม Subnet ได้ | ได้ | ไม่ได้ | ได้ (ต้องตั้ง Routing) |
| ภาระต่ออุปกรณ์อื่น | ไม่มี | สูง | ต่ำ |
| รองรับ IPv6 | ได้ | ไม่ได้ | ได้ |
| เหมาะกับ | ควบคุมเฉพาะจุด | ค้นหาอุปกรณ์ / ทดลองในแล็บ | ส่งข้อมูลให้หลายผู้รับแบบกลุ่ม |

**สรุป:** 255.255.255.255 เหมาะกับการทดลองในห้องแล็บและการค้นหาอุปกรณ์ในเครือข่ายขนาดเล็ก แต่สำหรับระบบ IoT ที่ใช้งานจริงและมีขนาดใหญ่ ควรใช้ Multicast เมื่อมีผู้รับหลายเครื่อง และใช้ Unicast เมื่อต้องการความน่าเชื่อถือ ความปลอดภัย หรือการสื่อสารสองทาง