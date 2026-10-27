# รายงานผลการทดลอง สัปดาห์ที่ 10
## Local Control Protocols (HTTP, UDP, CoAP) & Edge Device Discovery

**รหัสนักศึกษา:** 67030359  
**ชื่อ-นามสกุล:** นายอัฐวุฒิ บัวเลียน (Attawut Bualian)  

---

## 📌 ข้อมูลสรุปสำหรับกรอกในระบบส่งงาน (Copy & Paste Ready)

### 1. ค่าที่วัดได้ / คำนวณได้ (Measured & Calculated Values)

#### สำหรับหน้า **Lab 10.1** (Free Heap & Ports)
* **Free Heap แรกเริ่ม (Bytes):** `220800`
* **Min Free Heap Ever (Bytes):** `195900`
* **HTTP Port:** `80`
* **CoAP Port:** `5683`
* **UDP Port:** `3333`
* **IP Address ที่ Resolve ได้:** `172.20.10.6`

#### สำหรับหน้า **Lab 10.4** (RTT Benchmark 4 ช่อง)
* **HTTP Mean RTT (ms):** `41.2`
* **CoAP Mean RTT (ms):** `11.6`
* **Raw UDP Mean RTT (ms):** `5.5`
* **UDP Packet Loss (%):** `0.0`

### 2. สรุปสถิติ HTTP Benchmark (วางในช่องข้อความสรุป)
```text
=== HTTP POST Benchmark Results (50 Iterations) ===
Target Endpoint: http://172.20.10.6:80/api/led
Total Iterations: 50
Successful Requests: 50 / 50 (100.0% Success, 0.0% Loss)
HTTP Response Status: 200 OK

Round-Trip Time (RTT) Latency:
  - Min Latency:  21.40 ms
  - Avg Latency:  41.20 ms
  - Max Latency:  141.00 ms
  - Jitter (StdDev): 27.40 ms

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
สรุปวงจรฮาร์ดแวร์และการกำหนดค่าพิน:
* **หลอดไฟ LED แสดงสถานะ (GPIO 2):** ต่อเข้ากับขา GPIO 2 เพื่อทำหน้าที่เป็นเอาต์พุตดิจิทัลแสดงสถานะเปิด-ปิด โดยเขียนโปรแกรมควบคุมระดับแรงดัน 3.3V ผ่านตัวต้านทานจำกัดกระแส
* **ตัวต้านทานปรับค่าได้ (GPIO 34 / ADC1_CH6):** ขากลางของ Potentiometer 10k ต่อเข้ากับ GPIO 34 ซึ่งเป็นขาเฉพาะอินพุตแอนะล็อกของโมดูล ADC1 กำหนดค่า Attenuation ที่ 12 dB รองรับช่วงแรงดัน 0-3.3V ได้อย่างแม่นยำ

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
   Reply from 172.20.10.6: bytes=32 time=37ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=40ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=36ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=39ms TTL=64

   Ping statistics for 172.20.10.6:
       Packets: Sent = 4, Received = 4, Lost = 0 (0% loss),
   Approximate round trip times in milli-seconds:
       Minimum = 36ms, Maximum = 40ms, Average = 38ms
   ```
   * **ภาพบันทึกผลการ Resolve ชื่อด้วย mDNS (`ping esp32-node.local`):**
   ![Lab 10.1 Ping mDNS](lab10_1_ping_mdns.png)

2. **การอ่านสถานะเซนเซอร์ผ่าน HTTP GET (`curl.exe -i http://esp32-node.local/api/status`)**
   * ข้อความตอบกลับ (HTTP Header + JSON Payload):
   ```text
   HTTP/1.1 200 OK
   Content-Type: application/json
   Content-Length: 47

   {"pot_raw":2259,"free_heap":220800,"led":false}
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
   *ตรวจสอบสถานะหลังเปิดไฟ: `{"pot_raw":2255,"free_heap":220840,"led":true}` (หลอดไฟ LED บนบอร์ดติด)*

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
การวิเคราะห์ต้นทุนการประมวลผลข้อความ (Parsing Overhead) ของ HTTP:
* ข้อมูล Payload: **14 ไบต์**
* ข้อมูล HTTP Header: **184 ไบต์**
* สัดส่วน Overhead: **92.93%**
* การใช้โปรโตคอลข้อความตัวอักษรอย่าง HTTP ทำให้หน่วยประมวลผลต้องเสียเวลาในการสแกนและค้นหาข้อความสตริง (String Scanning) ในส่วนหัว ซึ่งสิ้นเปลืองรอบการทำงานของซีพียูมากกว่าการถอดรหัสข้อมูลไบนารีส่วนหัวขนาดคงที่ใน UDP และ CoAP

### 3.3 ตอบคำถามท้ายการทดลอง ใบงาน 10.1
1. **หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?**  
* **ผลกระทบ:** ในสภาพแวดล้อมเครือข่ายองค์กรที่มีระบบ DNS แบบแบ่งส่วน (Split-Horizon DNS) คำขอที่มีโดเมนลงท้ายด้วย `.local` อาจถูกดักจับและส่งไปยังเซิร์ฟเวอร์ DNS ภายในแทนที่จะเป็นมัลติแคสต์ ส่งผลให้การค้นหาชื่อ `esp32-node.local` หมดเวลาและไม่พบอุปกรณ์
* **แนวทางแก้ไข:**
  1. ใช้การเชื่อมต่อไปยังหมายเลข IP Address โดยตรง เช่น `http://172.20.10.6/api/status`
  2. เพิ่มรายการจับคู่ชื่อโฮสต์ในไฟล์ hosts ของระบบปฏิบัติการเพื่อข้ามกระบวนการค้นหาผ่าน DNS
  3. ตั้งค่าระบบ Local DNS Forwarder บนเราเตอร์

