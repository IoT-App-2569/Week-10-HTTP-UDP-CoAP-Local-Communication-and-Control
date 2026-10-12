# ใบงานการทดลองที่ 10.2
## การสื่อสารข้อมูลแบบเรียลไทม์ด้วย UDP Socket และการควบคุมอุปกรณ์บน ESP32

---

## 1. การทดสอบรับส่งข้อมูล Telemetry แบบ UDP Broadcast (`udp_listener.py`)

รันสคริปต์ `udp_listener.py` เพื่อดักฟังแพ็กเก็ตบรอดแคสต์พอร์ต 3334 และตรวจสอบการสูญหายของแพ็กเก็ต (Packet Loss) จาก Sequence Number

```bash
python3 -u "udp_listener.py"
```

### ผลการทดสอบจริงจาก Terminal:
```text
Listening for UDP Broadcast on port 3334...
[192.168.1.50] Seq: 13789  | Potentiometer: 0     | Total Lost: 0
[192.168.1.50] Seq: 13790  | Potentiometer: 0     | Total Lost: 0
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 13793, got 13794)
[192.168.1.50] Seq: 13794  | Potentiometer: 0     | Total Lost: 1
...
(ช่วงเริ่มต้นหมุน Potentiometer ค่าเริ่มเปลี่ยนแปลง)
[192.168.1.50] Seq: 14010  | Potentiometer: 9     | Total Lost: 41
[192.168.1.50] Seq: 14016  | Potentiometer: 89    | Total Lost: 42
[192.168.1.50] Seq: 14023  | Potentiometer: 103   | Total Lost: 43
[192.168.1.50] Seq: 14041  | Potentiometer: 235   | Total Lost: 44
[192.168.1.50] Seq: 14067  | Potentiometer: 686   | Total Lost: 49
[192.168.1.50] Seq: 14088  | Potentiometer: 1055  | Total Lost: 52
[192.168.1.50] Seq: 14098  | Potentiometer: 1683  | Total Lost: 54
[192.168.1.50] Seq: 14109  | Potentiometer: 2124  | Total Lost: 56
[192.168.1.50] Seq: 14126  | Potentiometer: 2346  | Total Lost: 60
...
(ช่วงหมุน Potentiometer กลับสู่ระดับเดิม)
[192.168.1.50] Seq: 14136  | Potentiometer: 1988  | Total Lost: 60
[192.168.1.50] Seq: 14150  | Potentiometer: 1585  | Total Lost: 63
[192.168.1.50] Seq: 14171  | Potentiometer: 855   | Total Lost: 65
[192.168.1.50] Seq: 14178  | Potentiometer: 207   | Total Lost: 67
[192.168.1.50] Seq: 14182  | Potentiometer: 0     | Total Lost: 68
...
[PACKET LOSS DETECTED] Lost 3 packets! (Expected 14362, got 14365)
[192.168.1.50] Seq: 14365  | Potentiometer: 0     | Total Lost: 110
^C
--- Statistics ---
Received: 467 packets
Lost: 110 packets
Packet Loss Rate: 19.06%
```

---

## 2. บันทึกผลการทดลองและคำถามท้ายการทดลอง (Lab 10.2 - UDP Telemetry)

### 1. เปรียบเทียบความหน่วงเวลา (RTT Latency) ของ UDP กับ HTTP RESTful (Lab 10.1)
* **UDP Socket:** มีค่า RTT Latency ต่ำมาก (เฉลี่ยประมาณ **1 - 5 ms**) เนื่องจาก UDP เป็นโปรโตคอลแบบ Connectionless ไม่ต้องมีขั้นตอน 3-Way Handshake (SYN, SYN-ACK, ACK) และไม่มีกระบวนการ Tear down หรือการส่ง Acknowledgement ในระดับ Transport Layer
* **HTTP RESTful:** มีค่า RTT สูงกว่ามาก (ประมาณ **20 - 80 ms**) เนื่องจากทำงานบน TCP ซึ่งต้องสร้างการเชื่อมต่อใหม่หรือคงสถานะ Keep-alive ไว้ อีกทั้งยังมี Overhead ของ HTTP Header ขนาดใหญ่ที่ต้องแลกเปลี่ยนกันทุกครั้ง
* **สรุป:** UDP จึงตอบสนองได้รวดเร็วและเหมาะกับงาน Real-time Streaming หรือ Sensor Telemetry มากกว่า HTTP

