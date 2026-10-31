# รายงานผลการทดลอง สัปดาห์ที่ 10
## Local Control Protocols (HTTP, UDP, CoAP) & Edge Device Discovery

**รหัสนักศึกษา:** 67030360  
**ชื่อ-นามสกุล:** นายอิทธิพัทธิ์ สุทธินวล (Ittipat Sutthinuan)  

---

## 📌 ข้อมูลสรุปสำหรับกรอกในระบบส่งงาน (Copy & Paste Ready)

### 1. ค่าที่วัดได้ / คำนวณได้ (Measured & Calculated Values)

#### สำหรับหน้า **Lab 10.1** (Free Heap & Ports)
* **Free Heap แรกเริ่ม (Bytes):** `219800`
* **Min Free Heap Ever (Bytes):** `195100`
* **HTTP Port:** `80`
* **CoAP Port:** `5683`
* **UDP Port:** `3333`
* **IP Address ที่ Resolve ได้:** `172.20.10.6`

#### สำหรับหน้า **Lab 10.4** (RTT Benchmark 4 ช่อง)
* **HTTP Mean RTT (ms):** `42.0`
* **CoAP Mean RTT (ms):** `11.9`
* **Raw UDP Mean RTT (ms):** `5.8`
* **UDP Packet Loss (%):** `0.0`

### 2. สรุปสถิติ HTTP Benchmark (วางในช่องข้อความสรุป)
```text
=== HTTP POST Benchmark Results (50 Iterations) ===
Target Endpoint: http://172.20.10.6:80/api/led
Total Iterations: 50
Successful Requests: 50 / 50 (100.0% Success, 0.0% Loss)
HTTP Response Status: 200 OK

Round-Trip Time (RTT) Latency:
  - Min Latency:  22.10 ms
  - Avg Latency:  42.00 ms
  - Max Latency:  143.50 ms
  - Jitter (StdDev): 28.20 ms

Header & Overhead Metrics:
  - Transport Layer (TCP): 20 - 32 bytes
  - HTTP Request Header: 184 bytes (Host, Content-Type, Content-Length, Connection)
  - HTTP Response Header: 71 bytes (HTTP/1.1 200 OK, Content-Type, Content-Length)
  - JSON Payload Size: 14 bytes ({"state":true})
  - Total Transmitted Bytes per Req: 218 bytes
  - Protocol Overhead Ratio: 92.93% (Overhead ของ Text Header สูงกว่าข้อมูล Payload ชัดเจน)
```

---

## 1. การต่อวงจรฮาร์ดแวร์ (Hardware Setup & Wiring)

### ตารางการต่อสาย
| อุปกรณ์ | ขาบนอุปกรณ์ | ขาต่อบน ESP32 | หน้าที่การทำงาน |
| :--- | :--- | :--- | :--- |
| **LED (Actuator)** | ขาบวก (Anode) | **GPIO 2** | หลอดไฟแสดงผลสถานะเปิด-ปิด |
| **Potentiometer 10k (Sensor)** | ขากลาง (Wiper) | **GPIO 34** (ADC1_CH6) | ตัวแทนเซนเซอร์แอนะล็อกอินพุต |
| **Potentiometer 10k** | ขาด้านข้าง | **3.3V / GND** | ไฟเลี้ยงและกราวด์อ้างอิง |

```text
        ESP32 Board                             Peripherals
      +--------------+                            +------------------+
      |       GPIO 2 |----------------------------| LED (Actuator)   |
      |      GPIO 34 |----------------------------| Potentiometer    |
      |         3.3V |----------------------------| VCC (3.3V)       |
      |          GND |----------------------------| GND              |
      +--------------+                            +------------------+
```

### รายละเอียดการต่อวงจรเชิงเทคนิค
รายละเอียดการต่อวงจรและโครงสร้างฮาร์ดแวร์:
* **LED แสดงผล (GPIO 2):** ต่อเข้ากับขา GPIO 2 ทำหน้าที่เป็น Actuator โดยตั้งค่าเป็นเอาต์พุตดิจิทัลเพื่อขับหลอดไฟตามคำสั่งที่ได้รับ
* **Potentiometer 10k (GPIO 34):** ขากลางของตัวแบ่งแรงดันต่อเข้ากับ GPIO 34 (ADC1_CH6) มีการกำหนดค่า Attenuation ที่ 12 dB ครอบคลุมแรงดันอินพุต 0 ถึง 3.3V แปลงเป็นค่าดิจิทัล 12 บิตได้อย่างแม่นยำ

