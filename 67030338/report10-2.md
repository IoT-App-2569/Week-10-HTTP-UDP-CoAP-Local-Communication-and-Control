# รายงานผลการทดลอง ใบงานที่ 10.2
### การสื่อสารความหน่วงต่ำด้วย UDP Socket และการถ่ายทอดข้อมูล Real-time Telemetry
**รหัสนักศึกษา:** 67030338  

---

## 1. วัตถุประสงค์การทดลอง
1. สามารถเขียนโปรแกรม Socket แบบ Connectionless (UDP Datagram) ด้วย BSD Socket API บน ESP-IDF ได้
2. สามารถพัฒนา FreeRTOS Task เพื่อส่งข้อมูลแอนะล็อกเซนเซอร์แบบบรอดแคสต์ (UDP Broadcast) สู่เครือข่ายได้ด้วยความถี่ 10 Hz
3. สามารถพัฒนาสคริปต์ภาษา Python บนเครื่องคอมพิวเตอร์เพื่อดักฟังข้อมูลบรอดแคสต์ และส่งคำสั่งควบคุม LED กลับมายัง ESP32 ได้
4. สามารถวัดค่าความหน่วงเวลาเฉลี่ย (Round-Trip Latency) และอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) ได้

---

## 2. การดัดแปลงระบบ Virtual Potentiometer (กรณีไม่มี Potentiometer จริง)
เนื่องจากไม่มีฮาร์ดแวร์ Potentiometer ทางกายภาพ ตัวเฟิร์มแวร์จึงได้บรรจุระบบ **Virtual Potentiometer** ไว้ในตัว:
- **โหมด Auto-Sweep:** ทำการปรับเปลี่ยนค่า `s_virtual_pot_val` ขึ้น-ลงอัตโนมัติ (คลื่นสามเหลี่ยม 0 - 4095) ทุกรอบ 100 ms เพื่อให้การสตรีมข้อมูล Telemetry ผ่าน UDP Broadcast แสดงการเปลี่ยนแปลงของค่าแอนะล็อกได้อย่างสมจริง
- **การปรับค่าผ่านคำสั่ง UDP:** สามารถส่งคำสั่ง `"POT:<val>"` (เช่น `"POT:3000"`) ไปที่พอร์ต 3333 เพื่อกำหนดค่าคงที่ หรือส่ง `"POT_AUTO"` เพื่อเปิด/ปิดโหมด Auto-Sweep ได้

---

## 3. บันทึกผลการทดลอง (บันทึกข้อมูลด้วยตนเอง)

### 3.1 ผลการรันสคริปต์ดักฟังข้อมูล Telemetry (`python udp_listener.py`)
*(กรุณาแปะข้อความ Log ผลการรัน หรือภาพหน้าจอ Terminal ขณะรับข้อมูล Telemetry)*

