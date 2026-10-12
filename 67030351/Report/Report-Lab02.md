# ใบงานการทดลองที่ 10.2
## การสื่อสารข้อมูลแบบเรียลไทม์ด้วย UDP Socket และการควบคุมอุปกรณ์บน ESP32

---

## 1. การทดสอบรับส่งข้อมูล Telemetry แบบ UDP Broadcast (`udp_listener.py`)

รันสคริปต์ `udp_listener.py` เพื่อดักฟังแพ็กเก็ตบรอดแคสต์พอร์ต 3334 และตรวจสอบการสูญหายของแพ็กเก็ต (Packet Loss) จาก Sequence Number

```bash
python3 -u "udp_listener.py"
```

### ผลการทดสอบจริงจาก Terminal
```text
Listening for UDP Broadcast on port 3334...
[192.168.1.50] Seq: 1164   | Potentiometer: 0     | Total Lost: 0
[192.168.1.50] Seq: 1165   | Potentiometer: 0     | Total Lost: 0
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 1166, got 1167)
[192.168.1.50] Seq: 1167   | Potentiometer: 0     | Total Lost: 1
[192.168.1.50] Seq: 1168   | Potentiometer: 0     | Total Lost: 1
[192.168.1.50] Seq: 1169   | Potentiometer: 0     | Total Lost: 1
[192.168.1.50] Seq: 1170   | Potentiometer: 0     | Total Lost: 1
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 1171, got 1172)
[192.168.1.50] Seq: 1172   | Potentiometer: 0     | Total Lost: 2
...
[PACKET LOSS DETECTED] Lost 2 packets! (Expected 1537, got 1539)
[192.168.1.50] Seq: 1539   | Potentiometer: 0     | Total Lost: 63
[192.168.1.50] Seq: 1540   | Potentiometer: 0     | Total Lost: 63
^C
--- Statistics ---
Received: 314 packets
Lost: 63 packets
Packet Loss Rate: 16.71%
```

---

## 2. บันทึกผลการทดลองและคำถามท้ายการทดลอง (Lab 10.2 - UDP Telemetry)

### 1. เปรียบเทียบความหน่วงเวลา (RTT Latency) ของ UDP กับ HTTP RESTful (Lab 10.1)
* **UDP Socket** มีค่า RTT Latency ต่ำมาก (เฉลี่ยประมาณ **1 - 5 ms**) เนื่องจาก UDP เป็นโปรโตคอลแบบ Connectionless ไม่ต้องมีขั้นตอน 3-Way Handshake (SYN, SYN-ACK, ACK) และไม่มีกระบวนการ Tear down หรือการส่ง Acknowledgement ในระดับ Transport Layer
* **HTTP RESTful** มีค่า RTT สูงกว่ามาก (ประมาณ **20 - 80 ms**) เนื่องจากทำงานบน TCP ซึ่งต้องสร้างการเชื่อมต่อใหม่หรือคงสถานะ Keep-alive ไว้ อีกทั้งยังมี Overhead ของ HTTP Header ขนาดใหญ่ที่ต้องแลกเปลี่ยนกันทุกครั้ง
* **สรุป** UDP จึงตอบสนองได้รวดเร็วและเหมาะกับงาน Real-time Streaming หรือ Sensor Telemetry มากกว่า HTTP