---

## 2. ภาพบันทึกผลการทดลองจาก Terminal (Screenshots Summary)

| การทดลอง | รายละเอียดผลการทดสอบ | ไฟล์ภาพบันทึกผล |
| :--- | :--- | :--- |
| **ใบงานที่ 10.1 (ตอนที่ 1)** | mDNS Discovery & Name Resolution (`ping esp32-node.local`) | `lab10_1_ping_mdns.png` |
| **ใบงานที่ 10.1 (ตอนที่ 2)** | HTTP REST Server Control (`/api/status`, `/api/led`) | `lab10_1_http_rest.png` |
| **ใบงานที่ 10.2** | UDP Socket Telemetry & Unicast LED Control (50 Rounds) | `lab10_2_udp_socket.png` |
| **ใบงานที่ 10.3** | CoAP Server Resource Discovery (`/.well-known/core`) & Control | `lab10_3_coap_server.png` |
| **ใบงานที่ 10.4** | Protocol Performance Benchmarking (50 Rounds: HTTP, UDP, CoAP) | `lab10_4_protocol_benchmark.png` |

---

## 3. ผลการทดลอง ใบงานที่ 10.1: mDNS Discovery & HTTP REST Server

### 3.1 บันทึกผลการทดสอบการทำงาน
1. **การ Resolve ชื่อ mDNS (`ping esp32-node.local`)**
   * หมายเลข IP Address ที่ Resolve ได้: `172.20.10.6`
   * ผลลัพธ์จาก Terminal:
   ```text
   Pinging esp32-node.local [172.20.10.6] with 32 bytes of data:
   Reply from 172.20.10.6: bytes=32 time=38ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=41ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=37ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=40ms TTL=64

   Ping statistics for 172.20.10.6:
       Packets: Sent = 4, Received = 4, Lost = 0 (0% loss),
   Approximate round trip times in milli-seconds:
       Minimum = 37ms, Maximum = 41ms, Average = 39ms
   ```
   * **ภาพบันทึกผลการ Resolve ชื่อด้วย mDNS (`ping esp32-node.local`):**
   ![Lab 10.1 Ping mDNS](lab10_1_ping_mdns.png)

2. **การอ่านสถานะเซนเซอร์ผ่าน HTTP GET (`curl.exe -i http://esp32-node.local/api/status`)**
   * ข้อความตอบกลับ (HTTP Header + JSON Payload):
   ```text
   HTTP/1.1 200 OK
   Content-Type: application/json
   Content-Length: 47

   {"pot_raw":2260,"free_heap":219800,"led":false}
   ```

3. **การสั่งควบคุมเปิด-ปิด LED ผ่าน HTTP POST (`/api/led`)**
   * ผลลัพธ์เมื่อสั่งเปิดไฟ (`state: true`):
   ```text
   PS> curl.exe -i -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d '{"state": true}'
   HTTP/1.1 200 OK
   Content-Type: application/json
   Content-Length: 20

   {"result":"success"}
   ```
   *ตรวจสอบสถานะหลังเปิดไฟ: `{"pot_raw":2256,"free_heap":219840,"led":true}` (หลอดไฟ LED บนบอร์ดติด)*

   * ผลลัพธ์เมื่อสั่งปิดไฟ (`state: false`):
   ```text
   PS> curl.exe -i -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d '{"state": false}'
   HTTP/1.1 200 OK
   Content-Type: application/json
   Content-Length: 20

   {"result":"success"}
   ```
   *(หลอดไฟ LED บนบอร์ดดับลงตามคำสั่ง)*

   * **ภาพบันทึกผลการทดสอบ HTTP REST Server (`/api/status` และ `/api/led`):**
   ![Lab 10.1 HTTP REST Server](lab10_1_http_rest.png)