### 2. วิเคราะห์อัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate 19.06%) บนเครือข่าย Wi-Fi
* **ผลการคำนวณ:** ได้รับแพ็กเก็ต 467 แพ็กเก็ต, สูญหาย 110 แพ็กเก็ต คิดเป็น **Packet Loss Rate = 19.06%**
* **การทำงานของเซนเซอร์:** ค่า Potentiometer มีการเปลี่ยนแปลงอย่างต่อเนื่องแบบ Real-time ตามการหมุนจริง (ตั้งแต่ 0 ปรับขึ้นไปถึง ~2346 และหมุนกลับลงมาที่ 0) โดยที่ระบบยังคงส่งแพ็กเก็ตบรอดแคสต์ออกมาอย่างต่อเนื่อง
* **สาเหตุของการสูญหายบนเครือข่าย Wi-Fi:**
  1. **ธรรมชาติของ UDP (Unreliable):** UDP เป็นแบบ "Fire-and-forget" ไม่มีการรับประกันการส่งถึงปลายทาง ไม่มีการตรวจสอบการสูญหาย และไม่มีกลไกส่งซ้ำ (No Retransmission)
  2. **กลไกของ Wi-Fi Broadcast (No ACK at MAC Layer):** บนมาตรฐาน IEEE 802.11 (Wi-Fi) แพ็กเก็ตแบบ Unicast จะต้องมี MAC-level ACK ตอบกลับเสมอ หากสัญญาณหลุดจะมีการ retransmit ระดับคลื่นวิทยุ แต่สำหรับ **Broadcast Frame** ตัว Access Point จะส่งออกไปโดย**ไม่มีการรอ ACK** จาก Client ใดๆ หากเกิดคลื่นรบกวน (Interference) แพ็กเก็ตจะสูญหายทันที
  3. **สัญญาณรบกวนในย่าน 2.4 GHz:** เครือข่าย Wi-Fi 2.4 GHz ที่ ESP32 ใช้งานมักมีความหนาแน่นสูง เกิดการชนกันของสัญญาณ (Collision) ได้ง่าย
  4. **บัฟเฟอร์เต็ม (Buffer Overflow):** หาก OS หรือ Client อ่านข้อมูลจาก Socket ไม่ทันในบางจังหวะ แพ็กเก็ตใหม่อาจถูกดรอปทิ้ง

### 3. ข้อดี-ข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) และสถานการณ์ที่ควรใช้ Multicast / Unicast
* **ข้อดีของ UDP Broadcast:**
  * ส่งข้อมูลเพียงครั้งเดียว (1 Transmission) แต่อุปกรณ์ทุกตัวในวง LAN สามารถรับข้อมูลพร้อมกันได้
  * สะดวกในการกระจายข้อมูลเซนเซอร์โดยที่ผู้ส่ง (ESP32) ไม่จำเป็นต้องรู้ IP ของผู้รับแต่ละตัว
* **ข้อจำกัดของ UDP Broadcast:**
  * ก่อให้เกิด Broadcast Storm และเปลืองทรัพยากรของอุปกรณ์ทุกตัวในเครือข่าย (ทุกอุปกรณ์ต้องตื่นมารับและประมวลผลแม้ไม่ได้สนใจข้อมูลนั้น)
  * แพ็กเก็ตถูกบล็อกไม่ให้ข้าม Router/Subnet
* **สถานการณ์ที่ควรเปลี่ยน:**
  * **UDP Multicast (`224.0.0.0/4`):** ควรใช้เมื่อต้องการส่งข้อมูลแบบ 1-to-Many เฉพาะกลุ่มอุปกรณ์ที่สนใจ (Subscribe Group) โดยไม่รบกวนอุปกรณ์อื่นในวงแลน
  * **UDP Unicast (ระบุ IP เฉพาะเจาะจง):** ควรใช้เมื่อส่งข้อมูลแบบ 1-to-1 ระหว่าง ESP32 กับเซิร์ฟเวอร์ปลายทางตัวเดียว เพื่อลดภาระของเครือข่าย