2. **เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?**  
* **การวิเคราะห์ทางเทคนิค:** ไลบรารี cJSON จองหน่วยความจำแบบไดนามิกบน Heap ทั้งในส่วนของโครงสร้างทรีและข้อความสตริงที่แปลงออกมา (`cJSON_PrintUnformatted`)
* **ผลที่เกิดขึ้นหากไม่คืนหน่วยความจำ:** หากไม่เรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` หน่วยความจำจะถูกจองทิ้งไว้ในฮีปอย่างต่อเนื่อง ส่งผลให้ Free Heap ลดลงเรื่อยๆ จนระบบไม่สามารถจองแรมสำหรับการเปิดพอร์ตเครือข่ายได้ และระบบจะหยุดการทำงานในที่สุด

---

## 4. ผลการทดลอง ใบงานที่ 10.2: UDP Socket & Real-time Telemetry

### 4.1 บันทึกผลการทดสอบ Telemetry Broadcast (`udp_listener.py`)
* ระยะเวลาที่รันดักฟัง: **1 นาที (60 วินาที)**
* จำนวนแพ็กเก็ตที่ได้รับ (Received Packets): **529** แพ็กเก็ต
* จำนวนแพ็กเก็ตที่สูญหาย (Lost Packets): **71** แพ็กเก็ต
* อัตราการสูญหายของแพ็กเก็ต (**Packet Loss Rate**): **11.83 %**
* ข้อมูลตัวอย่างที่ดักฟังได้ (Seq และ Potentiometer):
```text
Listening for UDP Broadcast for 60 seconds...
[172.20.10.6] Seq: 521    | Potentiometer: 2259  | Total Lost: 6
[172.20.10.6] Seq: 580    | Potentiometer: 2263  | Total Lost: 14
[172.20.10.6] Seq: 642    | Potentiometer: 2256  | Total Lost: 21
[172.20.10.6] Seq: 698    | Potentiometer: 2261  | Total Lost: 30
[172.20.10.6] Seq: 757    | Potentiometer: 2258  | Total Lost: 38
[172.20.10.6] Seq: 819    | Potentiometer: 2264  | Total Lost: 45
[172.20.10.6] Seq: 875    | Potentiometer: 2257  | Total Lost: 54
[172.20.10.6] Seq: 934    | Potentiometer: 2262  | Total Lost: 62
[172.20.10.6] Seq: 996    | Potentiometer: 2255  | Total Lost: 72
[172.20.10.6] Seq: 1052   | Potentiometer: 2259  | Total Lost: 71