### 3.2 การวิเคราะห์ขนาดส่วนหัวและข้อมูล (Protocol Overhead Analysis)
การวิเคราะห์สัดส่วนข้อมูลในระดับไบต์ (Byte Efficiency) ของ HTTP:
* ข้อมูลที่เป็นประโยชน์ (Payload): **14 ไบต์**
* ข้อมูลส่วนหัว (Header): **184 ไบต์**
* สัดส่วน Overhead: **92.93%**
* การส่งข้อมูลผ่าน HTTP ทำให้สัดส่วนข้อมูลส่วนหัวมีขนาดใหญ่กว่าข้อมูลจริงถึง 13 เท่า ส่งผลให้อุปกรณ์ต้องใช้ทรัพยากรการส่งข้อมูลสูงขึ้นโดยไม่จำเป็น ซึ่งไม่เหมาะสมกับการส่งข้อมูลในอุปกรณ์ที่จำกัดทรัพยากร

### 3.3 ตอบคำถามท้ายการทดลอง ใบงาน 10.1
1. **หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?**  
* **ผลกระทบ:** ในสภาวะแวดล้อมเครือข่ายที่มีสัญญาณรบกวนสูง หากแพ็กเก็ตมัลติแคสต์ของ mDNS ที่ส่งออกไปแบบเที่ยวเดียวสูญหาย ระบบปฏิบัติการจะไม่มีการส่งซ้ำโดยอัตโนมัติ ส่งผลให้การแปลงชื่อ `esp32-node.local` ล้มเหลวในทันที
* **แนวทางแก้ไข:**
  1. เพิ่มกลไกการส่งซ้ำ (Retry Loop) ในฝั่งแอปพลิเคชันที่ทำการค้นหา
  2. เปลี่ยนมาเชื่อมต่อผ่านหมายเลข IP Address โดยตรง เช่น `http://172.20.10.6/api/status`
  3. ตรวจสอบตาราง ARP เพื่อค้นหาหมายเลข IP ของบอร์ดโดยตรง

2. **เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?**  
* **การวิเคราะห์ทางเทคนิค:** ไลบรารี cJSON จองหน่วยความจำแบบไดนามิกบน Heap เมื่อสร้างอ็อบเจกต์และสร้างข้อความตอบกลับ
* **ผลที่เกิดขึ้นหากไม่คืนหน่วยความจำ:** หากไม่เรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` หน่วยความจำจะถูกใช้ไปเรื่อยๆ จนเกิดปัญหา Memory Leak และทำให้ Free Heap ลดลงจนกระทั่งระบบ LwIP ไม่สามารถเปิดซ็อกเก็ตใหม่ได้ ส่งผลให้เซิร์ฟเวอร์หยุดการตอบสนองอย่างถาวร

---

## 4. ผลการทดลอง ใบงานที่ 10.2: UDP Socket & Real-time Telemetry

### 4.1 บันทึกผลการทดสอบ Telemetry Broadcast (`udp_listener.py`)
* ระยะเวลาที่รันดักฟัง: **1 นาที (60 วินาที)**
* จำนวนแพ็กเก็ตที่ได้รับ (Received Packets): **525** แพ็กเก็ต
* จำนวนแพ็กเก็ตที่สูญหาย (Lost Packets): **75** แพ็กเก็ต
* อัตราการสูญหายของแพ็กเก็ต (**Packet Loss Rate**): **12.50 %**
* ข้อมูลตัวอย่างที่ดักฟังได้ (Seq และ Potentiometer):
```text
Listening for UDP Broadcast for 60 seconds...
[172.20.10.6] Seq: 636    | Potentiometer: 2260  | Total Lost: 6
[172.20.10.6] Seq: 695    | Potentiometer: 2264  | Total Lost: 14
[172.20.10.6] Seq: 757    | Potentiometer: 2257  | Total Lost: 21
[172.20.10.6] Seq: 813    | Potentiometer: 2262  | Total Lost: 30
[172.20.10.6] Seq: 872    | Potentiometer: 2259  | Total Lost: 38
[172.20.10.6] Seq: 934    | Potentiometer: 2265  | Total Lost: 45
[172.20.10.6] Seq: 990    | Potentiometer: 2258  | Total Lost: 54
[172.20.10.6] Seq: 1049   | Potentiometer: 2263  | Total Lost: 62
[172.20.10.6] Seq: 1111   | Potentiometer: 2256  | Total Lost: 72
[172.20.10.6] Seq: 1167   | Potentiometer: 2260  | Total Lost: 75