### 2. วิเคราะห์อัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate 16.71%) บนเครือข่าย Wi-Fi
* **ผลการคำนวณ** ได้รับแพ็กเก็ต 314 แพ็กเก็ต, สูญหาย 63 แพ็กเก็ต คิดเป็น **Packet Loss Rate = 16.71%**
* **สาเหตุของการสูญหายบนเครือข่าย Wi-Fi**
  1. **ธรรมชาติของ UDP (Unreliable)** UDP เป็นแบบ "Fire-and-forget" ไม่มีการรับประกันการส่งถึงปลายทาง ไม่มีการตรวจสอบการสูญหาย และไม่มีกลไกส่งซ้ำ (No Retransmission)
  2. **กลไกของ Wi-Fi Broadcast (No ACK at MAC Layer)** บนมาตรฐาน IEEE 802.11 (Wi-Fi) แพ็กเก็ตแบบ Unicast จะต้องมี MAC-level ACK ตอบกลับเสมอ หากสัญญาณหลุดจะมีการ retransmit ระดับคลื่นวิทยุ แต่สำหรับ **Broadcast Frame** ตัว Access Point จะส่งออกไปโดย**ไม่มีการรอ ACK** จาก Client ใดๆ หากเกิดคลื่นรบกวน (Interference) แพ็กเก็ตจะสูญหายทันที
  3. **สัญญาณรบกวนในย่าน 2.4 GHz** เครือข่าย Wi-Fi 2.4 GHz ที่ ESP32 ใช้งานมักมีความหนาแน่นสูง เกิดการชนกันของสัญญาณ (Collision) ได้ง่าย
  4. **บัฟเฟอร์เต็ม (Buffer Overflow)** หาก OS หรือ Client อ่านข้อมูลจาก Socket ไม่ทันในบางจังหวะ แพ็กเก็ตใหม่อาจถูกดรอปทิ้ง

### 3. ข้อดี-ข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) และสถานการณ์ที่ควรใช้ Multicast / Unicast
* **ข้อดีของ UDP Broadcast**
  * ส่งข้อมูลเพียงครั้งเดียว (1 Transmission) แต่อุปกรณ์ทุกตัวในวง LAN สามารถรับข้อมูลพร้อมกันได้
  * สะดวกในการกระจายข้อมูลเซนเซอร์โดยที่ผู้ส่ง (ESP32) ไม่จำเป็นต้องรู้ IP ของผู้รับแต่ละตัว
* **ข้อจำกัดของ UDP Broadcast**
  * ก่อให้เกิด Broadcast Storm และเปลืองทรัพยากรของอุปกรณ์ทุกตัวในเครือข่าย (ทุกอุปกรณ์ต้องตื่นมารับและประมวลผลแม้ไม่ได้สนใจข้อมูลนั้น)
  * แพ็กเก็ตถูกบล็อกไม่ให้ข้าม Router/Subnet
* **สถานการณ์ที่ควรเปลี่ยน**
  * **UDP Multicast (`224.0.0.0/4`)** ควรใช้เมื่อต้องการส่งข้อมูลแบบ 1-to-Many เฉพาะกลุ่มอุปกรณ์ที่สนใจ (Subscribe Group) โดยไม่รบกวนอุปกรณ์อื่นในวงแลน
  * **UDP Unicast (ระบุ IP เฉพาะเจาะจง)** ควรใช้เมื่อส่งข้อมูลแบบ 1-to-1 ระหว่าง ESP32 กับเซิร์ฟเวอร์ปลายทางตัวเดียว เพื่อลดภาระของเครือข่าย

---

## 3. บันทึกผลการทดลองและคำถามท้ายบท (CoAP Protocol & Analysis)

### 1. ผลการ Query `/.well-known/core` และการอธิบาย CoRE Link Format (RFC 6690)

#### ผลการทดสอบ Query
```text
</sensor/pot>,</actuator/led>
```


#### การอธิบายโครงสร้าง CoRE Link Format (RFC 6690)
* **CoRE Link Format** เป็นมาตรฐานการประกาศรายการ Resource ของอุปกรณ์ IoT ที่มีข้อจำกัดด้านทรัพยากร (Constrained RESTful Environments)
* แต่ละ Resource จะถูกคั่นด้วยเครื่องหมายจุลภาค (`,`)
* **URI Path** ระบุอยู่ภายในเครื่องหมาย `<...>` เช่น `</sensor/pot>` และ `</actuator/led>`
* **Resource Attributes** ระบุต่อท้าย Path ด้วยเครื่องหมายเซมิโคลอน (`;`) เพื่อบอกคุณลักษณะ เช่น
  * `rt` (Resource Type) กำหนดประเภทของทรัพยากร เช่น `rt="sensor"`
  * `title` ข้อความอธิบายความหมายที่มนุษย์อ่านเข้าใจ
  * `if` (Interface Description) กำหนดรูปแบบอินเทอร์เฟซ

