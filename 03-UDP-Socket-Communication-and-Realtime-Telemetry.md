# 10.3 การสื่อสารระดับซ็อกเก็ต UDP และข้อมูล Telemetry แบบเรียลไทม์

## 10.3.1  คุณลักษณะของโปรโตคอล UDP

**User Datagram Protocol (UDP)** เป็นโปรโตคอลการสื่อสารระดับ Transport Layer (Layer 4) ตามมาตรฐาน RFC 768 ซึ่งได้รับการออกแบบมาให้ทำงานแบบ **Connectionless (ไร้การเชื่อมต่อ)** แตกต่างจาก TCP อย่างสิ้นเชิง

1. **Connectionless (ไม่สร้างการเชื่อมต่อ)**  
   UDP ไม่จำเป็นต้องทำกระบวนการสร้างการเชื่อมต่อ (ไม่มี 3-Way Handshake: SYN, SYN-ACK, ACK) ก่อนที่จะส่งข้อมูล ข้อมูลสามารถถูกส่งตรงไปยังปลายทางได้ทันที และไม่มีภาระในการคงสถานะการเชื่อมต่อ (Connection State) ทั้งฝั่งส่งและฝั่งรับ
2. **Minimal Header Overhead (ภาระส่วนหัวน้อยที่สุด)**  
   ขนาดส่วนหัว (Header) ของ UDP มีขนาดคงที่เพียง **8 ไบต์** เท่านั้น (ประกอบด้วย Source Port, Destination Port, Length, และ Checksum) ขณะที่ TCP มี Header ขั้นต่ำถึง 20 ไบต์
3. **No Retransmission / Best-Effort Delivery (ส่งแบบพยายามสูงสุด)**  
   UDP ไม่มีการตรวจสอบยืนยันการรับส่ง (No ACK), ไม่มีการส่งซ้ำเมื่อแพ็กเก็ตสูญหาย (No Retransmission) และไม่มีกลไกควบคุมการไหลของข้อมูล (Flow & Congestion Control)
4. **รองรับการส่งแบบ Broadcast และ Multicast**  
   UDP รองรับการส่งทั้งแบบจุดต่อจุด (Unicast 1-to-1), การบรอดแคสต์ (Broadcast 1-to-All) และการมัลติแคสต์เฉพาะกลุ่ม (Multicast 1-to-Many) ซึ่ง TCP ไม่สามารถทำได้

<p align="center">
<!-- [รูปภาพ: การเปรียบเทียบโครงสร้าง Header ระหว่าง TCP 20 ไบต์ และ UDP 8 ไบต์] -->
<!-- <img src="Images/tcp_vs_udp_header.svg" width="600"> -->
</p>

### ตารางเปรียบเทียบความแตกต่างระหว่าง TCP และ UDP 

