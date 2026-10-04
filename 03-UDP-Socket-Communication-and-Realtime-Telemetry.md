# 10.3 การสื่อสารระดับซ็อกเก็ต UDP และข้อมูล Telemetry แบบเรียลไทม์ (UDP Socket & Real-time Telemetry)

## 10.3.1  คุณลักษณะของโปรโตคอล UDP (UDP Characteristics)
**User Datagram Protocol (UDP)** เป็นโปรโตคอลในระดับ Transport Layer (Layer 4) ตามมาตรฐาน RFC 768 ซึ่งแตกต่างจาก TCP อย่างสิ้นเชิง

1. **Connectionless**
   ไม่มีการเชื่อมต่อก่อนส่งข้อมูล (ไม่มี 3-Way Handshake SYN, SYN-ACK, ACK) สามารถยิงแพ็กเก็ตข้อมูลออกไปได้ทันที
2. **Minimal Header Overhead**
   ขนาด Header ของ UDP มีขนาดคงที่เพียง **8 ไบต์** เท่านั้น (ประกอบด้วย Source Port, Destination Port, Length, และ Checksum) เทียบกับ TCP ที่มี Header ขั้นต่ำ 20 ไบต์บวก Options
3. **No Retransmission / No Flow Control**
   ไม่มีการรับประกันว่าข้อมูลจะถึงปลายทาง ไม่มีการจัดเรียงลำดับใหม่เมื่อแพ็กเก็ตมาสลับลำดับ (Best-effort Delivery)
4. **รองรับ Broadcast และ Multicast**
   ข้อมูล 1 แพ็กเก็ตสามารถกระจายไปยังอุปกรณ์หลายตัวพร้อมกันในวงแลนได้

<p align="center">
<!-- [รูปภาพ: การเปรียบเทียบโครงสร้าง Header ระหว่าง TCP 20-60 ไบต์ และ UDP 8 ไบต์] -->
<!-- <img src="Images/tcp_vs_udp_header.svg" width="600"> -->
</p>

---

## 10.3.2 เหตุผลที่ต้องใช้ UDP ในระบบ IoT และ Local Control
* **ความเร็วระดับไมโครวินาที (Ultra-Low Latency)** 
  เนื่องจากไม่มี Handshake จึงประหยัดเวลา Round-Trip Time (RTT) เหมาะกับการส่งข้อมูลเซนเซอร์ความถี่สูง (เช่น IMU, สัญญาณแอนะล็อก 50-100 Hz)
* **ประหยัดพลังงาน (Power Efficiency)**
  การเปิด-ปิด Wi-Fi Radio เพื่อส่งข้อมูลขนาดเล็ก 1 ดาตาแกรมใช้พลังงานไฟฟ้าน้อยกว่าการเชื่อมต่อ TCP หลายเท่าตัว
* **การค้นหาอุปกรณ์ในพริบตา (Zero-Config Broadcast Discovery)**
  Client สามารถส่งคำสั่ง "DISCOVER_DEVICE" ไปยัง `255.255.255.255` เพื่อหาอุปกรณ์ทุกตัวในเครือข่ายพร้อมกัน

---

## 10.3.3 การเขียนโปรแกรม BSD Socket API บน ESP-IDF (LwIP)

ESP-IDF ใช้ระบบเครือข่าย **LwIP (Lightweight IP)** ซึ่งรองรับมาตรฐาน **BSD Socket API (POSIX-like)** ที่เป็นสากล

### 10.3.3.1 การสร้าง UDP Server สำหรับรับคำสั่งควบคุม
```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_PORT 3333

void udp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 1. สร้าง Socket แบบ DGRAM (UDP)
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE("UDP", "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    // 2. กำหนด Address และ Port ที่ต้องการ Bind
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);

    // 3. ผูก Socket เข้ากับพอร์ต
    int err = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err < 0) {
        ESP_LOGE("UDP", "Socket unable to bind: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI("UDP", "UDP Server listening on port %d...", UDP_PORT);

    while (1) {
        // 4. รอรับข้อมูลจาก Client
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&client_addr, &client_addr_len);
        if (len < 0) {
            ESP_LOGE("UDP", "recvfrom failed: errno %d", errno);
            break;
        }

        rx_buffer[len] = '\0';
        ESP_LOGI("UDP", "Received %d bytes from %s: %s", len, 
                 inet_ntoa(client_addr.sin_addr), rx_buffer);

        // ประมวลผลคำสั่ง เช่น "LED_ON" หรือ "LED_OFF"
        if (strcmp(rx_buffer, "LED_ON") == 0) {
            gpio_set_level(GPIO_NUM_2, 1);
            const char *reply = "STATUS:LED_IS_ON";
            sendto(sock, reply, strlen(reply), 0, (struct sockaddr *)&client_addr, client_addr_len);
        } else if (strcmp(rx_buffer, "LED_OFF") == 0) {
            gpio_set_level(GPIO_NUM_2, 0);
            const char *reply = "STATUS:LED_IS_OFF";
            sendto(sock, reply, strlen(reply), 0, (struct sockaddr *)&client_addr, client_addr_len);
        }
    }

    close(sock);
    vTaskDelete(NULL);
}
```

---

## 10.3.4 การจัดการปัญหาข้อมูลสูญหาย

เนื่องจาก UDP ไม่มีการรับประกันการส่ง (Unreliable) ในงานจริงเราจึงนิยมใช้เทคนิคต่อไปนี้เสริมในระดับ Application Layer:
1. **Sequence Number**
   ใส่หมายเลขลำดับแพ็กเก็ต (เช่น ไบต์ที่ 0-1) เพื่อให้ฝั่งรับตรวจสอบว่าแพ็กเก็ตหายหรือไม่ และคำนวณอัตรา Packet Loss
2. **Heartbeat & Keep-Alive**
   ส่งแพ็กเก็ตขนาดเล็กสม่ำเสมอทุกๆ ช่วงเวลา (เช่น 1 วินาที)
3. **Application ACK**
   ส่งแพ็กเก็ตยืนยันกลับสั้นๆ สำหรับคำสั่งสำคัญ (ซึ่งแนวคิดนี้ได้กลายมาเป็นหัวใจของโปรโตคอล **CoAP** ในบทเรียนถัดไป)

---

## 10.3.5 สรุปท้ายบทเรียน
* UDP เหมาะอย่างยิ่งสำหรับงานที่ต้องการความหน่วงต่ำที่สุด (Sub-millisecond) และการกระจายข้อมูลแบบ Broadcast
* การเขียนโปรแกรม Socket ด้วย BSD API บน ESP-IDF ให้การควบคุมระดับล่างที่ยืดหยุ่นและมีประสิทธิภาพสูง
* ข้อจำกัดของ UDP คือความไม่น่าเชื่อถือ ซึ่งหากเราต้องการความเร็วของ UDP แต่ต้องการโครงสร้าง RESTful API แบบ HTTP จะมีคำตอบในหัวข้อที่ 10.4 คือ **CoAP**