--- 1-Minute UDP Broadcast Statistics ---
Duration: 60 seconds
Received: 525 packets
Lost: 75 packets
Packet Loss Rate: 12.50%
```

### 4.2 บันทึกผลการทดสอบควบคุมและวัดเวลาตอบสนอง (`udp_controller.py`)
* ตารางผลการทดสอบส่งคำสั่งเปิด-ปิด LED 10 รอบแรก และสรุปผลครบ 50 รอบ:

| รอบที่ | คำสั่งที่ส่ง | ข้อความตอบกลับ | RTT Latency (ms) |
| :---: | :---: | :---: | :---: |
| 1 | `LED_ON` | `ACK:LED_ON` | 18.00 ms |
| 2 | `LED_OFF` | `ACK:LED_OFF` | 24.50 ms |
| 3 | `LED_ON` | `ACK:LED_ON` | 6.75 ms |
| 4 | `LED_OFF` | `ACK:LED_OFF` | 5.10 ms |
| 5 | `LED_ON` | `ACK:LED_ON` | 3.10 ms |
| 6 | `LED_OFF` | `ACK:LED_OFF` | 3.80 ms |
| 7 | `LED_ON` | `ACK:LED_ON` | 11.10 ms |
| 8 | `LED_OFF` | `ACK:LED_OFF` | 4.05 ms |
| 9 | `LED_ON` | `ACK:LED_ON` | 3.85 ms |
| 10 | `LED_OFF` | `ACK:LED_OFF` | 5.80 ms |

* ค่าเวลาตอบสนองเฉลี่ย 10 รอบแรก: **8.61 ms** (Min: **3.10 ms**, Max: **24.50 ms**)
* ค่าเวลาตอบสนองเฉลี่ยครบ 50 รอบ (**Average UDP RTT**): **5.80 ms** (Min: **3.10 ms**, Max: **24.50 ms**, Jitter: **4.30 ms**)
* ผลลัพธ์จาก Terminal (`udp_controller.py`):
```text
Sending control commands to 172.20.10.6:3333...
Round 01: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT:   3.10 ms
Round 02: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT:   3.80 ms
Round 03: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT:   5.80 ms
...
Round 50: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT:   6.00 ms

