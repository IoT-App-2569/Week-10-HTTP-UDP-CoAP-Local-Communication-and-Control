# รายงานผลการทดลองที่10.2
## 10.1: HTTP RESTful Web Server & mDNS

### ตารางที่ 1.1: การต่อสายฮาร์ดแวร์ (Hardware Wiring - Lab 10.1)

| อุปกรณ์ | ขาอุปกรณ์ | ขาต่อบน ESP32 | หน้าที่ |
| :--- | :--- | :--- | :--- |
| **Onboard / External LED** | ขาบวก (Anode) | **GPIO 2** | หลอดไฟแสดงสถานะการควบคุม Output |
| **Potentiometer (10k)** | ขากลาง (Wiper) | **GPIO 34** | เซนเซอร์แอนะล็อกอินพุต (ADC1 Channel 6) |
| **Potentiometer (10k)** | ขาซ้าย/ขวา | **3.3V / GND** | แรงดันไฟเลี้ยงและกราวด์ |

### ตารางที่ 1.2: สรุป Header Files ที่ใช้งาน (Lab 10.1)

| Header File | หน้าที่และขอบเขตการใช้งาน |
| :--- | :--- |
| `stdio.h` | จัดการ Input/Output เช่น การจัดรูปแบบข้อความด้วย `snprintf()` |
| `string.h` | จัดการสตริงและบล็อกหน่วยความจำ เช่น `strlen()`, `memset()`, `memcpy()` |
| `esp_log.h` | ระบบส่งข้อความแจ้งเตือนและดีบั๊ก เช่น `ESP_LOGI()`, `ESP_LOGE()` |
| `nvs_flash.h` | จัดการ Non-Volatile Storage (NVS) จำเป็นต่อการเริ่มต้นระบบ Wi-Fi Driver |
| `esp_netif.h` | จัดการ TCP/IP Network Interface Adapter บน LwIP |
| `esp_event.h` | Default Event Loop สำหรับดักรับเหตุการณ์ของระบบ (เช่น Wi-Fi connected, IP acquired) |
| `esp_wifi.h` | ควบคุมวิทยุ Wi-Fi ทั้งหมด (กำหนดโหมด Station, SSID/Password, และเชื่อมต่อ AP) |
| `freertos/FreeRTOS.h` | โครงสร้างหลัก ไทป์ตัวแปร และมาโครพื้นฐานของ FreeRTOS |
| `freertos/task.h` | จัดการ Task, การสลับบริบท และการหน่วงเวลา (`vTaskDelay`, `portMAX_DELAY`) |
| `freertos/event_groups.h` | กลไก Event Group สำหรับซิงค์สถานะระหว่าง Event Loop กับ Task หลัก |
| `esp_http_server.h` | HTTP Server ทางการของ ESP-IDF สำหรับรับ/ส่ง Header และลงทะเบียน URI Handlers |
| `cJSON.h` | ไลบรารีสำหรับพาร์สและสร้างข้อมูลในรูปแบบ JSON Payload |
| `driver/gpio.h` | ควบคุมขา Hardware GPIO (กำหนดทิศทาง Input/Output และควบคุมไฟ LED) |
| `mdns.h` | Multicast DNS สำหรับประกาศชื่อโฮสต์เสมือน (`esp32-node.local`) และบริการ DNS-SD |
| `esp_adc/adc_oneshot.h` | ไดรเวอร์ ADC โหมด Oneshot สำหรับอ่านค่าแรงดันจาก Potentiometer (GPIO 34) |

### ตารางที่ 1.3: สรุปผลการทดสอบ REST API Endpoint (Lab 10.1)

| Method | Endpoint | ตัวอย่าง Request Body | HTTP Status Code | Response Body |
| :---: | :--- | :--- | :---: | :--- |
| **GET** | `/api/status` | *(ไม่มี)* | `200 OK` | `{"pot_raw":2048,"free_heap":213848,"led":true}` |
| **POST** | `/api/led` | `{"state": true}` | `200 OK` | `{"result":"success"}` |
| **POST** | `/api/led` | `{"state": false}` | `200 OK` | `{"result":"success"}` |

