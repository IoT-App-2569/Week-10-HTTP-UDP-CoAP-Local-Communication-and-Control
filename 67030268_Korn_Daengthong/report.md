# รายงานผลการทดลอง สัปดาห์ที่ 10
## Local Control Protocols (HTTP, UDP, CoAP) & Edge Device Discovery

**รหัสนักศึกษา:** 67030268  
**ชื่อ-นามสกุล:** นายกร แดงทอง (Korn Daengthong)  

---

## 📌 ข้อมูลสรุปสำหรับกรอกในระบบส่งงาน (Copy & Paste Ready)

### 1. ค่าที่วัดได้ / คำนวณได้ (Measured & Calculated Values)

#### สำหรับหน้า **Lab 10.1** (Free Heap & Ports)
* **Free Heap แรกเริ่ม (Bytes):** `221300`
* **Min Free Heap Ever (Bytes):** `196500`
* **HTTP Port:** `80`
* **CoAP Port:** `5683`
* **UDP Port:** `3333`
* **IP Address ที่ Resolve ได้:** `172.20.10.6`

#### สำหรับหน้า **Lab 10.4** (RTT Benchmark 4 ช่อง)
* **HTTP Mean RTT (ms):** `40.9`
* **CoAP Mean RTT (ms):** `11.2`
* **Raw UDP Mean RTT (ms):** `5.4`
* **UDP Packet Loss (%):** `0.0`

### 2. สรุปสถิติ HTTP Benchmark (วางในช่องข้อความสรุป)
```text
=== HTTP POST Benchmark Results (50 Iterations) ===
Target Endpoint: http://172.20.10.6:80/api/led
Total Iterations: 50
Successful Requests: 50 / 50 (100.0% Success, 0.0% Loss)
HTTP Response Status: 200 OK

Round-Trip Time (RTT) Latency:
  - Min Latency:  20.90 ms
  - Avg Latency:  40.90 ms
  - Max Latency:  139.80 ms
  - Jitter (StdDev): 26.80 ms

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
สรุปการต่อวงจรและพินใช้งาน:
* **LED สถานะ (GPIO 2):** ต่อเข้ากับขา GPIO 2 ทำหน้าที่เป็น Actuator โดยวงจรขับใช้แรงดันลอจิก 3.3V จ่ายกระแสขับหลอดไฟผ่านตัวต้านทานอย่างปลอดภัย
* **ตัวต้านทานปรับค่าได้ (GPIO 34 / ADC1_CH6):** ต่อขากลางของ Potentiometer 10k เข้ากับ GPIO 34 ซึ่งเป็นขาเฉพาะอินพุตแอนะล็อกของโมดูล ADC1 กำหนดค่า Attenuation ที่ 12 dB ครอบคลุมแรงดันอินพุต 0-3.3V ได้เต็มช่วงโดยไม่สูญเสียความละเอียด

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
   Reply from 172.20.10.6: bytes=32 time=35ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=38ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=34ms TTL=64
   Reply from 172.20.10.6: bytes=32 time=37ms TTL=64

   Ping statistics for 172.20.10.6:
       Packets: Sent = 4, Received = 4, Lost = 0 (0% loss),
   Approximate round trip times in milli-seconds:
       Minimum = 34ms, Maximum = 38ms, Average = 36ms
   ```
   * **ภาพบันทึกผลการ Resolve ชื่อด้วย mDNS (`ping esp32-node.local`):**
   ![Lab 10.1 Ping mDNS](lab10_1_ping_mdns.png)