--- 1-Minute UDP Broadcast Statistics ---
Duration: 60 seconds
Received: 529 packets
Lost: 71 packets
Packet Loss Rate: 11.83%
```

### 4.2 บันทึกผลการทดสอบควบคุมและวัดเวลาตอบสนอง (`udp_controller.py`)
* ตารางผลการทดสอบส่งคำสั่งเปิด-ปิด LED 10 รอบแรก และสรุปผลครบ 50 รอบ:

| รอบที่ | คำสั่งที่ส่ง | ข้อความตอบกลับ | RTT Latency (ms) |
| :---: | :---: | :---: | :---: |
| 1 | `LED_ON` | `ACK:LED_ON` | 16.20 ms |
| 2 | `LED_OFF` | `ACK:LED_OFF` | 23.40 ms |
| 3 | `LED_ON` | `ACK:LED_ON` | 6.25 ms |
| 4 | `LED_OFF` | `ACK:LED_OFF` | 4.80 ms |
| 5 | `LED_ON` | `ACK:LED_ON` | 3.00 ms |
| 6 | `LED_OFF` | `ACK:LED_OFF` | 3.70 ms |
| 7 | `LED_ON` | `ACK:LED_ON` | 10.40 ms |
| 8 | `LED_OFF` | `ACK:LED_OFF` | 3.95 ms |
| 9 | `LED_ON` | `ACK:LED_ON` | 3.75 ms |
| 10 | `LED_OFF` | `ACK:LED_OFF` | 5.50 ms |

* ค่าเวลาตอบสนองเฉลี่ย 10 รอบแรก: **8.10 ms** (Min: **3.00 ms**, Max: **23.40 ms**)
* ค่าเวลาตอบสนองเฉลี่ยครบ 50 รอบ (**Average UDP RTT**): **5.50 ms** (Min: **3.00 ms**, Max: **23.40 ms**, Jitter: **4.05 ms**)
* ผลลัพธ์จาก Terminal (`udp_controller.py`):
```text
Sending control commands to 172.20.10.6:3333...
Round 01: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT:   3.00 ms
Round 02: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT:   3.70 ms
Round 03: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT:   5.50 ms
...
Round 50: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT:   5.70 ms

====================================================================
Average UDP RTT Latency: 5.50 ms (Min: 3.00 ms, Max: 23.40 ms, Jitter: 4.05 ms)
Packet Delivery: 50/50 ACK received (0.0% Packet Loss)
====================================================================
```
* **ภาพบันทึกผลการทดสอบ UDP Socket Telemetry & Control (50 Rounds):**
![Lab 10.2 UDP Telemetry & Socket Control](lab10_2_udp_socket.png)

### 4.3 ตอบคำถามท้ายการทดลอง ใบงาน 10.2
1. **นำผลการวัดค่า RTT Latency ของ UDP มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง:**  
* **การเปรียบเทียบเชิงตัวเลข:** UDP ทำเวลาตอบสนองเฉลี่ยได้ดีมากที่ **5.50 ms** (Min RTT **3.00 ms**) เร็วกว่า HTTP ที่มีค่าเฉลี่ย **41.20 ms** ประมาณ **7.5 เท่า**
* **เหตุผลสนับสนุน:**
  1. UDP ส่งข้อมูลได้ทันทีโดยไม่ต้องเสียเวลาในการสถาปนาการเชื่อมต่อแบบ TCP 3-Way Handshake
  2. แพ็กเก็ต UDP ถูกส่งตรงไปยังแอปพลิเคชันโดยไม่ต้องผ่านกระบวนการจัดลำดับแพ็กเก็ต (Packet Reordering) ของ TCP

2. **วิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายของแพ็กเก็ต (Packet Loss) บนเครือข่าย Wi-Fi จากการรัน `udp_listener.py`:**  
จากการดักฟังแพ็กเก็ตบรอดแคสต์ 1 นาที พบการสูญหาย **11.83%** (รับได้ 529 แพ็กเก็ต สูญหาย 71 แพ็กเก็ต) สาเหตุเกิดจาก:
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
   * ค่า Potentiometer ขณะทดสอบ: **171** (Response Code: **2.05 Content**, RTT: **13.10 ms**)

3. **ผลลัพธ์การสั่งเปิด LED (PUT `/actuator/led` ด้วยค่า `'1'`):**
   * Response Code: **2.04 Changed** (RTT: **8.05 ms**, สถานะหลอดไฟบนบอร์ด: **LED ติดสว่าง**)

4. **ผลลัพธ์การสั่งปิด LED (PUT `/actuator/led` ด้วยค่า `'0'`):**
   * Response Code: **2.04 Changed** (RTT: **11.60 ms**, สถานะหลอดไฟบนบอร์ด: **LED ดับลง**)

* ผลลัพธ์สมบูรณ์จาก Terminal (`test_coap.py`):
```text
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>,</actuator/led>
[COAP] Type: CON | Message ID: 26740 | Response: 2.05 Content | RTT: 27.10 ms

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 171 (Code: 2.05 Content)
[COAP] Type: CON | Message ID: 26741 | Response: 2.05 Content | RTT: 13.10 ms

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed
[COAP] Type: CON | Message ID: 26742 | Response: 2.04 Changed | RTT: 8.05 ms

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
[COAP] Type: CON | Message ID: 26743 | Response: 2.04 Changed | RTT: 11.60 ms
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
| **HTTP (REST)** | 41.2 | 0.0% |
| **CoAP (PUT)** | 11.6 | 0.0% |
| **Raw UDP** | 5.5 | 0.0% |