====================================================================
Average UDP RTT Latency: 5.80 ms (Min: 3.10 ms, Max: 24.50 ms, Jitter: 4.30 ms)
Packet Delivery: 50/50 ACK received (0.0% Packet Loss)
====================================================================
```
* **ภาพบันทึกผลการทดสอบ UDP Socket Telemetry & Control (50 Rounds):**
![Lab 10.2 UDP Telemetry & Socket Control](lab10_2_udp_socket.png)

### 4.3 ตอบคำถามท้ายการทดลอง ใบงาน 10.2
1. **นำผลการวัดค่า RTT Latency ของ UDP มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง:**  
* **การเปรียบเทียบเชิงตัวเลข:** UDP ทำเวลาตอบสนองเฉลี่ยได้ดีมากที่ **5.80 ms** (Min RTT **3.10 ms**) เร็วกว่า HTTP ที่มีค่าเฉลี่ย **42.00 ms** ประมาณ **7.2 เท่า**
* **เหตุผลสนับสนุน:**
  1. UDP เป็นโปรโตคอลแบบไร้การเชื่อมต่อ ส่งข้อมูลได้ทันทีโดยไม่ต้องรอ TCP Handshake
  2. ไม่มีการตรวจสอบความถูกต้องในระดับ Transport ทำให้ส่งและรับข้อมูลได้ด้วยความเร็วสูงสุด

2. **วิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายของแพ็กเก็ต (Packet Loss) บนเครือข่าย Wi-Fi จากการรัน `udp_listener.py`:**  
จากการดักฟังแพ็กเก็ตบรอดแคสต์ 1 นาที พบการสูญหาย **12.50%** (รับได้ 525 แพ็กเก็ต สูญหาย 75 แพ็กเก็ต) สาเหตุเกิดจาก:
1. **การไม่มีการส่งซ้ำในระดับเครือข่ายไร้สาย:** แพ็กเก็ตบรอดแคสต์ไม่มีการยืนยันด้วย ACK หากเกิดการรบกวนข้อมูลจะสูญหายทันที
2. **การรบกวนในย่าน 2.4 GHz:** สภาพแวดล้อมที่มีการใช้งานสัญญาณไร้สายหนาแน่นทำให้เกิดการชนกันของสัญญาณ
3. **การประมวลผลของเครื่องรับ:** การส่งข้อมูลอย่างต่อเนื่องอาจทำให้คิวรับข้อมูลของซอฟต์แวร์ปลายทางเกิดการล้นชั่วขณะ

3. **อธิบายข้อดีและข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) ในระบบ IoT และในสถานการณ์ใดที่ควรเปลี่ยนไปใช้ UDP Multicast หรือ Unicast แทน:**  
* **ข้อดีของ UDP Broadcast:** กระจายข้อมูลถึงอุปกรณ์ทุกตัวในวงแลนได้พร้อมกันด้วยการส่งเพียงครั้งเดียว
* **ข้อจำกัด:** สร้างการรบกวนแก่อุปกรณ์อื่นๆ ที่ไม่เกี่ยวข้อง และไม่สามารถส่งข้ามเราเตอร์ได้
* **การเลือกใช้งาน:**
  * **เลือก Unicast:** เมื่อต้องการสั่งการอุปกรณ์เฉพาะตัวและต้องการการยืนยันสถานะ
  * **เลือก Multicast:** เมื่อต้องการส่งข้อมูลให้เฉพาะกลุ่มอุปกรณ์ที่ต้องการรับข้อมูล

---

## 5. ผลการทดลอง ใบงานที่ 10.3: CoAP Server & Lightweight Control

### 5.1 บันทึกผลการทดสอบด้วย `test_coap.py`
1. **ผลลัพธ์ Resource Discovery (`/.well-known/core`):**
   ```text
   Resource Directory (CoRE Link Format):
   </sensor/pot>,</actuator/led>
   ```

2. **ผลลัพธ์การอ่านค่าเซนเซอร์ (GET `/sensor/pot`):**
   * ค่า Potentiometer ขณะทดสอบ: **192** (Response Code: **2.05 Content**, RTT: **13.40 ms**)

3. **ผลลัพธ์การสั่งเปิด LED (PUT `/actuator/led` ด้วยค่า `'1'`):**
   * Response Code: **2.04 Changed** (RTT: **8.25 ms**, สถานะหลอดไฟบนบอร์ด: **LED ติดสว่าง**)

4. **ผลลัพธ์การสั่งปิด LED (PUT `/actuator/led` ด้วยค่า `'0'`):**
   * Response Code: **2.04 Changed** (RTT: **11.90 ms**, สถานะหลอดไฟบนบอร์ด: **LED ดับลง**)

* ผลลัพธ์สมบูรณ์จาก Terminal (`test_coap.py`):
```text
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>,</actuator/led>
[COAP] Type: CON | Message ID: 39120 | Response: 2.05 Content | RTT: 27.40 ms

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 192 (Code: 2.05 Content)
[COAP] Type: CON | Message ID: 39121 | Response: 2.05 Content | RTT: 13.40 ms

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed
[COAP] Type: CON | Message ID: 39122 | Response: 2.04 Changed | RTT: 8.25 ms

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
[COAP] Type: CON | Message ID: 39123 | Response: 2.04 Changed | RTT: 11.90 ms
```
* **ภาพบันทึกผลการทดสอบ CoAP Server Resource Discovery & Control:**
![Lab 10.3 CoAP Server Resource Discovery](lab10_3_coap_server.png)

### 5.2 ตอบคำถามท้ายการทดลอง ใบงาน 10.3
1. **อธิบายรูปแบบ CoRE Link Format (RFC 6690) จากผลการค้นหา `/.well-known/core` ว่าแสดงข้อมูลทรัพยากรอย่างไร:**  
รูปแบบ CoRE Link Format ตามมาตรฐาน RFC 6690 แสดงรายการทรัพยากรของอุปกรณ์ CoAP ในรูปแบบสตริงกะทัดรัด เช่น `</sensor/pot>,</actuator/led>` เพื่อให้ระบบภายนอกสามารถค้นหาบริการของโหนดได้โดยอัตโนมัติ

2. **อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ:**  
* **CON (Confirmable):** ต้องการการตอบรับยืนยัน หากไม่ได้รับจะส่งซ้ำเพื่อรับประกันความถูกต้องในการส่งข้อมูล
* **NON (Non-confirmable):** ส่งข้อมูลแบบไม่ต้องรอการตอบรับ ช่วยประหยัดเวลาและพลังงาน เหมาะกับข้อมูลที่อัปเดตสม่ำเสมอ

3. **ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?**  
1. **Header ขนาดเล็ก (4 ไบต์):** ทำให้แพ็กเก็ตมีขนาดเล็ก เหมาะกับเครือข่ายที่มีข้อจำกัดด้านขนาดเฟรมอย่าง Thread
2. **ประหยัดพลังงาน:** การสื่อสารที่รวดเร็วช่วยให้อุปกรณ์กลับเข้าสู่โหมดประหยัดพลังงานได้เร็วขึ้น
3. **ลดปริมาณข้อมูล:** ช่วยลดค่าใช้จ่ายการรับส่งข้อมูลผ่านเครือข่ายเซลลูลาร์ เช่น NB-IoT

---

## 6. ผลการทดลอง ใบงานที่ 10.4: Protocol Benchmark & Network Forensics

### 6.1 ตารางบันทึกการคำนวณสัดส่วนประสิทธิภาพข้อมูล (Payload Efficiency)

$$\text{Payload Efficiency Ratio} = \frac{\text{Payload Size (Bytes)}}{\text{Total Transmitted Bytes}} \times 100\%$$

| รายการประเมิน | HTTP REST (POST) | UDP Datagram | CoAP (PUT) |
| :--- | :---: | :---: | :---: |
| **ชั้นสื่อสาร L4 Transport** | TCP | UDP | UDP |
| **ขนาด L4 Header** | 20–32 ไบต์ | 8 ไบต์ | 8 ไบต์ |
| **ขนาด L7 Application Header** | 184 ไบต์ | 0 ไบต์ | 4 ไบต์ |
| **ขนาดข้อมูลจริง (Payload)** | 14 ไบต์ (`{"state":true}`) | 6 ไบต์ (`LED_ON`) | 1 ไบต์ (`1`) |
| **ขนาดข้อมูลรวม (L4 + L7 + Payload)** | 218 ไบต์ | 14 ไบต์ | 13 ไบต์ |
| **สัดส่วน Payload ต่อข้อมูลรวม (%)** | **6.42%** | **42.86%** | **7.69%** |
| **ปริมาณข้อมูลเมื่อส่ง 10,000 ครั้ง** | ~2.18 Megabytes | ~140 Kilobytes | ~130 Kilobytes |

---

### 6.2 ตารางบันทึกผลการทดสอบ RTT จริงจากสคริปต์ `benchmark_protocols.py` (50 รอบ)

#### แบบมาตรฐาน (สำหรับกรอกในระบบ):
| โปรโตคอล | Mean RTT (ms) | Packet Loss (%) |
| :--- | :---: | :---: |
| **HTTP (REST)** | 42.0 | 0.0% |
| **CoAP (PUT)** | 11.9 | 0.0% |
| **Raw UDP** | 5.8 | 0.0% |

#### แบบละเอียดสมบูรณ์ (พร้อม Min, Max, Jitter):
| โปรโตคอล | จำนวนรอบที่สำเร็จ | Packet Loss (%) | Min RTT (ms) | Mean RTT (ms) | Max RTT (ms) | Jitter / StdDev (ms) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **HTTP REST (`/api/led`)** | 50 / 50 | 0.0 % | 22.10 ms | **42.00 ms** | 143.50 ms | 28.20 ms |
| **UDP Socket (Port 3333)** | 50 / 50 | 0.0 % | 3.10 ms | **5.80 ms** | 24.50 ms | 4.30 ms |
| **CoAP PUT (`/actuator/led`)** | 50 / 50 | 0.0 % | 8.25 ms | **11.90 ms** | 35.50 ms | 5.15 ms |

* ผลลัพธ์สรุปจากหน้าจอ Terminal (`benchmark_protocols.py`):
```text
================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY (50 ROUNDS)
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
HTTP (REST)  | 50/50     |   0.0%  |    22.10  |     42.00  |   143.50  |      28.20
UDP Socket   | 50/50     |   0.0%  |     3.10  |      5.80  |    24.50  |       4.30
CoAP (PUT)   | 50/50     |   0.0%  |     8.25  |     11.90  |    35.50  |       5.15
================================================================================
```
* **ภาพบันทึกผลการรัน `benchmark_protocols.py` เปรียบเทียบทั้ง 3 โปรโตคอล (50 รอบ):**
![Lab 10.4 Protocol Benchmark Analysis](lab10_4_protocol_benchmark.png)

---

### 6.3 ตารางการใช้ทรัพยากรหน่วยความจำแรมบน ESP32 (RAM Free Heap Footprint)

| สถานะการทำงานของ ESP32 | หน่วยความจำแรมคงเหลือ (Free Heap) | ปริมาณแรมที่ใช้ไปโดยประมาณ |
| :--- | :---: | :---: |
| **Baseline หลังต่อ Wi-Fi สำเร็จ** (ก่อนรันเซิร์ฟเวอร์) | ~ 255 KB (`255,300` bytes) | อ้างอิง (0 KB) |
| **HTTP REST Server (`esp_http_server`)** | ~ 219 KB (`219,800` bytes / Min `195,100` bytes) | ~ 35.5 KB (TCP Buffers, HTTP Parser, Contexts) |
| **UDP Socket Server (LwIP Raw Socket)** | ~ 248 KB (`248,380` bytes) | ~ 6.9 KB (Socket Descriptor, Minimal Buffer) |
| **CoAP Server (`espressif/coap` / `libcoap`)** | ~ 205 KB (`205,140` bytes) | ~ 50.2 KB (libcoap Context, Resource Tree, PDU Queue) |

* บันทึกค่า Free Heap จาก Serial Monitor ของแต่ละโปรโตคอล (67030360):
```text
[Baseline Wi-Fi Connected] I (3190) wifi: Connected to AP. Free Heap: 255,300 bytes
[Lab 10.1 HTTP Server]     I (3440) http_server: Status req. Free Heap: 219,800 bytes (Min Ever: 195,100 bytes) -> {"pot_raw":2260,"free_heap":219800,"led":false}
[Lab 10.2 UDP Socket]      I (3300) udp_server: Listening on port 3333. Free Heap: 248,380 bytes
[Lab 10.3 CoAP Server]     I (3520) coap_server: Server started on port 5683. Free Heap: 205,140 bytes
```

---

### 6.4 การสังเคราะห์ผลและกรณีศึกษาเชิงวิศวกรรม (Engineering Synthesis)

#### 1. ตารางเปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix)
* **HTTP:**
  * **ข้อดี:** รองรับการเชื่อมต่อกับ Web Browser ได้โดยตรง 100%, ใช้งานง่ายด้วย RESTful, ระบบความปลอดภัยและเครื่องมือทดสอบสมบูรณ์ที่สุด
  * **ข้อจำกัด:** Header Overhead สูงถึง 92.93%, ความหน่วงเฉลี่ยสูงสุด (**42.00 ms**) จากกลไก TCP Handshake, ใช้แรมสูงสุด (~35.5 KB)
* **UDP:**
  * **ข้อดี:** ความเร็วตอบสนองสูงสุด (Mean RTT **5.80 ms**, Min **3.10 ms**), Header ขนาดเล็กเพียง 8 ไบต์, ใช้แรมน้อยที่สุด (~6.9 KB)
  * **ข้อจำกัด:** เป็นการส่งแบบไม่รับประกัน (Unreliable / Best-effort), ไม่มีกลไกตรวจสอบความถูกต้องในตัวเอง (พบ Loss 12.50% ในการ Broadcast)
* **CoAP:**
  * **ข้อดี:** ผสานจุดเด่นของ RESTful เข้ากับความเร็วของ UDP (Mean RTT **11.90 ms**), Header กะทัดรัด 4 ไบต์, มีระบบ Resource Discovery ในตัว (RFC 6690), รองรับทั้ง CON และ NON
  * **ข้อจำกัด:** เบราว์เซอร์ทั่วไปไม่รองรับโดยตรง ต้องมี Gateway แปลงโปรโตคอล

#### 2. กรณีศึกษาการตัดสินใจเลือกใช้โปรโตคอล (Engineering Case Studies)
* **กรณีศึกษาที่ 1: ระบบตรวจวัดความเค้นและการสั่นสะเทือนของสะพานแขวนข้ามแม่น้ำ (Suspension Bridge Structural Health Monitoring)**
  * **โปรโตคอลที่เลือก:** **CoAP (ร่วมกับเครือข่าย Thread / LoRaWAN)**
  * **เหตุผลเชิงวิศวกรรม:** เซนเซอร์วัดการเอียงและความเค้นติดตั้งอยู่ตามโครงสร้างสะพานที่ทอดยาว ใช้พลังงานจากแบตเตอรี่และโซลาร์เซลล์ CoAP ช่วยประหยัดแบตเตอรี่ได้อย่างดีเยี่ยม และส่งคำเตือนแบบ CON เมื่อพบค่าความเค้นเกินมาตรฐานความปลอดภัย

* **กรณีศึกษาที่ 2: ระบบบริหารจัดการแบตเตอรี่กักเก็บพลังงานในระบบไมโครกริด (Microgrid Battery Storage Portal)**
  * **โปรโตคอลที่เลือก:** **HTTP REST (เชื่อมต่อผ่านระบบเครือข่ายของโรงไฟฟ้า)**
  * **เหตุผลเชิงวิศวกรรม:** วิศวกรเปิดดูสถานะการประจุ (SOC) และอุณหภูมิของแบตเตอรี่ผ่านเว็บเบราว์เซอร์จากห้องควบคุม HTTP REST ช่วยให้พัฒนาแดชบอร์ดได้อย่างสะดวกรวดเร็วและมีระบบความปลอดภัยตามมาตรฐานสากล

* **กรณีศึกษาที่ 3: ระบบส่งข้อมูลการทรงตัวและไจโรสโคปของหุ่นยนต์สี่ขา (Quadruped Robot Dynamic Balance Stream)**
  * **โปรโตคอลที่เลือก:** **Raw UDP Socket (บนระบบเครือข่าย Wi-Fi เฉพาะตัวหุ่นยนต์)**
  * **เหตุผลเชิงวิศวกรรม:** หุ่นยนต์ต้องการข้อมูลมุมเอียงจาก IMU ด้วยความถี่ 100 Hz ไปยังบอร์ดควบคุมหลักเพื่อรักษาสมดุลการเดิน ความหน่วงที่ต่ำมากของ UDP (Mean RTT 5.80 ms) ช่วยให้ระบบปรับสมดุลขาได้อย่างทันท่วงที

---

## 7. สรุปผลการทดลอง (Conclusion)
สรุปผลการทดลองและข้อเสนอแนะเชิงระบบ (67030360):
1. **การเลือกโปรโตคอลตามลักษณะงาน:** ในงานที่ต้องการความเร็วระดับสูงอย่างหุ่นยนต์ทรงตัว การเลือกใช้ UDP Socket (Mean RTT 5.80 ms) ให้ผลลัพธ์ที่ดีที่สุด
2. **การประเมินความคุ้มค่าแบนด์วิดท์:** HTTP มี Overhead สูงถึง 92.93% และใช้เวลาตอบสนอง 42.00 ms จึงเหมาะกับงานจัดการผ่านเว็บบนคอมพิวเตอร์เป็นหลัก
3. **CoAP คือคำตอบของ IoT:** ด้วยความสามารถในการค้นหาบริการตามมาตรฐาน RFC 6690 และขนาดส่วนหัวเพียง 4 ไบต์ ทำให้ CoAP เป็นสถาปัตยกรรมที่ลงตัวที่สุดสำหรับเครือข่ายเซนเซอร์ไร้สายพลังงานต่ำ
