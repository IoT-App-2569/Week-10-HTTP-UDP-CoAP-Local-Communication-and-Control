# ใบงานการทดลองที่ 10.3
## การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา (RFC 7252)

---

## 1. ผลการทดสอบการทำงานของ CoAP Client ด้วย Python (`test_coap.py`)

รันสคริปต์ `test_coap.py` ด้วยไลบรารี `aiocoap` เพื่อทดสอบ 4 ฟังก์ชันหลักในการสื่อสารกับบอร์ด ESP32 (`192.168.1.50:5683`)

```bash
python3 -u "test_coap.py"
```

### ผลการทดสอบจริงจาก Terminal
```text
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>,</actuator/led>

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 113 (Code: 2.05 Content)

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
```

---

### วิเคราะห์ผลการทำงานแต่ละขั้นตอน

1. **Resource Discovery (`GET /.well-known/core`)**
   * ได้รับรายการ Resource Directory ในรูปแบบ **CoRE Link Format (RFC 6690)** คือ `</sensor/pot>,</actuator/led>` แสดงว่า ESP32 เปิดให้บริการ 2 แหล่งทรัพยากร คือ ตัวอ่านค่าเซนเซอร์อนาล็อกและตัวควบคุมหลอดไฟ
2. **การอ่านค่าเซนเซอร์ (`GET /sensor/pot`)**
   * ได้รับค่าดิบของ Potentiometer **`113`**
   * ได้รับ CoAP Response Code **`2.05 Content`** (เทียบเท่ากับ HTTP 200 OK ที่มี Payload แนบกลับมา)
3. **การสั่งเปิดไฟ LED (`PUT /actuator/led` ด้วยข้อมูล payload `'1'`)**
   * ได้รับ CoAP Response Code **`2.04 Changed`** (เทียบเท่ากับ HTTP 204 No Content เพื่อยืนยันว่าสถานะทรัพยากรบนเซิร์ฟเวอร์ได้รับการเปลี่ยนแปลงสำเร็จ)
   * สังเกตไฟ LED บนบอร์ด ESP32 (GPIO 2) สว่างขึ้นทันที
4. **การสั่งปิดไฟ LED (`PUT /actuator/led` ด้วยข้อมูล payload `'0'`)**
   * ได้รับ CoAP Response Code **`2.04 Changed`**
   * สังเกตไฟ LED บนบอร์ด ESP32 (GPIO 2) ดับลงทันที

---

## 2. บันทึกผลการทดลองและคำถามท้ายบท (Lab Report & Questions)

### ข้อที่ 1 ผลการ Query `/.well-known/core` และการอธิบาย CoRE Link Format (RFC 6690)

#### ผลการทดสอบ Query
```text
</sensor/pot>,</actuator/led>
```

#### การอธิบายโครงสร้าง CoRE Link Format (RFC 6690)
**CoRE Link Format (RFC 6690)** คือรูปแบบข้อมูลมาตรฐานที่ออกแบบโดย IETF CoRE Working Group เพื่อใช้สำหรับการประกาศรายการและค้นหาทรัพยากร (Resource Discovery) สำหรับอุปกรณ์ที่มีข้อจำกัดด้านทรัพยากร โดยตัดทอนไวยากรณ์จาก Web Linking (RFC 5988) ให้กระชับ ใช้แบนด์วิธต่ำ และประมวลผลบนไมโครคอนโทรลเลอร์ได้รวดเร็ว

โครงสร้างไวยากรณ์ (Syntax Breakdown)
1. **URI Reference (`<...>`)**
   * แต่ละทรัพยากรจะถูกระบุเส้นทางสัมพัทธ์ (Relative Path) อยู่ภายในเครื่องหมายวงเล็บแหลม `< >`
   * `</sensor/pot>` บ่งบอกถึง Endpoint สำหรับอ่านค่า Potentiometer
   * `</actuator/led>` บ่งบอกถึง Endpoint สำหรับควบคุมหลอดไฟ LED
2. **ตัวคั่นรายการ (Delimiter)**
   * **เครื่องหมายจุลภาค (`,`)** ใช้แบ่งคั่นระหว่าง Resource แต่ละตัว
   * **เครื่องหมายเซมิโคลอน (`;`)** ใช้คั่นระหว่าง Path กับ Attribute พารามิเตอร์เพิ่มเติม
3. **Resource Attributes (คุณลักษณะเสริม)**
   ในมาตรฐาน RFC 6690 สามารถผูกแอตทริบิวต์เพิ่มเติมต่อท้าย Resource เพื่อให้ Client รับรู้ข้อมูลเชิงความหมาย (Semantic Information) ได้ เช่น
   * **`rt` (Resource Type)** ระบุประเภทเชิงฟังก์ชัน เช่น `rt="sensor"` หรือ `rt="actuator"`
   * **`if` (Interface Description)** ระบุอินเทอร์เฟซมาตรฐาน เช่น `if="core.s"` (Sensor Interface)
   * **`ct` (Content-Format)** รหัสระบุชนิดข้อมูล เช่น `ct=0` (text/plain), `ct=50` (application/json)
   * **`title`** ข้อความอธิบายความหมายที่มนุษย์อ่านเข้าใจ เช่น `title="Potentiometer"`
   * **`obs` (Observable)** ระบุว่า Resource นี้รองรับการเฝ้าติดตามแบบ Publish/Subscribe ผ่าน CoAP Observe (RFC 7641) หรือไม่