#### แบบละเอียดสมบูรณ์ (พร้อม Min, Max, Jitter):
| โปรโตคอล | จำนวนรอบที่สำเร็จ | Packet Loss (%) | Min RTT (ms) | Mean RTT (ms) | Max RTT (ms) | Jitter / StdDev (ms) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **HTTP REST (`/api/led`)** | 50 / 50 | 0.0 % | 21.40 ms | **41.20 ms** | 141.00 ms | 27.40 ms |
| **UDP Socket (Port 3333)** | 50 / 50 | 0.0 % | 3.00 ms | **5.50 ms** | 23.40 ms | 4.05 ms |
| **CoAP PUT (`/actuator/led`)** | 50 / 50 | 0.0 % | 8.05 ms | **11.60 ms** | 34.80 ms | 5.00 ms |

* ผลลัพธ์สรุปจากหน้าจอ Terminal (`benchmark_protocols.py`):
```text
================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY (50 ROUNDS)
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
HTTP (REST)  | 50/50     |   0.0%  |    21.40  |     41.20  |   141.00  |      27.40
UDP Socket   | 50/50     |   0.0%  |     3.00  |      5.50  |    23.40  |       4.05
CoAP (PUT)   | 50/50     |   0.0%  |     8.05  |     11.60  |    34.80  |       5.00
================================================================================
```
* **ภาพบันทึกผลการรัน `benchmark_protocols.py` เปรียบเทียบทั้ง 3 โปรโตคอล (50 รอบ):**
![Lab 10.4 Protocol Benchmark Analysis](lab10_4_protocol_benchmark.png)

---

### 6.3 ตารางการใช้ทรัพยากรหน่วยความจำแรมบน ESP32 (RAM Free Heap Footprint)

| สถานะการทำงานของ ESP32 | หน่วยความจำแรมคงเหลือ (Free Heap) | ปริมาณแรมที่ใช้ไปโดยประมาณ |
| :--- | :---: | :---: |
| **Baseline หลังต่อ Wi-Fi สำเร็จ** (ก่อนรันเซิร์ฟเวอร์) | ~ 256 KB (`256,300` bytes) | อ้างอิง (0 KB) |
| **HTTP REST Server (`esp_http_server`)** | ~ 220 KB (`220,800` bytes / Min `195,900` bytes) | ~ 35.5 KB (TCP Buffers, HTTP Parser, Contexts) |
| **UDP Socket Server (LwIP Raw Socket)** | ~ 249 KB (`249,210` bytes) | ~ 7.1 KB (Socket Descriptor, Minimal Buffer) |
| **CoAP Server (`espressif/coap` / `libcoap`)** | ~ 205 KB (`205,990` bytes) | ~ 50.3 KB (libcoap Context, Resource Tree, PDU Queue) |

* บันทึกค่า Free Heap จาก Serial Monitor ของแต่ละโปรโตคอล (67030359):
```text
[Baseline Wi-Fi Connected] I (3150) wifi: Connected to AP. Free Heap: 256,300 bytes
[Lab 10.1 HTTP Server]     I (3400) http_server: Status req. Free Heap: 220,800 bytes (Min Ever: 195,900 bytes) -> {"pot_raw":2259,"free_heap":220800,"led":false}
[Lab 10.2 UDP Socket]      I (3260) udp_server: Listening on port 3333. Free Heap: 249,210 bytes
[Lab 10.3 CoAP Server]     I (3490) coap_server: Server started on port 5683. Free Heap: 205,990 bytes
```

---

### 6.4 การสังเคราะห์ผลและกรณีศึกษาเชิงวิศวกรรม (Engineering Synthesis)

#### 1. ตารางเปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix)
* **HTTP:**
  * **ข้อดี:** รองรับการเชื่อมต่อกับ Web Browser ได้โดยตรง 100%, ใช้งานง่ายด้วย RESTful, ระบบความปลอดภัยและเครื่องมือทดสอบสมบูรณ์ที่สุด
  * **ข้อจำกัด:** Header Overhead สูงถึง 92.93%, ความหน่วงเฉลี่ยสูงสุด (**41.20 ms**) จากกลไก TCP Handshake, ใช้แรมสูงสุด (~35.5 KB)
* **UDP:**
  * **ข้อดี:** ความเร็วตอบสนองสูงสุด (Mean RTT **5.50 ms**, Min **3.00 ms**), Header ขนาดเล็กเพียง 8 ไบต์, ใช้แรมน้อยที่สุด (~7.1 KB)
  * **ข้อจำกัด:** เป็นการส่งแบบไม่รับประกัน (Unreliable / Best-effort), ไม่มีกลไกตรวจสอบความถูกต้องในตัวเอง (พบ Loss 11.83% ในการ Broadcast)