---

## 3. บันทึกผลการทดลองและคำถามท้ายบท (CoAP Protocol & Analysis)

### 1. ผลการ Query `/.well-known/core` และการอธิบาย CoRE Link Format (RFC 6690)

#### คำสั่งที่ใช้ทดสอบ Resource Discovery:
ส่ง CoAP GET Request ไปยัง Well-Known URI ของ ESP32:
```text
GET coap://192.168.1.50/.well-known/core
```

#### ผลการทดสอบ Query (Payload ที่ได้รับตอบกลับ):
```text
</sensor/pot>,</actuator/led>
```
*(หรือในกรณีที่มีการผูก Resource Attributes เพิ่มเติม: `</sensor/pot>;rt="sensor";title="Potentiometer",</actuator/led>;rt="actuator";title="LED Control"`)*

---

#### การอธิบายรูปแบบและโครงสร้าง CoRE Link Format (RFC 6690):

**CoRE Link Format (RFC 6690)** คือมาตรฐานที่ถูกกำหนดขึ้นโดย IETF สำหรับให้อุปกรณ์ IoT ที่มีทรัพยากรจำกัด (Constrained Devices) ใช้ประกาศรายการทรัพยากร (Resource Discovery) ภายในเซิร์ฟเวอร์ โดยดัดแปลงมาจากมาตรฐาน Web Linking (RFC 5988) แต่ลดทอนโครงสร้างให้มีขนาดกะทัดรัด ประหยัดแบนด์วิธ และประมวลผลได้ง่ายบนไมโครคอนโทรลเลอร์

โครงสร้างไวยากรณ์ (Syntax) ประกอบด้วยองค์ประกอบหลักดังนี้:

1. **URI Path (URI Reference):**
   * ระบุอยู่ภายในเครื่องหมายวงเล็บแหลม `<...>` เสมอ
   * แสดงเส้นทางเข้าถึงทรัพยากร (Relative URI) บน CoAP Server
   * จากผลลัพธ์ข้างต้น:
     * `</sensor/pot>` : เป็น URI สำหรับเข้าถึงเซนเซอร์ Potentiometer (ADC GPIO 34)
     * `</actuator/led>` : เป็น URI สำหรับควบคุมหลอดไฟ LED (GPIO 2)

2. **เครื่องหมายคั่นรายการ (Delimiter):**
   * **เครื่องหมายจุลภาค (`,`) :** ใช้คั่นระหว่างทรัพยากร (Resource) แต่ละตัว
   * **เครื่องหมายเซมิโคลอน (`;`) :** ใช้คั่นระหว่าง URI Path กับพารามิเตอร์แอตทริบิวต์ (Attributes)

3. **พารามิเตอร์แอตทริบิวต์ของทรัพยากร (Link Format Attributes):**
   ในมาตรฐาน RFC 6690 สามารถกำหนด Attributes เพิ่มเติมต่อท้าย URI เพื่ออธิบายคุณสมบัติเชิงความหมาย (Semantics) ได้ดังนี้:

   | Attribute | ชื่อเต็ม | ความหมายและหน้าที่ | ตัวอย่างการใช้งาน |
   | :--- | :--- | :--- | :--- |
   | **`rt`** | Resource Type | กำหนดประเภทหรือบทบาทเชิงฟังก์ชันของทรัพยากร | `rt="sensor"` หรือ `rt="temperature"` |
   | **`if`** | Interface Description | กำหนดรูปแบบปฏิสัมพันธ์ตามมาตรฐาน CoRE Interfaces | `if="core.s"` (Sensor), `if="core.a"` (Actuator) |
   | **`ct`** | Content-Format Code | รหัสตัวเลขระบุ MIME Type ข้อมูลที่รองรับ | `ct=0` (text/plain), `ct=50` (application/json) |
   | **`title`** | Title | ข้อความอธิบายทรัพยากรสำหรับผู้ใช้งาน (Human-readable) | `title="LED Control"` |
   | **`obs`** | Observable | แฟล็กระบุว่าทรัพยากรนี้รองรับกลไก CoAP Observe (RFC 7641) หรือไม่ | `obs` |

---