### 2. ความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) ในเครือข่ายที่มีสัญญาณรบกวน

| คุณสมบัติ | CON (Confirmable) | NON (Non-confirmable) |
| :--- | :--- | :--- |
| **การยืนยันการรับ** | ต้องมีแพ็กเก็ต **ACK (Acknowledgement)** ตอบกลับจากปลายทางเสมอ | **ไม่มีการส่ง ACK** ตอบกลับ |
| **กลไกการส่งซ้ำ (Retransmission)** | มีกลไก Retransmission อัตโนมัติ (Exponential Backoff) หากไม่ได้รับ ACK ภายในเวลาที่กำหนด | **ไม่มีการส่งซ้ำ** หากแพ็กเก็ตหลุดจะสูญหายทันที |
| **พฤติกรรมเมื่อสัญญาณรบกวน** | สามารถกู้คืนข้อมูลที่สูญหายได้ ส่งผลให้ความน่าเชื่อถือสูง แต่ Latency อาจเพิ่มขึ้นตามจำนวนครั้งที่ส่งซ้ำ | แพ็กเก็ตจะสูญหายทันทีตามระดับสัญญาณรบกวน ข้อมูลอาจขาดหาย |
| **ความเหมาะสมในการใช้งาน** | คำสั่งควบคุมที่มีความสำคัญสูง เช่น สั่งเปิด-ปิดวาล์ว, เปิด-ปิดไฟ LED (`PUT /actuator/led`) | การส่งข้อมูลเซนเซอร์ความถี่สูงเป็นระยะ ซึ่งสามารถยอมรับข้อมูลสูญหายได้บางส่วน |

### 3. เหตุผลที่ CoAP เหมาะสมกับเครือข่าย Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP

1. **Header Overhead ต่ำมาก (Compact Binary Header)**
   * HTTP ใช้ Header แบบ Plain-text มีขนาดใหญ่ระดับหลายสิบถึงร้อยไบต์
   * CoAP ใช้ **Fixed Binary Header ขนาดเพียง 4 ไบต์** เท่านั้น ทำให้ประหยัดขนาด Frame ของเครือข่าย Low-Power (เช่น IEEE 802.15.4 MTU 127 ไบต์) ได้อย่างมาก
2. **ทำงานบน UDP (Transport Layer)**
   * เครือข่าย LPWAN อย่าง NB-IoT หรือ Thread มีข้อจำกัดด้านแบนด์วิธและพลังงาน การใช้ UDP ช่วยลดภาระการทำ 3-Way Handshake ของ TCP
3. **การประหยัดพลังงาน (Energy Efficiency & Sleep Mode)**
   * อุปกรณ์ที่ใช้แบตเตอรี่สามารถตื่นขึ้นมาส่งแพ็กเก็ต CoAP เพียงไม่กี่ไบต์ แล้วเข้าสู่ Deep Sleep ได้ทันทีโดยไม่ต้องรอตัดการเชื่อมต่อ TCP
4. **รองรับ Asynchronous & Observe Pattern (RFC 7641)**
   * CoAP รองรับระบบ Publish/Subscribe ผ่าน CoAP Observe โดย Client แจ้งความจำนงขอรับข้อมูล เมื่อเซนเซอร์มีการเปลี่ยนแปลง Server จะส่งข้อมูลอัปเดตให้อัตโนมัติโดย Client ไม่ต้องทำ Polling เหมือน HTTP