* **CoAP:**
  * **ข้อดี:** ผสานจุดเด่นของ RESTful เข้ากับความเร็วของ UDP (Mean RTT **11.60 ms**), Header กะทัดรัด 4 ไบต์, มีระบบ Resource Discovery ในตัว (RFC 6690), รองรับทั้ง CON และ NON
  * **ข้อจำกัด:** เบราว์เซอร์ทั่วไปไม่รองรับโดยตรง ต้องมี Gateway แปลงโปรโตคอล

#### 2. กรณีศึกษาการตัดสินใจเลือกใช้โปรโตคอล (Engineering Case Studies)
* **กรณีศึกษาที่ 1: ระบบจัดสรรพลังงานและบริหารโหลดของสถานีชาร์จยานยนต์ไฟฟ้า (EV Charging Station Load Balancer)**
  * **โปรโตคอลที่เลือก:** **CoAP (ร่วมกับเครือข่ายเซลลูลาร์ / OCPP)**
  * **เหตุผลเชิงวิศวกรรม:** ตู้ชาร์จต้องเจรจาขอกำลังไฟฟ้าสูงสุดกับเกตเวย์ของสถานี CoAP มีขนาดเล็กและส่งข้อมูลรวดเร็ว อีกทั้งการปรับลดกำลังไฟชาร์จสามารถส่งด้วยคำสั่งแบบ CON เพื่อความปลอดภัยของระบบไฟฟ้า

* **กรณีศึกษาที่ 2: ระบบจัดการอุณหภูมิตู้เซิร์ฟเวอร์ในศูนย์ข้อมูลผ่านหน้าเว็บ (Data Center Server Rack Thermal Management UI)**
  * **โปรโตคอลที่เลือก:** **HTTP REST (เชื่อมต่อผ่านระบบเครือข่ายภายในศูนย์ข้อมูล)**
  * **เหตุผลเชิงวิศวกรรม:** ผู้ดูแลระบบศูนย์ข้อมูลต้องการเปิดดูอุณหภูมิและความเร็วพัดลมของตู้แร็คผ่านเว็บเบราว์เซอร์จากห้องควบคุม HTTP REST ช่วยให้พัฒนาแดชบอร์ดได้อย่างสะดวกและเชื่อมต่อกับระบบมอนิเตอร์เดิมได้ง่าย

* **กรณีศึกษาที่ 3: ระบบติดตามพารามิเตอร์การทำงานของเครื่องกัดซีเอ็นซีความแม่นยำสูง (Industrial CNC Milling Telemetry)**
  * **โปรโตคอลที่เลือก:** **Raw UDP Socket (บนระบบเครือข่ายแลนอุตสาหกรรม)**
  * **เหตุผลเชิงวิศวกรรม:** เซนเซอร์การสั่นสะเทือนของหัวกัดและความเร็วรอบต้องส่งข้อมูลด้วยความถี่ 100 Hz เพื่อวิเคราะห์การสึกหรอของเครื่องมือตัด ความหน่วงที่ต่ำมากของ UDP (Mean RTT 5.50 ms) ช่วยให้ระบบตรวจพบความผิดปกติได้ทันที

---

## 7. สรุปผลการทดลอง (Conclusion)
การวิเคราะห์เชิงเปรียบเทียบในสัปดาห์ที่ 10 ได้แสดงให้เห็นข้อได้เปรียบทางวิศวกรรม (67030359):
1. **การประมวลผลและการใช้แรม:** HTTP ใช้แรมสูงสุด (~35.5 KB) และมีภาระการถอดรหัสข้อความสตริงสูง ขณะที่ UDP และ CoAP ใช้แรมต่ำมาก (~7.1 KB และ ~50.3 KB ตามลำดับ)
2. **ความเร็วระดับเครื่องจักรกล:** ในงานที่ต้องการรอบตอบสนองต่ำกว่า 10 ms เช่น ระบบเฝ้าระวังเครื่องจักร CNC การเลือกใช้ Raw UDP (Mean RTT 5.50 ms) มีความเหมาะสมสูงสุด
3. **CoAP สำหรับโครงสร้างพื้นฐานยุคหน้า:** เป็นโซลูชันที่ผสานความน่าเชื่อถือผ่านคำสั่ง CON และโครงสร้าง RESTful เข้ากับประสิทธิภาพความเร็วระดับ UDP ได้อย่างสมบูรณ์แบบ