| มิติการเปรียบเทียบ                    | Transmission Control Protocol (TCP)                                                           | User Datagram Protocol (UDP)                                                                           |
| :------------------------------------ | :-------------------------------------------------------------------------------------------- | :----------------------------------------------------------------------------------------------------- |
| **ความน่าเชื่อถือ (Reliability)**     | **น่าเชื่อถือสูง**: รับประกันส่งถึงปลายทาง มีการส่งซ้ำ มี Flow Control และ Congestion Control | **ไม่รับประกัน**: ส่งแบบ Best-effort ข้อมูลอาจสูญหายหรือสลับลำดับได้                                   |
| **รูปแบบการเชื่อมต่อ (Connection)**   | **Connection-Oriented**: ทำ 3 Handshakes เพื่อเปิด และ 4 Handshakes เพื่อปิดการเชื่อมต่อ      | **Connectionless**: ยิงข้อมูลออกไปได้ทันที ไม่ต้องเปิดหรือปิดเซสชัน                                    |
| **เป้าหมายการส่ง (Addressing)**       | **One-to-One**: รองรับเฉพาะ Unicast (จุดต่อจุด)                                               | **หลากหลาย**: รองรับทั้ง Unicast (1:1), Broadcast (1:All), และ Multicast (1:Many)                      |
| **ขนาดส่วนหัว (Header Overhead)**     | **20 ไบต์** (หรือมากกว่าเมื่อมี Options)                                                      | **8 ไบต์** (ขนาดคงที่ตลอดเวลา)                                                                         |
| **อัตราความเร็ว (Transmission Rate)** | ขึ้นอยู่กับสภาพเครือข่าย หากเกิด Packet Loss ความเร็วจะตกลงเพราะต้องรอส่งซ้ำ                  | **เร็วและสม่ำเสมอ**: ส่งข้อมูลออกสู่สายอากาศทันทีโดยไม่ชะลอรอคอย                                       |
| **ความเหมาะสมในการใช้งาน**            | งานที่ห้ามข้อมูลสูญหาย เช่น โอนไฟล์, Web Page, การส่งคำสั่งเชิงธุรกิจ                         | **งาน Real-time**: เช่น การสตรีมเซนเซอร์ความถี่สูง, ระบบนำทาง, เสียง/วิดีโอ (VoIP), และการค้นหาอุปกรณ์ |

---

## 10.3.2 เหตุผลที่ต้องใช้ UDP ในระบบ IoT และ Local Control

เมื่ออ่านคุณสมบัติข้างต้น หลายคนอาจสงสัยว่า *"หาก UDP ไม่รับประกันความถูกต้อง แล้วทำไมระบบ IoT จึงยังนิยมใช้งาน?"* คำตอบอยู่ที่ข้อดีเชิงวิศวกรรมต่อไปนี้

* **Ultra-Low Latency (ความหน่วงเวลาต่ำระดับมิลลิวินาที)**  
  เนื่องจากไม่ต้องรอสร้างการเชื่อมต่อ เวลาในการส่งข้อมูล (Round-Trip Time - RTT) จึงลดลงเหลือเพียงเวลาที่คลื่นเดินทางจริง เหมาะอย่างยิ่งกับการส่งข้อมูลเซนเซอร์ที่มีการเปลี่ยนแปลงอย่างรวดเร็ว (เช่น ข้อมูลไจโรสโคป/IMU, สัญญาณแอนะล็อกความถี่ 50–100 Hz)
* **Power Efficiency (ประหยัดพลังงานแบตเตอรี่)**  
  การเปิดภาคส่งสัญญาณ Wi-Fi Radio เพื่อยิงแพ็กเก็ต UDP สั้นๆ เพียง 1 ดาตาแกรม ใช้เวลาทำงานของ CPU และวิทยุน้อยกว่าวงรอบการเชื่อมต่อของ TCP หลายเท่าตัว จึงช่วยยืดอายุการใช้งานของโหนดเซนเซอร์ที่ใช้พลังงานจากแบตเตอรี่
* **Zero-Configuration Discovery (การค้นหาอุปกรณ์ในพริบตา)**  
  ดังที่เราได้ศึกษาในหัวข้อที่ 1 แอปพลิเคชันสามารถส่งคำสั่งบรอดแคสต์หรือมัลติแคสต์ไปยังเครือข่ายเพื่อสอบถามหาอุปกรณ์ได้พร้อมกันในแพ็กเก็ตเดียว

---

## 10.3.3 การสร้าง UDP Server ด้วย BSD Socket บน ESP-IDF

การสร้าง UDP Server สำหรับรอรับคำสั่งควบคุมทำได้โดยการใช้ฟังก์ชันมาตรฐานของ **BSD Socket API**