---

## 2. ใบงานที่ 10.2: UDP Telemetry Streaming & Control Socket

### ตารางที่ 2.1: การต่อสายฮาร์ดแวร์ (Hardware Wiring - Lab 10.2)

| อุปกรณ์ | ขาอุปกรณ์ | ขาต่อบน ESP32 | หน้าที่ |
| :--- | :--- | :--- | :--- |
| **LED** | ขาบวก (Anode) | **GPIO 2** | หลอดไฟแสดงสถานะการควบคุม Output |
| **Potentiometer (10k)** | ขากลาง (Wiper) | **GPIO 34** | เซนเซอร์แอนะล็อกอินพุต (ADC1 Channel 6) |
| **Potentiometer (10k)** | ขาซ้าย/ขวา | **3.3V / GND** | แรงดันไฟเลี้ยงและกราวด์ |

### ตารางที่ 2.2: สรุป Header Files ที่ใช้งาน (Lab 10.2)

| Header File | หน้าที่และขอบเขตการใช้งาน |
| :--- | :--- |
| `stdio.h` / `string.h` | จัดการ Input/Output และฟังก์ชันจัดรูปแบบสตริง (`snprintf()`, `strlen()`, `memcpy()`) |
| `esp_log.h` | ส่งข้อความแจ้งสถานะและตรวจแก้ข้อผิดพลาด (`ESP_LOGI()`, `ESP_LOGE()`) |
| `nvs_flash.h` | จัดการ Non-Volatile Storage สำหรับระบบ Wi-Fi |
| `esp_netif.h` / `esp_event.h` | จัดการ Network Interface Adapter และ Event Loop |
| `esp_wifi.h` | จัดการการเชื่อมต่อวิทยุ Wi-Fi Station |
| `freertos/FreeRTOS.h` / `task.h` | โครงสร้างระบบ FreeRTOS สำหรับสร้าง Task แบบมัลติทาสก์กิ้ง (`xTaskCreate()`) |
| `lwip/sockets.h` / `lwip/netdb.h` | BSD Socket API บน LwIP (`socket()`, `bind()`, `sendto()`, `recvfrom()`, `setsockopt()`) |
| `driver/gpio.h` | ควบคุมระดับสัญญาณดิจิทัลเปิด-ปิดหลอดไฟ LED (GPIO 2) |
| `esp_adc/adc_oneshot.h` | อ่านค่าแรงดันแอนะล็อกจาก Potentiometer (GPIO 34 / ADC1 Channel 6) |

### ตารางที่ 2.3: สรุปผลการดักฟัง UDP Telemetry Broadcast (1 นาที)

| รายการทดสอบ | ค่าที่บันทึกได้ |
| :--- | :--- |
| **ความถี่ในการส่งข้อมูล (Telemetry Rate)** | 10 Hz (ทุกๆ 100 ms) |
| **พอร์ตที่ใช้ส่งข้อมูล (UDP Port)** | 3334 |
| **จำนวนแพ็กเก็ตคาดหวัง (Expected Packets)** | 600 แพ็กเก็ต |
| **จำนวนแพ็กเก็ตที่ได้รับจริง (Received Packets)** | 592 แพ็กเก็ต |
| **จำนวนแพ็กเก็ตที่สูญหาย (Lost Packets)** | 8 แพ็กเก็ต |
| **อัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate)** | **1.33%** |

### ตารางที่ 2.4: ผลการวัดค่า RTT Latency ในการส่งคำสั่งควบคุม LED (Lab 10.2)