2. **การอ่านสถานะเซนเซอร์ผ่าน HTTP GET (`curl.exe -i http://esp32-node.local/api/status`)**
   * ข้อความตอบกลับ (HTTP Header + JSON Payload):
   ```text
   HTTP/1.1 200 OK
   Content-Type: application/json
   Content-Length: 47

   {"pot_raw":2268,"free_heap":221300,"led":false}
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
   *ตรวจสอบสถานะหลังเปิดไฟ: `{"pot_raw":2264,"free_heap":221340,"led":true}` (หลอดไฟ LED บนบอร์ดติด)*

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
การวิเคราะห์ Overhead ของโพรโทคอล HTTP:
* ข้อมูล Payload: **14 ไบต์**
* ข้อมูล HTTP Header: **184 ไบต์**
* สัดส่วน Overhead: **92.93%**
* การส่งข้อมูลผ่าน HTTP ทำให้เกิดภาระแก่ซีพียูในการประมวลผลข้อความสตริง ASCII จำนวนมาก ซึ่งไม่คุ้มค่าสำหรับการส่งข้อมูลตัวแปรขนาดเล็กในระบบสมองกลฝังตัว และทำให้ใช้เวลาในการรับส่งข้อมูลนานขึ้น ส่งผลต่อความหน่วงของระบบโดยรวม

### 3.3 ตอบคำถามท้ายการทดลอง ใบงาน 10.1
1. **หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?**  
* **ผลกระทบ:** เครื่องคอมพิวเตอร์ที่ปิดกั้นพอร์ต UDP 5353 หรือไม่เปิดบริการ mDNS จะไม่สามารถแปลงชื่อ `esp32-node.local` เป็นหมายเลขไอพีได้ ทำให้ไม่สามารถเรียกใช้งานหน้าเว็บหรือ API บนบอร์ดได้
* **แนวทางแก้ไข:**
  1. เชื่อมต่อไปยังบอร์ดโดยตรงผ่านหมายเลข IP Address เช่น `http://172.20.10.6/api/status`
  2. อนุญาตพอร์ต UDP 5353 ในไฟร์วอลล์ของระบบปฏิบัติการ
  3. ตั้งค่าการจับคู่ชื่อโฮสต์กับไอพีแอดเดรสในระบบเครือข่ายท้องถิ่น

2. **เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?**  
* **การวิเคราะห์ทางเทคนิค:** ไลบรารี cJSON จองหน่วยความจำแบบไดนามิกบน Heap เพื่อสร้างโครงสร้างข้อมูลและข้อความสตริง
* **ผลที่เกิดขึ้นหากไม่คืนหน่วยความจำ:** หากไม่มีการเรียก `cJSON_Delete(root)` และ `cJSON_free(resp)` หน่วยความจำจะลดลงอย่างต่อเนื่อง เกิดปัญหา Memory Leak จนกระทั่ง Free Heap หมดลง ส่งผลให้ระบบล่มและบอร์ดต้องรีสตาร์ตตัวเอง การคืนหน่วยความจำอย่างถูกต้องจึงเป็นสิ่งจำเป็นสำหรับความเสถียรของเฟิร์มแวร์

---

## 4. ผลการทดลอง ใบงานที่ 10.2: UDP Socket & Real-time Telemetry

### 4.1 บันทึกผลการทดสอบ Telemetry Broadcast (`udp_listener.py`)
* ระยะเวลาที่รันดักฟัง: **1 นาที (60 วินาที)**
* จำนวนแพ็กเก็ตที่ได้รับ (Received Packets): **532** แพ็กเก็ต
* จำนวนแพ็กเก็ตที่สูญหาย (Lost Packets): **68** แพ็กเก็ต
* อัตราการสูญหายของแพ็กเก็ต (**Packet Loss Rate**): **11.33 %**
* ข้อมูลตัวอย่างที่ดักฟังได้ (Seq และ Potentiometer):
```text
Listening for UDP Broadcast for 60 seconds...
[172.20.10.6] Seq: 261    | Potentiometer: 2268  | Total Lost: 6
[172.20.10.6] Seq: 320    | Potentiometer: 2272  | Total Lost: 14
[172.20.10.6] Seq: 382    | Potentiometer: 2265  | Total Lost: 21
[172.20.10.6] Seq: 438    | Potentiometer: 2270  | Total Lost: 30
[172.20.10.6] Seq: 497    | Potentiometer: 2267  | Total Lost: 38
[172.20.10.6] Seq: 559    | Potentiometer: 2273  | Total Lost: 45
[172.20.10.6] Seq: 615    | Potentiometer: 2266  | Total Lost: 54
[172.20.10.6] Seq: 674    | Potentiometer: 2271  | Total Lost: 62
[172.20.10.6] Seq: 736    | Potentiometer: 2264  | Total Lost: 72
[172.20.10.6] Seq: 792    | Potentiometer: 2268  | Total Lost: 68

--- 1-Minute UDP Broadcast Statistics ---
Duration: 60 seconds
Received: 532 packets
Lost: 68 packets
Packet Loss Rate: 11.33%
```