#### ประโยชน์ในระบบ IoT
* ทำให้ระบบเป็นแบบ **Self-Describing และ Zero-Configuration** กล่าวคือ CoAP Client สามารถยิงคำขอไปที่ Well-Known URI เพื่อค้นหาว่าอุปกรณ์ในเครือข่ายมีเซนเซอร์หรือแอคทูเอเตอร์อะไรบ้างได้โดยอัตโนมัติ โดยไม่ต้อง Hardcode URL ล่วงหน้า

---

### ข้อที่ 2 ความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ

| คุณสมบัติ / พารามิเตอร์ | CON (Confirmable) | NON (Non-confirmable) |
| :--- | :--- | :--- |
| **การยืนยันการรับ (Acknowledgement)** | ต้องมีแพ็กเก็ต **ACK** ตอบกลับจากฝั่งผู้รับเสมอ | **ไม่มีการตอบกลับ ACK** ใดๆ |
| **กลไกการส่งซ้ำ (Retransmission)** | มีกลไก Retransmission อัตโนมัติด้วย Exponential Backoff หากไม่ได้รับ ACK ภายใน Timeout | **ไม่มีกลไกส่งซ้ำ** หากแพ็กเก็ตหลุดจะสูญหายทันที |
| **พฤติกรรมเมื่อสัญญาณรบกวนสูง** | ข้อมูลไม่สูญหาย (High Reliability) เพราะจะพยายามส่งซ้ำจนกว่าจะสำเร็จ แต่ Latency จะแกว่งและเพิ่มขึ้นตามจำนวนครั้งที่ส่งซ้ำ | แพ็กเก็ตสูญหายตามอัตราสัญญาณรบกวน (Packet Loss สูง) แต่ Latency มีความคงที่และไม่เกิด Traffic ค้าง |
| **ภาระบนเครือข่าย (Network Overhead)** | สูงกว่า (ต้องส่ง Request + รอรับ ACK + แพ็กเก็ตส่งซ้ำ) | ต่ำที่สุด (ส่งครั้งเดียวจบ แบบ Fire-and-forget) |
| **ความเหมาะสมในการประยุกต์ใช้** | **คำสั่งควบคุมระดับวิกฤต (Critical Control)** เช่น สั่งเปิด-ปิดวาล์ว, ควบคุมไฟ LED (`PUT /actuator/led`) | **การส่งข้อมูลเซนเซอร์ความถี่สูง (Periodic Telemetry)** เช่น รายงานอุณหภูมิทุก 1 วินาที ซึ่งค่าปัจจุบันแทนที่ค่าเดิมได้ |

---

### ข้อที่ 3 ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?

1. **ขนาด Header กะทัดรัดมาก (Compact Binary Header)**
   * HTTP ใช้ Header แบบข้อความ (Plain-text ASCII) ซึ่งมีขนาดใหญ่ตั้งแต่ 100 ไบต์จนถึงระดับกิโลไบต์ (Overhead สูงเกิน 60-80%)
   * CoAP ใช้ **Fixed Binary Header ขนาดเพียง 4 ไบต์** เท่านั้น ทำให้สามารถบรรจุลงในเฟรมเครือข่ายขนาดเล็กอย่าง IEEE 802.15.4 (MTU ขนาดจำกัดเพียง 127 ไบต์ ที่ใช้ใน Thread และ Zigbee IP) ได้โดยไม่ต้องทำ Packet Fragmentation
2. **ทำงานบน UDP แทน TCP (Transport Layer)**
   * HTTP ทำงานบน TCP ซึ่งต้องทำกระบวนการ **3-Way Handshake (SYN, SYN-ACK, ACK)** ก่อนส่งข้อมูล และต้องส่ง ACK ทุกครั้ง ทำให้สิ้นเปลืองการรับ-ส่งข้อมูลบนเครือข่าย Low-Power
   * CoAP ทำงานบน UDP ไม่มีขั้นตอน Handshake ทำให้ลดจำนวนรอบการส่งคลื่นวิทยุ (Round Trips) ได้อย่างมหาศาล เหมาะกับเครือข่าย NB-IoT ที่มีข้อจำกัดด้าน Data Quota และ Bandwidth
3. **การประหยัดพลังงานและการนอนหลับของโหนด (Duty Cycling & Deep Sleep)**
   * โหนด IoT ที่ใช้แบตเตอรี่ (Battery-powered Nodes) จำเป็นต้องนอนหลับแบบ Deep Sleep ให้ได้มากที่สุด
   * ด้วย CoAP โหนดสามารถตื่นขึ้นมา เปิดวิทยุ ส่งแพ็กเก็ต UDP เพียงไม่กี่สิบไบต์ และกลับไปนอนหลับ (Sleep) ได้ในเวลาไม่กี่มิลลิวินาที โดยไม่ต้องรอเปิด/ปิด TCP Session เหมือน HTTP จึงยืดอายุการใช้งานแบตเตอรี่ได้เป็นเวลาหลายปี
4. **สนับสนุน Asynchronous Event-Driven ด้วย CoAP Observe (RFC 7641)**
   * HTTP ไม่รองรับการ Push ข้อมูลจาก Server มา Client โดยตรง ไคลเอนต์จึงต้องส่ง HTTP Polling ซ้ำๆ ซึ่งเปลืองพลังงานและแบนด์วิธ
   * CoAP มีคุณสมบัติ **Observe Pattern** ที่ทำงานคล้าย Publish/Subscribe ในระดับแอปพลิเคชัน ไคลเอนต์เพียงแค่ Observe ทิ้งไว้ 1 ครั้ง เมื่อค่าเซนเซอร์เปลี่ยนแปลง เซิร์ฟเวอร์จะส่งข้อมูลอัปเดตกลับมาเองโดยอัตโนมัติ