```c
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "esp_log.h"
#include "driver/gpio.h"

#define UDP_PORT 3333
static const char *TAG = "UDP_SERVER";

esp_err_t esp_create_udp_server(void)
{
    char rx_buffer[128];
    char addr_str[32];
    esp_err_t err = ESP_FAIL;
    struct sockaddr_in server_addr;

    // 1. สร้าง IPv4 UDP Socket (SOCK_DGRAM)
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return err;
    }
    ESP_LOGI(TAG, "Socket created successfully, sock fd: %d", sock);

    // 2. เปิดใช้งาน SO_REUSEADDR เพื่อให้ Server สามารถ Bind แอดเดรสเดิมได้ทันที
    int opt = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret < 0) {
        ESP_LOGE(TAG, "Failed to set SO_REUSEADDR: errno %d", errno);
        goto exit;
    }

    // 3. กำหนดค่า Server Address: พอร์ต 3333 และรับทุก Interface (INADDR_ANY)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(UDP_PORT);

    // 4. ผูก Socket เข้ากับแอดเดรสและพอร์ตด้วย bind()
    ret = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        goto exit;
    }
    ESP_LOGI(TAG, "UDP server successfully bound to port %d", UDP_PORT);

    // 5. ลูปวนรอรับข้อมูลด้วย recvfrom()
    while (1) {
        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);
        memset(rx_buffer, 0, sizeof(rx_buffer));

        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&source_addr, &addr_len);

        // ตรวจสอบข้อผิดพลาดในการรับข้อมูล
        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        }

        // แปลง IP ของผู้ส่งเป็นสตริง
        inet_ntoa_r(source_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
        rx_buffer[len] = '\0'; // ปิดท้ายสตริง

        ESP_LOGI(TAG, "Received %d bytes from %s:%d | Message: %s",
                 len, addr_str, ntohs(source_addr.sin_port), rx_buffer);

        // ประมวลผลคำสั่งควบคุมอุปกรณ์ (Actuator Control)
        if (strcmp(rx_buffer, "Open the light") == 0 || strcmp(rx_buffer, "LED_ON") == 0) {
            gpio_set_level(GPIO_NUM_2, 1);
            const char *reply = "Open the light OK";
            sendto(sock, reply, strlen(reply), 0, (struct sockaddr *)&source_addr, addr_len);
        } else if (strcmp(rx_buffer, "Close the light") == 0 || strcmp(rx_buffer, "LED_OFF") == 0) {
            gpio_set_level(GPIO_NUM_2, 0);
            const char *reply = "Close the light OK";
            sendto(sock, reply, strlen(reply), 0, (struct sockaddr *)&source_addr, addr_len);
        }
    }

exit:
    close(sock);
    return err;
}
```

---

## 4. การสร้าง UDP Client ด้วย BSD Socket บน ESP-IDF

อุปกรณ์สามารถทำหน้าที่เป็น **UDP Client** เพื่อส่งข้อมูลคำสั่งหรือส่งข้อมูลเซนเซอร์ Telemetry ไปยังเครื่องปลายทางได้โดยตรงผ่านฟังก์ชัน `sendto()`

```c
#define DEST_HOST_IP "192.168.3.80"
#define DEST_PORT    3333

esp_err_t esp_create_udp_client(void)
{
    esp_err_t err = ESP_FAIL;
    char *payload = "Open the light";
    struct sockaddr_in dest_addr;

    // 1. กำหนด IP Address และ Port ของเครื่องเป้าหมาย
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DEST_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(DEST_HOST_IP);

    // 2. สร้าง UDP Socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return err;
    }

    // 3. ส่งข้อมูลดาตาแกรมออกไปทันทีด้วย sendto() โดยไม่ต้องเรียก connect()
    int ret = sendto(sock, payload, strlen(payload), 0,
                     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        goto exit;
    }

    ESP_LOGI(TAG, "Message '%s' sent successfully to %s:%d", payload, DEST_HOST_IP, DEST_PORT);
    err = ESP_OK;

exit:
    close(sock);
    return err;
}
```

---

## 10.3.5 การจัดการปัญหาข้อมูลสูญหายในระดับ Application Layer