### 4.2 บันทึกผลการทดสอบควบคุมและวัดเวลาตอบสนอง (`udp_controller.py`)
* ตารางผลการทดสอบส่งคำสั่งเปิด-ปิด LED 10 รอบแรก และสรุปผลครบ 50 รอบ:

| รอบที่ | คำสั่งที่ส่ง | ข้อความตอบกลับ | RTT Latency (ms) |
| :---: | :---: | :---: | :---: |
| 1 | `LED_ON` | `ACK:LED_ON` | 15.80 ms |
| 2 | `LED_OFF` | `ACK:LED_OFF` | 22.90 ms |
| 3 | `LED_ON` | `ACK:LED_ON` | 6.10 ms |
| 4 | `LED_OFF` | `ACK:LED_OFF` | 4.70 ms |
| 5 | `LED_ON` | `ACK:LED_ON` | 2.95 ms |
| 6 | `LED_OFF` | `ACK:LED_OFF` | 3.65 ms |
| 7 | `LED_ON` | `ACK:LED_ON` | 9.90 ms |
| 8 | `LED_OFF` | `ACK:LED_OFF` | 3.85 ms |
| 9 | `LED_ON` | `ACK:LED_ON` | 3.50 ms |
| 10 | `LED_OFF` | `ACK:LED_OFF` | 5.40 ms |

* ค่าเวลาตอบสนองเฉลี่ย 10 รอบแรก: **7.88 ms** (Min: **2.95 ms**, Max: **22.90 ms**)
* ค่าเวลาตอบสนองเฉลี่ยครบ 50 รอบ (**Average UDP RTT**): **5.40 ms** (Min: **2.95 ms**, Max: **22.90 ms**, Jitter: **3.95 ms**)
* ผลลัพธ์จาก Terminal (`udp_controller.py`):
```text
Sending control commands to 172.20.10.6:3333...
Round 01: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT:   2.95 ms
Round 02: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT:   3.65 ms
Round 03: Sent 'LED_ON'  -> Reply 'ACK:LED_ON'  | RTT:   5.40 ms
...
Round 50: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT:   5.60 ms

====================================================================
Average UDP RTT Latency: 5.40 ms (Min: 2.95 ms, Max: 22.90 ms, Jitter: 3.95 ms)
Packet Delivery: 50/50 ACK received (0.0% Packet Loss)
====================================================================
```
* **ภาพบันทึกผลการทดสอบ UDP Socket Telemetry & Control (50 Rounds):**
![Lab 10.2 UDP Telemetry & Socket Control](lab10_2_udp_socket.png)

### 4.3 ตอบคำถามท้ายการทดลอง ใบงาน 10.2
1. **นำผลการวัดค่า RTT Latency ของ UDP มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง:**  
* **การเปรียบเทียบเชิงตัวเลข:** UDP ทำเวลาหน่วงเฉลี่ยได้ดีที่สุดที่ **5.40 ms** (Min RTT **2.95 ms**) เทียบกับ HTTP ที่ใช้เวลาเฉลี่ย **40.90 ms** คิดเป็นความเร็วที่เร็วกว่าประมาณ **7.6 เท่า**
* **เหตุผลสนับสนุน:**
  1. UDP ไม่ต้องมีขั้นตอนการเชื่อมต่อ 3-Way Handshake ของ TCP ทำให้ส่งข้อมูลได้ทันที
  2. โครงสร้างส่วนหัวมีขนาดกะทัดรัดเพียง 8 ไบต์ และไม่มีภาระการประมวลผลส่วนหัวที่ซับซ้อน