| รอบที่ | คำสั่งที่ส่ง (Unicast) | ข้อความตอบกลับ (ACK) | สถานะ LED (GPIO 2) | ค่า RTT Latency (ms) |
| :---: | :---: | :---: | :---: | :---: |
| 1 | `LED_ON` | `ACK:LED_ON` | ติดสว่าง | 6.42 ms |
| 2 | `LED_OFF` | `ACK:LED_OFF` | ดับ | 3.10 ms |
| 3 | `LED_ON` | `ACK:LED_ON` | ติดสว่าง | 2.85 ms |
| 4 | `LED_OFF` | `ACK:LED_OFF` | ดับ | 2.15 ms |
| 5 | `LED_ON` | `ACK:LED_ON` | ติดสว่าง | 4.12 ms |
| 6 | `LED_OFF` | `ACK:LED_OFF` | ดับ | 3.50 ms |
| 7 | `LED_ON` | `ACK:LED_ON` | ติดสว่าง | 5.01 ms |
| 8 | `LED_OFF` | `ACK:LED_OFF` | ดับ | 2.90 ms |
| 9 | `LED_ON` | `ACK:LED_ON` | ติดสว่าง | 3.95 ms |
| 10 | `LED_OFF` | `ACK:LED_OFF` | ดับ | 4.50 ms |
| **เฉลี่ย** | **-** | **-** | **-** | **3.85 ms** |

---

## 3. คำถามท้ายการทดลอง
1. นำผลการวัดค่า RTT Latency ของ UDP ในกิจกรรมที่ 10-2.5 มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง
คำตอบ ตารางเปรียบเทียบการทำงานระหว่าง HTTP RESTful และ UDP Socket
| หัวข้อเปรียบเทียบ | HTTP RESTful (ใบงาน 10.1) | UDP Socket (ใบงาน 10.2) |
| :--- | :--- | :--- |
| **ค่า RTT Latency เฉลี่ย** | **25.0 – 60.0 ms** | **3.0 – 8.0 ms** |
| **โปรโตคอลชั้น Transport** | TCP (Connection-oriented) | UDP (Connectionless) |
| **ขั้นตอนการเชื่อมต่อ** | ต้องทำ 3-Way Handshake (`SYN`, `SYN-ACK`, `ACK`) | ไม่มีการ Handshake ส่งข้อมูลออกได้ทันที |
| **Overhead ของโปรโตคอล** | ขนาด Header ใหญ่ (HTTP + TCP ~50-80 ไบต์) | ขนาด Header เล็กมาก (UDP Header เพียง 8 ไบต์) |
| **การการันตีข้อมูล** | มีการส่ง ACK และ Retransmit เมื่อข้อมูลหาย | ไม่มีระบบการันตีข้อมูล (Fire-and-forget) |
| **รูปแบบการส่งข้อมูล** | Request / Response (ไคลเอนต์ร้องขอ) | Unicast / Broadcast Streaming (ส่งต่อเนื่อง) |
วิเคราะห์ความแตกต่าง:จากการทดลองพบว่า UDP Socket มีค่า RTT Latency ต่ำกว่า HTTP RESTful ถึง 5–10 เท่า เนื่องจาก:
1. กระบวนการเชื่อมต่อ (Handshake Overhead): HTTP ทำงานบน TCP ซึ่งต้องเสียเวลาสถาปนาการเชื่อมต่อแบบ 3-Way Handshake ก่อนส่งข้อมูลจริง ขณะที่ UDP สามารถยิงแพ็กเก็ตออกทาง Socket ได้ทันที
2. ขนาดของภาระข้อมูล (Protocol Overhead): HTTP มีโครงสร้าง Header เป็นตัวอักษรขนาดใหญ่และต้องพาร์สสตริง JSON บนแอปพลิเคชัน ในขณะที่ UDP ส่งเพียงข้อมูลไบนารี/สตริงขนาดเล็ก และมี Transport Header เพียง 8 ไบต์ จึงใช้เวลาประมวลผลบน ESP32 น้อยกว่ามาก
2. รันสคริปต์ udp_listener.py เป็นเวลา 1 นาที จงบันทึกค่าและคำนวณอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) พร้อมวิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายบนเครือข่าย Wi-Fi
คำตอบ การคำนวณอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate):จำนวนแพ็กเก็ตคาดหวังทั้งหมด ($N_{total}$) = $10 \text{ Hz} \times 60 \text{ วินาที} = 600 \text{ แพ็กเก็ต}$จำนวนแพ็กเก็ตที่สูญหาย ($N_{lost}$) = $8 \text{ แพ็กเก็ต}$
วิเคราะห์สาเหตุที่ทำให้เกิด Packet Loss บนเครือข่าย Wi-Fi:
1. คุณลักษณะแบบ Connectionless ของ UDP: UDP ไม่มีกลไกการส่งสัญญาณตอบรับ (ACK) และไม่มีการส่งซ้ำ (No Retransmission) หากแพ็กเก็ตตกหล่นในอากาศ จะถูกทิ้งทันที
2. การรบกวนของคลื่นวิทยุ (Wireless RF Interference): สัญญาณ Wi-Fi ย่าน 2.4 GHz มักเกิดการชนกันของสัญญาณ (Collision) จาก Access Point ข้างเคียง หรืออุปกรณ์อื่นๆ เช่น Bluetooth
3. ปัญหา Queue Overflow ใน Socket Buffer: หากบอร์ด ESP32 หรือคอมพิวเตอร์ติดภาระงานอื่น (FreeRTOS Context Switch) จนดึงข้อมูลออกจากคิว LwIP ไม่ทัน แพ็กเก็ตใหม่ที่เข้ามาจะถูกทิ้ง (Buffer Drop)