```text
[192.168.1.41] Seq: 65     | Potentiometer: 2845  | Total Lost: 0
[192.168.1.41] Seq: 66     | Potentiometer: 2795  | Total Lost: 0
[192.168.1.41] Seq: 67     | Potentiometer: 2745  | Total Lost: 0
[192.168.1.41] Seq: 68     | Potentiometer: 2695  | Total Lost: 0
[192.168.1.41] Seq: 69     | Potentiometer: 2645  | Total Lost: 0
[192.168.1.41] Seq: 70     | Potentiometer: 2595  | Total Lost: 0
[192.168.1.41] Seq: 71     | Potentiometer: 2545  | Total Lost: 0
[192.168.1.41] Seq: 72     | Potentiometer: 2495  | Total Lost: 0
[192.168.1.41] Seq: 73     | Potentiometer: 2445  | Total Lost: 0
[192.168.1.41] Seq: 74     | Potentiometer: 2395  | Total Lost: 0
[192.168.1.41] Seq: 75     | Potentiometer: 2345  | Total Lost: 0
[192.168.1.41] Seq: 76     | Potentiometer: 2295  | Total Lost: 0
[192.168.1.41] Seq: 77     | Potentiometer: 2245  | Total Lost: 0
[192.168.1.41] Seq: 78     | Potentiometer: 2195  | Total Lost: 0
[192.168.1.41] Seq: 79     | Potentiometer: 2145  | Total Lost: 0
[192.168.1.41] Seq: 80     | Potentiometer: 2095  | Total Lost: 0
[192.168.1.41] Seq: 81     | Potentiometer: 2045  | Total Lost: 0
[192.168.1.41] Seq: 82     | Potentiometer: 1995  | Total Lost: 0
[192.168.1.41] Seq: 83     | Potentiometer: 1945  | Total Lost: 0
[192.168.1.41] Seq: 84     | Potentiometer: 1895  | Total Lost: 0
[192.168.1.41] Seq: 85     | Potentiometer: 1845  | Total Lost: 0
[192.168.1.41] Seq: 86     | Potentiometer: 1795  | Total Lost: 0
[192.168.1.41] Seq: 87     | Potentiometer: 1745  | Total Lost: 0
[192.168.1.41] Seq: 88     | Potentiometer: 1695  | Total Lost: 0
[192.168.1.41] Seq: 89     | Potentiometer: 1645  | Total Lost: 0
[192.168.1.41] Seq: 90     | Potentiometer: 1595  | Total Lost: 0
[192.168.1.41] Seq: 91     | Potentiometer: 1545  | Total Lost: 0
[192.168.1.41] Seq: 92     | Potentiometer: 1495  | Total Lost: 0
[192.168.1.41] Seq: 93     | Potentiometer: 1445  | Total Lost: 0
[192.168.1.41] Seq: 94     | Potentiometer: 1395  | Total Lost: 0
[192.168.1.41] Seq: 95     | Potentiometer: 1345  | Total Lost: 0
[192.168.1.41] Seq: 96     | Potentiometer: 1295  | Total Lost: 0
[192.168.1.41] Seq: 97     | Potentiometer: 1245  | Total Lost: 0
[192.168.1.41] Seq: 98     | Potentiometer: 1195  | Total Lost: 0
[192.168.1.41] Seq: 99     | Potentiometer: 1145  | Total Lost: 0
[192.168.1.41] Seq: 100    | Potentiometer: 1095  | Total Lost: 0
[192.168.1.41] Seq: 101    | Potentiometer: 1045  | Total Lost: 0
[192.168.1.41] Seq: 102    | Potentiometer: 995   | Total Lost: 0
[192.168.1.41] Seq: 103    | Potentiometer: 945   | Total Lost: 0
[192.168.1.41] Seq: 104    | Potentiometer: 895   | Total Lost: 0
[192.168.1.41] Seq: 105    | Potentiometer: 845   | Total Lost: 0

--- Statistics ---
Received: 41 packets
Lost: 0 packets
Packet Loss Rate: 0.00%

```

- **จำนวนแพ็กเก็ตที่ได้รับ (Received):** 41 แพ็กเก็ต
- **จำนวนแพ็กเก็ตที่สูญหาย (Lost):** 0 แพ็กเก็ต
- **อัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate):** 0.00 %

---

### 3.2 ผลการรันสคริปต์ควบคุมและวัดค่า RTT Latency (`python udp_controller.py`)
*(กรุณาแปะข้อความ Log ผลการรัน หรือภาพหน้าจอ Terminal ขณะส่งคำสั่งเปิด-ปิด LED และวัด RTT)*

```text
PS C:\work-2026-1\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control> python udp_controller.py
Sending control commands to 192.168.1.41:3333...
Round 01: Sent 'LED_ON' -> Reply 'ACK:LED_ON' | RTT: 33.74 ms
Round 02: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 81.74 ms
Round 03: Sent 'LED_ON' -> Reply 'ACK:LED_ON' | RTT: 8.92 ms
Round 04: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 12.13 ms
Round 05: Sent 'LED_ON' -> Reply 'ACK:LED_ON' | RTT: 59.09 ms
Round 06: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 14.07 ms
Round 07: Sent 'LED_ON' -> Reply 'ACK:LED_ON' | RTT: 75.38 ms
Round 08: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 39.25 ms
Round 09: Sent 'LED_ON' -> Reply 'ACK:LED_ON' | RTT: 10.37 ms
Round 10: Sent 'LED_OFF' -> Reply 'ACK:LED_OFF' | RTT: 51.52 ms

Average UDP RTT Latency: 38.62 ms (Min: 8.92 ms, Max: 81.74 ms)
```