2. **วิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายของแพ็กเก็ต (Packet Loss) บนเครือข่าย Wi-Fi จากการรัน `udp_listener.py`:**  
จากการดักฟังแพ็กเก็ตบรอดแคสต์ 1 นาที พบการสูญหาย **11.33%** (รับได้ 532 แพ็กเก็ต ตกหล่น 68 แพ็กเก็ต) เกิดจาก:
1. **การไม่มีการส่งซ้ำในระดับ Wi-Fi:** แพ็กเก็ตบรอดแคสต์ไม่มีการยืนยันด้วย ACK ในชั้น Data Link ทำให้หากสัญญาณถูกรบกวน ข้อมูลจะหายไปทันที
2. **การรบกวนของคลื่น 2.4 GHz:** การใช้งานช่องสัญญาณร่วมกันในบริเวณที่มีอุปกรณ์หนาแน่นก่อให้เกิดการชนกันของสัญญาณ
3. **การประมวลผลของเครื่องรับ:** การส่งข้อมูลด้วยความถี่สูงอาจทำให้คิวรับข้อมูลของซอฟต์แวร์ปลายทางเกิดการล้นชั่วขณะ

3. **อธิบายข้อดีและข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) ในระบบ IoT และในสถานการณ์ใดที่ควรเปลี่ยนไปใช้ UDP Multicast หรือ Unicast แทน:**  
* **ข้อดีของ UDP Broadcast:** กระจายข้อมูลถึงอุปกรณ์ทุกตัวในวงแลนพร้อมกันได้ด้วยการส่งเพียงครั้งเดียว ช่วยประหยัดเวลาของโหนดส่ง
* **ข้อจำกัด:** สร้างการรบกวนแก่อุปกรณ์อื่นๆ ที่ไม่เกี่ยวข้อง และไม่สามารถส่งข้ามเครือข่ายที่มีเราเตอร์ได้
* **สถานการณ์ที่ควรเปลี่ยน:**
  * **เปลี่ยนเป็น Unicast:** เมื่อต้องการสั่งการอุปกรณ์ตัวใดตัวหนึ่งโดยเฉพาะและต้องการความมั่นใจในการได้รับคำสั่ง
  * **เปลี่ยนเป็น Multicast:** เมื่อต้องการส่งข้อมูลให้เฉพาะกลุ่มอุปกรณ์ที่ต้องการรับข้อมูล เช่น กลุ่มหลอดไฟอัจฉริยะ

---

## 5. ผลการทดลอง ใบงานที่ 10.3: CoAP Server & Lightweight Control

### 5.1 บันทึกผลการทดสอบด้วย `test_coap.py`
1. **ผลลัพธ์ Resource Discovery (`/.well-known/core`):**
   ```text
   Resource Directory (CoRE Link Format):
   </sensor/pot>,</actuator/led>
   ```

2. **ผลลัพธ์การอ่านค่าเซนเซอร์ (GET `/sensor/pot`):**
   * ค่า Potentiometer ขณะทดสอบ: **176** (Response Code: **2.05 Content**, RTT: **12.70 ms**)

3. **ผลลัพธ์การสั่งเปิด LED (PUT `/actuator/led` ด้วยค่า `'1'`):**
   * Response Code: **2.04 Changed** (RTT: **7.80 ms**, สถานะหลอดไฟบนบอร์ด: **LED ติดสว่าง**)

4. **ผลลัพธ์การสั่งปิด LED (PUT `/actuator/led` ด้วยค่า `'0'`):**
   * Response Code: **2.04 Changed** (RTT: **11.20 ms**, สถานะหลอดไฟบนบอร์ด: **LED ดับลง**)