เนื่องจาก UDP ไม่มีการรับประกันการส่ง หากเกิดสัญญาณ Wi-Fi รบกวน คำสั่งสำคัญอย่าง `"Open the light"` อาจสูญหายในอากาศได้ ำแนวทางปฏิบัติเชิงวิศวกรรมสำหรับระบบควบคุมด้วย UDP ไว้ดังนี้

### 10.3.5.1 กลไก Application-level Acknowledgment & Timeout
1. เมื่อ Client ส่งคำสั่ง `"Open the light"` ออกไป ฝั่ง Client จะเริ่มจับเวลา (Start Timer)
2. เมื่อ Server ได้รับคำสั่ง จะต้องส่งข้อความตอบกลับยืนยัน เช่น `"Open the light OK"` กลับมาทันที
3. หาก Client ได้รับข้อความยืนยันภายในช่วงเวลาที่กำหนด (เช่น **1 วินาที**) จะถือว่าการส่งคำสั่งเสร็จสมบูรณ์
4. หากหมดเวลา 1 วินาที (Timeout) แล้วยังไม่ได้รับการตอบกลับ ฝั่ง Client จะทำการส่งคำสั่ง `"Open the light"` ซ้ำอีกครั้ง (Retransmission)

<p align="center">
<!-- [รูปภาพ: ลำดับการทำงานของ UDP Application-level ACK และ Timeout Retransmission] -->
<!-- <img src="Images/udp_ack_timeout_sequence.svg" width="600"> -->
</p>

### 10.3.5.2 การใช้หมายเลขลำดับ (Sequence Number) สำหรับงาน Telemetry Streaming
ในกรณีการส่งข้อมูลเซนเซอร์ความถี่สูง (เช่น ส่งค่าความสว่างหรือค่า Potentiometer ทุก ๆ 20 ms) เราไม่จำเป็นต้องรอ ACK ทุกแพ็กเก็ต แต่จะใส่ **Sequence Number** กำกับไว้ที่ส่วนหัวของข้อความ (เช่น `SEQ:1001,POT:2048`) 
* ฝั่งรับสามารถนำตัวเลข Sequence ไปตรวจจับการกระโดดข้ามเพื่อคำนวณหา **เปอร์เซ็นต์การสูญหายของแพ็กเก็ต (Packet Loss Rate)**
* สามารถตรวจจับและทิ้งแพ็กเก็ตที่มาถึงช้ากว่าลำดับปกติ (Out-of-order Packets) ได้ทันที

---

## 10.3.6 สรุป

1. **UDP** เป็นโปรโตคอล Transport Layer แบบ Connectionless ที่มี Header เล็กเพียง 8 ไบต์ ปราศจากความหน่วงเวลาของ Handshake จึงตอบสนองได้เร็วที่สุดในบรรดาโปรโตคอลเครือข่าย
2. แม้ UDP จะไม่การันตีการส่งถึง แต่เราสามารถเพิ่มความน่าเชื่อถือในระดับ **Application Layer** ได้ด้วยการใช้กลไก Request-ACK และการตั้งเวลา Timeout ส่งซ้ำ
3. การตั้งค่า Socket Option **`SO_REUSEADDR`** เป็นหัวใจสำคัญของ UDP Server บนระบบฝังตัว เพื่อให้สามารถผูกพอร์ตซ้ำได้ทันทีเมื่อรีสตาร์ต
4. อย่างไรก็ตาม การที่ผู้พัฒนาต้องมาเขียนระบบ ACK, ระบบ Timeout และระบบจัดโครงสร้างข้อมูลเองบน UDP ถือเป็นภาระงานที่ซ้ำซ้อน ด้วยเหตุนี้คณะทำงาน IETF จึงได้นำข้อดีของ **RESTful (แบบ HTTP)** มารวมเข้ากับความเร็วของ **UDP** จนเกิดเป็นโปรโตคอลมาตรฐานสำหรับ IoT ในหัวข้อถัดไป นั่นคือ **CoAP (Constrained Application Protocol)**
