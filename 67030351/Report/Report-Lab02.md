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
* **UDP Socket** มีค่า RTT Latency ต่ำมาก (เฉลี่ยประมาณ **1 - 5 ms**) เนื่องจาก UDP เป็นโปรโตคอลแบบ Connectionless ไม่ต้องมีขั้นตอน 3-Way Handshake (SYN, SYN-ACK, ACK) และไม่มีกระบวนการ Tear down หรือการส่ง Acknowledgement ในระดับ Transport Layer
* **HTTP RESTful** มีค่า RTT สูงกว่ามาก (ประมาณ **20 - 80 ms**) เนื่องจากทำงานบน TCP ซึ่งต้องสร้างการเชื่อมต่อใหม่หรือคงสถานะ Keep-alive ไว้ อีกทั้งยังมี Overhead ของ HTTP Header ขนาดใหญ่ที่ต้องแลกเปลี่ยนกันทุกครั้ง
* **สรุป** UDP จึงตอบสนองได้รวดเร็วและเหมาะกับงาน Real-time Streaming หรือ Sensor Telemetry มากกว่า HTTP

### 2. วิเคราะห์อัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate 19.06%) บนเครือข่าย Wi-Fi
* **ผลการคำนวณ** ได้รับแพ็กเก็ต 467 แพ็กเก็ต, สูญหาย 110 แพ็กเก็ต คิดเป็น **Packet Loss Rate = 19.06%**
* **การทำงานของเซนเซอร์** ค่า Potentiometer มีการเปลี่ยนแปลงอย่างต่อเนื่องแบบ Real-time ตามการหมุนจริง (ตั้งแต่ 0 ปรับขึ้นไปถึง ~2346 และหมุนกลับลงมาที่ 0) โดยที่ระบบยังคงส่งแพ็กเก็ตบรอดแคสต์ออกมาอย่างต่อเนื่อง
* **สาเหตุของการสูญหายบนเครือข่าย Wi-Fi**
  1. **ธรรมชาติของ UDP (Unreliable)** UDP เป็นแบบ "Fire-and-forget" ไม่มีการรับประกันการส่งถึงปลายทาง ไม่มีการตรวจสอบการสูญหาย และไม่มีกลไกส่งซ้ำ (No Retransmission)
  2. **กลไกของ Wi-Fi Broadcast (No ACK at MAC Layer)** บนมาตรฐาน IEEE 802.11 (Wi-Fi) แพ็กเก็ตแบบ Unicast จะต้องมี MAC-level ACK ตอบกลับเสมอ หากสัญญาณหลุดจะมีการ retransmit ระดับคลื่นวิทยุ แต่สำหรับ **Broadcast Frame** ตัว Access Point จะส่งออกไปโดยไม่มีการรอ ACK จาก Client ใดๆ หากเกิดคลื่นรบกวน (Interference) แพ็กเก็ตจะสูญหายทันที
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