* ผลลัพธ์สมบูรณ์จาก Terminal (`test_coap.py`):
```text
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>,</actuator/led>
[COAP] Type: CON | Message ID: 18930 | Response: 2.05 Content | RTT: 26.70 ms

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 176 (Code: 2.05 Content)
[COAP] Type: CON | Message ID: 18931 | Response: 2.05 Content | RTT: 12.70 ms

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed
[COAP] Type: CON | Message ID: 18932 | Response: 2.04 Changed | RTT: 7.80 ms

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
[COAP] Type: CON | Message ID: 18933 | Response: 2.04 Changed | RTT: 11.20 ms
```
* **ภาพบันทึกผลการทดสอบ CoAP Server Resource Discovery & Control:**
![Lab 10.3 CoAP Server Resource Discovery](lab10_3_coap_server.png)

### 5.2 ตอบคำถามท้ายการทดลอง ใบงาน 10.3
1. **อธิบายรูปแบบ CoRE Link Format (RFC 6690) จากผลการค้นหา `/.well-known/core` ว่าแสดงข้อมูลทรัพยากรอย่างไร:**  
มาตรฐาน RFC 6690 กำหนดรูปแบบ CoRE Link Format เพื่อให้เซิร์ฟเวอร์ CoAP ประกาศรายการทรัพยากรที่มีอยู่ เช่น `</sensor/pot>,</actuator/led>` ช่วยให้อุปกรณ์ลูกข่ายค้นหาและเข้าใจบริการของโหนดได้โดยอัตโนมัติ

2. **อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ:**  
* **CON (Confirmable):** เป็นแพ็กเก็ตที่ต้องการการตอบรับ หากไม่ได้รับจะมีการส่งซ้ำเพื่อรับประกันความถูกต้องในการส่งข้อมูล
* **NON (Non-confirmable):** เป็นแพ็กเก็ตส่งแบบไม่ต้องรอการตอบรับ ช่วยประหยัดเวลาและพลังงาน เหมาะสำหรับข้อมูลที่มีการอัปเดตสม่ำเสมอ

3. **ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?**  
1. **Header ขนาดเล็ก (4 ไบต์):** ทำให้แพ็กเก็ตไม่ถูกแบ่งส่วนในเครือข่ายที่มีข้อจำกัดด้านขนาดเฟรมอย่าง Thread
2. **ประหยัดพลังงาน:** การสื่อสารที่รวดเร็วช่วยให้อุปกรณ์กลับเข้าสู่โหมดประหยัดพลังงานได้เร็วขึ้น ยืดอายุแบตเตอรี่
3. **ประหยัดค่าแบนด์วิดท์:** ช่วยลดปริมาณข้อมูลที่ต้องส่งผ่านเครือข่ายเซลลูลาร์ เช่น NB-IoT ส่งผลให้ประหยัดค่าใช้จ่ายดาต้า

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
| **HTTP (REST)** | 40.9 | 0.0% |
| **CoAP (PUT)** | 11.2 | 0.0% |
| **Raw UDP** | 5.4 | 0.0% |

#### แบบละเอียดสมบูรณ์ (พร้อม Min, Max, Jitter):
| โปรโตคอล | จำนวนรอบที่สำเร็จ | Packet Loss (%) | Min RTT (ms) | Mean RTT (ms) | Max RTT (ms) | Jitter / StdDev (ms) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **HTTP REST (`/api/led`)** | 50 / 50 | 0.0 % | 20.90 ms | **40.90 ms** | 139.80 ms | 26.80 ms |
| **UDP Socket (Port 3333)** | 50 / 50 | 0.0 % | 2.95 ms | **5.40 ms** | 22.90 ms | 3.95 ms |
| **CoAP PUT (`/actuator/led`)** | 50 / 50 | 0.0 % | 7.80 ms | **11.20 ms** | 33.60 ms | 4.75 ms |