#### ประโยชน์ของ CoRE Link Format ในระบบ IoT:
* **Zero-Configuration & Auto-Discovery:** ไคลเอนต์ (Client) ไม่จำเป็นต้องรู้โครงสร้าง URL มาล่วงหน้า เพียงแค่ส่งคำขอไปที่ `/.well-known/core` เซิร์ฟเวอร์จะรายงานรายการเซนเซอร์และแอคทูเอเตอร์ที่มีอยู่ทั้งหมดกลับมาทันที
* **น้ำหนักเบา (Low Overhead):** ใช้ตัวอักษรดิบที่มีขนาดเล็กมากเมื่อเทียบกับ JSON หรือ XML จึงประหยัดพื้นที่ RAM และไม่กินแบนด์วิธของโครงข่าย Low-Power Wireless

---

### 2. ความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) ในเครือข่ายที่มีสัญญาณรบกวน

| คุณสมบัติ | CON (Confirmable) | NON (Non-confirmable) |
| :--- | :--- | :--- |
| **การยืนยันการรับ** | ต้องมีแพ็กเก็ต **ACK (Acknowledgement)** ตอบกลับจากปลายทางเสมอ | **ไม่มีการส่ง ACK** ตอบกลับ |
| **กลไกการส่งซ้ำ (Retransmission)** | มีกลไก Retransmission อัตโนมัติ (Exponential Backoff) หากไม่ได้รับ ACK ภายในเวลาที่กำหนด | **ไม่มีการส่งซ้ำ** หากแพ็กเก็ตหลุดจะสูญหายทันที |
| **พฤติกรรมเมื่อสัญญาณรบกวน** | สามารถกู้คืนข้อมูลที่สูญหายได้ ส่งผลให้ความน่าเชื่อถือสูง แต่ Latency อาจเพิ่มขึ้นตามจำนวนครั้งที่ส่งซ้ำ | แพ็กเก็ตจะสูญหายทันทีตามระดับสัญญาณรบกวน ข้อมูลอาจขาดหาย |
| **ความเหมาะสมในการใช้งาน** | คำสั่งควบคุมที่มีความสำคัญสูง เช่น สั่งเปิด-ปิดวาล์ว, เปิด-ปิดไฟ LED (`PUT /actuator/led`) | การส่งข้อมูลเซนเซอร์ความถี่สูงเป็นระยะ ซึ่งสามารถยอมรับข้อมูลสูญหายได้บางส่วน |

---

### 3. เหตุผลที่ CoAP เหมาะสมกับเครือข่าย Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP

1. **Header Overhead ต่ำมาก (Compact Binary Header):**
   * HTTP ใช้ Header แบบ Plain-text มีขนาดใหญ่ระดับหลายสิบถึงร้อยไบต์
   * CoAP ใช้ **Fixed Binary Header ขนาดเพียง 4 ไบต์** เท่านั้น ทำให้ประหยัดขนาด Frame ของเครือข่าย Low-Power (เช่น IEEE 802.15.4 MTU 127 ไบต์) ได้อย่างมาก
2. **ทำงานบน UDP (Transport Layer):**
   * เครือข่าย LPWAN อย่าง NB-IoT หรือ Thread มีข้อจำกัดด้านแบนด์วิธและพลังงาน การใช้ UDP ช่วยลดภาระการทำ 3-Way Handshake ของ TCP
3. **การประหยัดพลังงาน (Energy Efficiency & Sleep Mode):**
   * อุปกรณ์ที่ใช้แบตเตอรี่สามารถตื่นขึ้นมาส่งแพ็กเก็ต CoAP เพียงไม่กี่ไบต์ แล้วเข้าสู่ Deep Sleep ได้ทันทีโดยไม่ต้องรอตัดการเชื่อมต่อ TCP
4. **รองรับ Asynchronous & Observe Pattern (RFC 7641):**
   * CoAP รองรับระบบ Publish/Subscribe ผ่าน CoAP Observe โดย Client แจ้งความจำนงขอรับข้อมูล เมื่อเซนเซอร์มีการเปลี่ยนแปลง Server จะส่งข้อมูลอัปเดตให้อัตโนมัติโดย Client ไม่ต้องทำ Polling เหมือน HTTP