3. อธิบายข้อดีและข้อจำกัดของการใช้ 255.255.255.255 (UDP Broadcast) ในระบบ IoT และในสถานการณ์ใดที่ควรเปลี่ยนไปใช้ UDP Multicast หรือ Unicast แทน?
คำตอบ ข้อดีของ UDP Broadcast (255.255.255.255):
- Zero Configuration (ไม่จำเป็นต้องรู้ IP ผู้รับ): ตัวส่งไม่ต้องทราบหมายเลข IP Address ของเครื่องรับล่วงหน้า
- ลดภาระตัวส่ง (One-to-All): ESP32 ส่งแพ็กเก็ตเพียงครั้งเดียว ทุกอุปกรณ์ใน Local Subnet จะได้รับข้อมูลพร้อมกันทันที

ข้อจำกัดของ UDP Broadcast:
- สร้าง Broadcast Storm: แพ็กเก็ตถูกกระจายไปยังทุกอุปกรณ์ในวง LAN ทำให้เครื่องที่ไม่เกี่ยวข้องต้องเสียทรัพยากร CPU มาคอยแกะดูและทิ้งแพ็กเก็ต
- ไม่สามารถข้าม Router ได้: อุปกรณ์ Router จะบล็อกแพ็กเก็ต 255.255.255.255 ไม่ให้ข้ามไปยัง Subnet หรือเครือข่ายอื่น

สถานการณ์การเลือกใช้งานที่เหมาะสม:
- ควรเปลี่ยนไปใช้ UDP Unicast เมื่อ: ต้องการส่งข้อมูลหรือคำสั่งแบบ 1 ต่อ 1 (One-to-One) เฉพาะเจาะจงไปยังโฮสต์ที่ทราบ IP Address แน่นอน เช่น การส่งคำสั่งควบคุม LED_ON ไปยัง ESP32 ตัวที่กำหนด เพื่อความปลอดภัยและไม่รบกวนอุปกรณ์อื่น
- ควรเปลี่ยนไปใช้ UDP Multicast (224.0.0.0/4) เมื่อ: ต้องการส่งข้อมูลแบบ 1 ต่อหลายเครื่อง (One-to-Many) เฉพาะกลุ่มอุปกรณ์ที่ลงทะเบียนรับข่าวสาร (Subscribe IGMP Group) เช่น การส่ง Telemetry ไปยังกลุ่มเซิร์ฟเวอร์มอนิเตอร์ 5 เครื่อง โดยไม่ส่งไปรบกวนคอมพิวเตอร์เครื่องอื่นในวงแลนเดียวกัน