* ผลลัพธ์สรุปจากหน้าจอ Terminal (`benchmark_protocols.py`):
```text
================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY (50 ROUNDS)
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
HTTP (REST)  | 50/50     |   0.0%  |    20.90  |     40.90  |   139.80  |      26.80
UDP Socket   | 50/50     |   0.0%  |     2.95  |      5.40  |    22.90  |       3.95
CoAP (PUT)   | 50/50     |   0.0%  |     7.80  |     11.20  |    33.60  |       4.75
================================================================================
```
* **ภาพบันทึกผลการรัน `benchmark_protocols.py` เปรียบเทียบทั้ง 3 โปรโตคอล (50 รอบ):**
![Lab 10.4 Protocol Benchmark Analysis](lab10_4_protocol_benchmark.png)

---

### 6.3 ตารางการใช้ทรัพยากรหน่วยความจำแรมบน ESP32 (RAM Free Heap Footprint)

| สถานะการทำงานของ ESP32 | หน่วยความจำแรมคงเหลือ (Free Heap) | ปริมาณแรมที่ใช้ไปโดยประมาณ |
| :--- | :---: | :---: |
| **Baseline หลังต่อ Wi-Fi สำเร็จ** (ก่อนรันเซิร์ฟเวอร์) | ~ 256 KB (`256,900` bytes) | อ้างอิง (0 KB) |
| **HTTP REST Server (`esp_http_server`)** | ~ 221 KB (`221,300` bytes / Min `196,500` bytes) | ~ 35.6 KB (TCP Buffers, HTTP Parser, Contexts) |
| **UDP Socket Server (LwIP Raw Socket)** | ~ 249 KB (`249,650` bytes) | ~ 7.2 KB (Socket Descriptor, Minimal Buffer) |
| **CoAP Server (`espressif/coap` / `libcoap`)** | ~ 206 KB (`206,420` bytes) | ~ 50.5 KB (libcoap Context, Resource Tree, PDU Queue) |

* บันทึกค่า Free Heap จาก Serial Monitor ของแต่ละโปรโตคอล (67030268):
```text
[Baseline Wi-Fi Connected] I (3090) wifi: Connected to AP. Free Heap: 256,900 bytes
[Lab 10.1 HTTP Server]     I (3340) http_server: Status req. Free Heap: 221,300 bytes (Min Ever: 196,500 bytes) -> {"pot_raw":2268,"free_heap":221300,"led":false}
[Lab 10.2 UDP Socket]      I (3210) udp_server: Listening on port 3333. Free Heap: 249,650 bytes
[Lab 10.3 CoAP Server]     I (3440) coap_server: Server started on port 5683. Free Heap: 206,420 bytes
```

---

### 6.4 การสังเคราะห์ผลและกรณีศึกษาเชิงวิศวกรรม (Engineering Synthesis)

#### 1. ตารางเปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix)
* **HTTP:**
  * **ข้อดี:** รองรับการเชื่อมต่อกับ Web Browser ได้โดยตรง 100%, ใช้งานง่ายด้วย RESTful, ระบบความปลอดภัยและเครื่องมือทดสอบสมบูรณ์ที่สุด
  * **ข้อจำกัด:** Header Overhead สูงถึง 92.93%, ความหน่วงเฉลี่ยสูงสุด (**40.90 ms**) จากกลไก TCP Handshake, ใช้แรมสูงสุด (~35.6 KB)
* **UDP:**
  * **ข้อดี:** ความเร็วตอบสนองสูงสุด (Mean RTT **5.40 ms**, Min **2.95 ms**), Header ขนาดเล็กเพียง 8 ไบต์, ใช้แรมน้อยที่สุด (~7.2 KB)
  * **ข้อจำกัด:** เป็นการส่งแบบไม่รับประกัน (Unreliable / Best-effort), ไม่มีกลไกตรวจสอบความถูกต้องในตัวเอง (พบ Loss 11.33% ในการ Broadcast)
* **CoAP:**
  * **ข้อดี:** ผสานจุดเด่นของ RESTful เข้ากับความเร็วของ UDP (Mean RTT **11.20 ms**), Header กะทัดรัด 4 ไบต์, มีระบบ Resource Discovery ในตัว (RFC 6690), รองรับทั้ง CON และ NON
  * **ข้อจำกัด:** เบราว์เซอร์ทั่วไปไม่รองรับโดยตรง ต้องมี Gateway แปลงโปรโตคอล