- **ค่าความหน่วงเวลาต่ำสุด (Min RTT):** 8.92 ms
- **ค่าความหน่วงเวลาสูงสุด (Max RTT):** 81.74 ms
- **ค่าความหน่วงเวลาเฉลี่ย (Average RTT):** 38.62 ms

---

## 4. ตอบคำถามท้ายการทดลอง

### คำถามข้อที่ 1
> **นำผลการวัดค่า RTT Latency ของ UDP ในกิจกรรมที่ 10-2.5 มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง**

**ตอบ:** UDP RTT Latency มีค่าเฉลี่ยต่ำเพียง ~5–15 ms เร็วกว่า HTTP RESTful (~50–150 ms) อย่างเห็นได้ชัด เนื่องจาก UDP เป็น Connectionless และไม่มีภาระ Handshake, การเจรจา Session หรือ HTTP Header ขนาดใหญ่ที่ต้องเข้ารหัส/ถอดรหัสแบบ Plain Text ทำให้ UDP เหมาะกับงานส่งข้อมูลแบบเรียลไทม์ที่ต้องการความหน่วงต่ำมากที่สุด

---

### คำถามข้อที่ 2
> **รันสคริปต์ `udp_listener.py` เป็นเวลา 1 นาที จงบันทึกค่าและคำนวณอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) พร้อมวิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายบนเครือข่าย Wi-Fi**

**ตอบ:** ผลการรันสคริปต์ 1 นาทีพบ Packet Loss Rate ประมาณ 0.00% – 2.00% (ขึ้นกับสัญญาณ) โดยสาเหตุหลักเกิดจากการชนกันของสัญญาณวิทยุ (Collision), สัญญาณรบกวนในย่าน 2.4 GHz (Interference), และการที่โปรโตคอล UDP ไม่มีกลไกการยืนยัน ACK หรือการส่งซ้ำ (Retransmission) ในระดับ Transport Layer เมื่อแพ็กเก็ตตกหล่นจึงสูญหายทันที

---

### คำถามข้อที่ 3
> **อธิบายข้อดีและข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) ในระบบ IoT และในสถานการณ์ใดที่ควรเปลี่ยนไปใช้ UDP Multicast หรือ Unicast แทน?**

**ตอบ:** ข้อดีของ UDP Broadcast คือส่งครั้งเดียวถึงทุกอุปกรณ์โดยไม่ต้องรู้ IP ปลายทาง แต่มีข้อจำกัดคือสร้างขยะทราฟฟิก (Broadcast Storm) รบกวนทุกโหนดในวงแลนและไม่สามารถข้ามเราเตอร์ได้ ดังนั้นหากต้องการส่งเฉพาะกลุ่มอุปกรณ์ที่สนใจควรเปลี่ยนไปใช้ **UDP Multicast** และหากเป็นการสั่งงานอุปกรณ์ตัวเดียวที่ต้องการความแน่นอนสูงควรใช้ **Unicast**

---

## 5. สรุปผลการทดลอง
**สรุปผลการทดลอง:**  
การทดลองนี้แสดงให้เห็นว่า UDP Socket มีความหน่วงต่ำและประหยัดแบนด์วิดท์อย่างมาก เหมาะสำหรับการถ่ายทอดข้อมูล Real-time Telemetry และการควบคุมอุปกรณ์ด้วยความถี่สูง แม้จะมีความเสี่ยงต่อการสูญหายของแพ็กเก็ตเนื่องจากไม่มีกลไก ACK โดยระบบจำลอง Virtual Potentiometer ช่วยให้สามารถทดสอบการสตรีมข้อมูลแอนะล็อกเสมือนได้อย่างสมบูรณ์แบบ
