# ใบงานการทดลองที่ 10.2 (Lab 10.2)
### การสื่อสารความหน่วงต่ำด้วย UDP Socket และการถ่ายทอดข้อมูล Real-time Telemetry

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้ นักศึกษาจะได้เรียนรู้การเขียนโปรแกรมเครือข่ายระดับล่างด้วย **BSD Socket API** บนสแตก LwIP ของ ESP32 เพื่อสร้างบริการส่งข้อมูลเซนเซอร์แบบต่อเนื่องความถี่สูง (Real-time Telemetry Streaming) และการควบคุมอุปกรณ์ด้วยแพ็กเก็ต UDP ที่มีขนาด Header เล็กและตอบสนองได้รวดเร็วระดับมิลลิวินาที

---

## 1. วัตถุประสงค์การทดลอง (Objectives)
1. สามารถเขียนโปรแกรม Socket แบบ Connectionless (UDP Datagram) ด้วยคำสั่ง `socket()`, `bind()`, `recvfrom()`, และ `sendto()` บน ESP-IDF ได้
2. สามารถพัฒนา FreeRTOS Task เพื่อส่งข้อมูลแอนะล็อกเซนเซอร์แบบบรอดแคสต์ (UDP Broadcast) สู่เครือข่ายได้ด้วยความถี่ 10-50 Hz
3. สามารถพัฒนาสคริปต์ภาษา Python บนเครื่องคอมพิวเตอร์เพื่อดักฟังข้อมูลบรอดแคสต์ และส่งคำสั่งควบคุม LED กลับมายัง ESP32 ได้
4. สามารถวัดค่าความหน่วงเวลาเฉลี่ย (Round-Trip Latency) และอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) ได้

---

## 2. โครงสร้างระบบและการทำงาน (System Architecture)

```
   [ESP32 Node]                                              [PC Client / Python]
         |                                                             |
         | --- (UDP Broadcast: pot_raw, seq_no) : Port 3334 ---------> | (รับค่าแสดงผลกราฟ)
         |                                                             |
         | <--- (UDP Unicast Command: "LED_ON" / "LED_OFF") : Port 3333| (ส่งคำสั่งควบคุม)
         | --- (UDP Unicast ACK: "STATUS:OK") ------------------------>| (วัด RTT Latency)
```

---

## 3. ขั้นตอนการทดลอง (Deconstructed Activities)

### กิจกรรมที่ 2.1: สร้างโปรเจกต์ UDP Socket
```powershell
idf.py create-project Lab10-2_UDP_Telemetry_Socket
cd Lab10-2_UDP_Telemetry_Socket
idf.py set-target esp32
```

ตรวจสอบไฟล์ `main/CMakeLists.txt`:
```cmake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi esp_event nvs_flash lwip esp_adc)
```

---

### กิจกรรมที่ 2.2: พัฒนา Task รับคำสั่ง UDP Control Server (Port 3333)
ในไฟล์ `main/main.c`:

```c
#include "lwip/sockets.h"

#define UDP_CONTROL_PORT 3333

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
    ESP_LOGI("UDP_SERVER", "Listening for commands on port %d...", UDP_CONTROL_PORT);

    while (1) {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&client_addr, &client_addr_len);
        if (len > 0) {
            rx_buffer[len] = '\0';
            if (strcmp(rx_buffer, "LED_ON") == 0) {
                gpio_set_level(GPIO_NUM_2, 1);
                sendto(sock, "ACK:LED_ON", 10, 0, (struct sockaddr *)&client_addr, client_addr_len);
            } else if (strcmp(rx_buffer, "LED_OFF") == 0) {
                gpio_set_level(GPIO_NUM_2, 0);
                sendto(sock, "ACK:LED_OFF", 11, 0, (struct sockaddr *)&client_addr, client_addr_len);
            }
        }
    }
}
```

---

### กิจกรรมที่ 2.3: พัฒนา Task ส่งข้อมูล Telemetry แบบ Broadcast (Port 3334)
ส่งข้อมูลค่าเซนเซอร์พร้อมหมายเลขลำดับ (Sequence Number) ทุกๆ 100ms:

```c
#define UDP_BROADCAST_PORT 3334

void udp_telemetry_broadcast_task(void *pvParameters)
{
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_BROADCAST_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    // เปิดใช้งานตัวเลือก SO_BROADCAST
    int broadcast_enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    uint32_t seq_no = 0;
    char tx_buffer[64];

    while (1) {
        int pot_val = 2048; // แทนที่ด้วยค่า ADC จาก GPIO 34
        snprintf(tx_buffer, sizeof(tx_buffer), "SEQ:%lu,POT:%d\n", seq_no++, pot_val);

        sendto(sock, tx_buffer, strlen(tx_buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        vTaskDelay(pdMS_TO_TICKS(100)); // ส่งข้อมูลทุกๆ 100ms (10 Hz)
    }
}
```

---

### กิจกรรมที่ 2.4: ทดสอบด้วยสคริปต์ Python บนคอมพิวเตอร์
สร้างไฟล์ `udp_client.py` บนเครื่อง PC:

```python
import socket
import time

ESP32_IP = "192.168.1.xxx"  # แก้ไขเป็น IP ของบอร์ด หรือส่งผ่าน Broadcast
CMD_PORT = 3333

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# ทดสอบวัด Latency (RTT) ของคำสั่งควบคุม
for i in range(10):
    t_start = time.perf_counter()
    client.sendto(b"LED_ON", (ESP32_IP, CMD_PORT))
    data, _ = client.recvfrom(128)
    t_end = time.perf_counter()
    rtt_ms = (t_end - t_start) * 1000
    print(f"Round {i+1}: Reply '{data.decode().strip()}' - Latency: {rtt_ms:.2f} ms")
    time.sleep(0.5)
```

---

## 4. บันทึกผลการทดลองและคำถามท้ายบท (Lab Report & Questions)
1. นำผลการวัดค่า RTT Latency ของ UDP มาเปรียบเทียบกับ HTTP GET/POST ในใบงาน 10.1 สรุปข้อแตกต่างของความหน่วงเวลา
2. หากคอมพิวเตอร์รันสคริปต์ดักฟังสตรีม Telemetry เป็นเวลา 1 นาที จงตรวจสอบว่าหมายเลข `SEQ` มีการกระโดดข้าม (Packet Loss) หรือไม่ เป็นกี่เปอร์เซ็นต์
3. อธิบายข้อดีและข้อเสียของการใช้ `255.255.255.255` (Broadcast) ในเครือข่ายแลนที่มีอุปกรณ์จำนวนมาก