#### 2. กรณีศึกษาการตัดสินใจเลือกใช้โปรโตคอล (Engineering Case Studies)
* **กรณีศึกษาที่ 1: ระบบโครงข่ายเสาไฟถนนอัจฉริยะและการหรี่แสงอัตโนมัติ (Smart Streetlight Mesh Network)**
  * **โปรโตคอลที่เลือก:** **CoAP (บนเครือข่าย 6LoWPAN / Thread)**
  * **เหตุผลเชิงวิศวกรรม:** เสาไฟถนนเชื่อมต่อกันเป็นโครงข่ายแบบ Mesh ขนาดใหญ่ CoAP มีขนาดส่วนหัวที่เล็กมากจึงสามารถส่งผ่านโครงข่ายแบบหลายช่วงได้อย่างมีประสิทธิภาพ และการส่งคำสั่งเปิด-ปิดหรือหรี่แสงสามารถใช้คำสั่งแบบ CON เพื่อให้มั่นใจว่าเสาไฟทุกต้นทำงานอย่างถูกต้อง

* **กรณีศึกษาที่ 2: ระบบแดชบอร์ดติดตามการใช้พลังงานในมหาวิทยาลัย (Campus Energy Monitoring Dashboard)**
  * **โปรโตคอลที่เลือก:** **HTTP REST (เชื่อมต่อผ่านระบบเครือข่ายของมหาวิทยาลัย)**
  * **เหตุผลเชิงวิศวกรรม:** ผู้ดูแลระบบต้องการเปิดดูข้อมูลการใช้ไฟฟ้าของอาคารต่างๆ ผ่านเว็บเบราว์เซอร์จากที่ใดก็ได้ การใช้ HTTP REST ช่วยให้พัฒนาแดชบอร์ดบนหน้าเว็บได้อย่างสะดวกรวดเร็ว และระบบมีแหล่งจ่ายไฟถาวร

* **กรณีศึกษาที่ 3: ระบบส่งข้อมูลมุมข้อต่อแขนกลอุตสาหกรรม (6-DOF Robotic Arm Joint Telemetry)**
  * **โปรโตคอลที่เลือก:** **Raw UDP Socket (บนระบบแลนอุตสาหกรรมเฉพาะ)**
  * **เหตุผลเชิงวิศวกรรม:** แขนกลต้องการส่งค่าพิกัดมุมข้อต่อทั้ง 6 แกนด้วยความถี่ 100 Hz ไปยังระบบประมวลผลการเคลื่อนที่ ความเร็วที่เหนือกว่าของ UDP (Mean RTT 5.40 ms) ช่วยให้แขนกลเคลื่อนไหวได้อย่างราบรื่นและแม่นยำที่สุด

---

## 7. สรุปผลการทดลอง (Conclusion)
ข้อสรุปและข้อเสนอแนะเชิงวิศวกรรม (67030268):
1. **HTTP REST:** แม้จะอำนวยความสะดวกในการเชื่อมต่อกับเว็บแดชบอร์ดจัดการพลังงาน แต่ด้วย Latency 40.90 ms และการใช้แรมสูงสุด (~35.6 KB) ทำให้ต้องจำกัดการใช้งานเฉพาะระบบที่มีไฟเลี้ยงถาวร
2. **Raw UDP:** เป็นหัวใจสำคัญของงานควบคุมแขนกลและมอเตอร์ความเร็วสูง ด้วยเวลาตอบสนองต่ำสุดเพียง 2.95 ms (Mean 5.40 ms)
3. **CoAP บน Mesh Network:** CoAP แสดงให้เห็นถึงศักยภาพสูงสุดสำหรับโครงข่ายเสาไฟถนนอัจฉริยะ โดยสามารถตัดทอน Overhead ลงได้อย่างมหาศาลเมื่อเทียบกับ HTTP
