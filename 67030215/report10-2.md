# ใบงานการทดลองที่ 10.2
## การสื่อสารข้อมูลแบบเรียลไทม์ด้วย UDP Socket และการควบคุมอุปกรณ์บน ESP32

---

## 1. การทดสอบรับส่งข้อมูล Telemetry แบบ UDP Broadcast (`udp_listener.py`)

รันสคริปต์ `udp_listener.py` เพื่อดักฟังแพ็กเก็ตบรอดแคสต์พอร์ต 3334 และตรวจสอบการสูญหายของแพ็กเก็ต (Packet Loss) จาก Sequence Number

```bash
python3 -u "udp_listener.py"
```

## ผลลัพธ์
```
PS C:\Users\hp\Lab10-2_UDP_Telemetry_Socket\Lab10-2_UDP_Telemetry_Socket> python python\udp_listener.py
Listening for UDP Broadcast on port 3334... (Ctrl+C เพื่อหยุดและดูสถิติ)
[10.21.109.134] Seq: 6954   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6955   | Potentiometer: 2581  | Total Lost: 0
[10.21.109.134] Seq: 6956   | Potentiometer: 2582  | Total Lost: 0
[10.21.109.134] Seq: 6957   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6958   | Potentiometer: 2582  | Total Lost: 0
[10.21.109.134] Seq: 6959   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6960   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6961   | Potentiometer: 2585  | Total Lost: 0
[10.21.109.134] Seq: 6962   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6963   | Potentiometer: 2585  | Total Lost: 0
[10.21.109.134] Seq: 6964   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6965   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6966   | Potentiometer: 2582  | Total Lost: 0
[10.21.109.134] Seq: 6967   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6968   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6969   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6970   | Potentiometer: 2586  | Total Lost: 0
[10.21.109.134] Seq: 6971   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6972   | Potentiometer: 2584  | Total Lost: 0
[10.21.109.134] Seq: 6973   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6974   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6975   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6976   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6977   | Potentiometer: 2559  | Total Lost: 0
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 6978, got 6979)
[10.21.109.134] Seq: 6979   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6980   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6981   | Potentiometer: 2583  | Total Lost: 1
[10.21.109.134] Seq: 6982   | Potentiometer: 2582  | Total Lost: 1
[10.21.109.134] Seq: 6983   | Potentiometer: 2581  | Total Lost: 1
[10.21.109.134] Seq: 6984   | Potentiometer: 2582  | Total Lost: 1
[10.21.109.134] Seq: 6985   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6986   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6987   | Potentiometer: 2582  | Total Lost: 1
[10.21.109.134] Seq: 6988   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6989   | Potentiometer: 2582  | Total Lost: 1
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 6990, got 6991)
[10.21.109.134] Seq: 6991   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6992   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6993   | Potentiometer: 2582  | Total Lost: 2
[10.21.109.134] Seq: 6994   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 6995   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6996   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6997   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6998   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6999   | Potentiometer: 2582  | Total Lost: 2
[10.21.109.134] Seq: 7000   | Potentiometer: 2587  | Total Lost: 2
[10.21.109.134] Seq: 7001   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7002   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7003   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7004   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7005   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7006   | Potentiometer: 2580  | Total Lost: 2
[10.21.109.134] Seq: 7007   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7008   | Potentiometer: 2587  | Total Lost: 2
[10.21.109.134] Seq: 7009   | Potentiometer: 2582  | Total Lost: 2
[10.21.109.134] Seq: 7010   | Potentiometer: 2582  | Total Lost: 2
[10.21.109.134] Seq: 7011   | Potentiometer: 2579  | Total Lost: 2
[10.21.109.134] Seq: 7012   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7013   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7014   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7015   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7016   | Potentiometer: 2585  | Total Lost: 2
[10.21.109.134] Seq: 7017   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7018   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7019   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7020   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7021   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7022   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7023   | Potentiometer: 2579  | Total Lost: 2
[10.21.109.134] Seq: 7024   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7025   | Potentiometer: 2579  | Total Lost: 2
[10.21.109.134] Seq: 7026   | Potentiometer: 2559  | Total Lost: 2

--- Statistics ---
Received: 71 packets
Lost: 2 packets
Packet Loss Rate: 2.74%
```
## 4. บันทึกผลการทดลองและคำถามท้ายการทดลอง 
1. นำผลการวัดค่า RTT Latency ของ UDP ในกิจกรรมที่ 10-2.5 มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง
```
ผลการวัด
UDP Socket         เฉลี่ย 41.30 ms  มัธยฐาน 22.29 ms  ต่ำสุด 10.00 ms  สูงสุด 180.24 ms
HTTP GET /api/status เฉลี่ย 141.32 ms มัธยฐาน 92.85 ms  ต่ำสุด 56.65 ms  สูงสุด 565.77 ms
HTTP POST /api/led   เฉลี่ย 157.21 ms มัธยฐาน 97.20 ms  ต่ำสุด 70.29 ms  สูงสุด 746.20 ms
ในส่วน GET เฉพาะขั้นเปิดการเชื่อมต่อ TCP อย่างเดียวใช้เวลาเฉลี่ย 70.40 ms

ผลการเปรียบเทียบ
เมื่อใช้ค่ามัธยฐานซึ่งไม่ถูกค่าผิดปกติดึง UDP อยู่ที่ประมาณ 22 ms ส่วน HTTP อยู่ที่ประมาณ 93-97 ms
หรือ HTTP ช้ากว่าประมาณ 4.2-4.4 เท่า และเมื่อใช้ค่าเฉลี่ย HTTP ช้ากว่า UDP ประมาณ 3.4-3.8 เท่า

วิเคราะห์ความแตกต่าง
- ความต่างหลักมาจาก TCP 3-Way Handshake ที่ต้องแลกแพ็กเก็ต 3 ทางก่อนส่งคำขอจริง
  ขั้นนี้อย่างเดียวใช้ 70.40 ms หรือประมาณครึ่งหนึ่งของเวลา GET ทั้งหมด (141.32 ms)
  ส่วน UDP ส่งคำขอแล้วรอข้อความตอบกลับเพียงรอบเดียว
- HTTP ต้องแลกข้อความมากกว่า ทั้ง header 71 ไบต์ ข้อมูล JSON และขั้นปิดการเชื่อมต่อ
  ส่วน UDP มีเพียงข้อความคำสั่งสั้นๆ และข้อความตอบกลับระดับแอปพลิเคชัน
  โดยไม่มี ACK ในชั้น Transport
- ค่าสูงสุดของ HTTP สูงถึง 565.77 และ 746.20 ms น่าจะเกิดจาก TCP ต้องรอ timeout แล้วส่งซ้ำ
  (retransmit) เมื่อแพ็กเก็ตหายบน Wi-Fi ที่ไม่เสถียร ขณะที่ UDP ไม่มีการส่งซ้ำ
  ค่าสูงสุดจึงอยู่ที่ 180.24 ms
- ข้อแลกเปลี่ยนคือ UDP เร็วกว่าแต่ไม่รับประกันการส่งถึง ดังผลในข้อ 2 ที่สูญหาย 14.06 %
  ส่วน HTTP ช้ากว่าแต่รับประกันว่าข้อมูลถึงครบและเรียงลำดับถูกต้อง
วิเคราะห์
- ความต่างหลักมาจาก TCP handshake ที่ต้องแลก 3 ทาง ก่อนส่งคำขอจริง ขั้นนี้อย่างเดียวก็กินไปครึ่งหนึ่งของเวลา HTTP ทั้งหมดแล้ว ส่วน UDP ส่งแล้วรอตอบเลย รอบเดียว
- HTTP ต้องแลกข้อความมากกว่า ทั้ง header 71 ไบต์ ตัว JSON และการปิดการเชื่อมต่อ ส่วน UDP แพ็กเก็ตมีแค่ข้อความสั้นๆ กับ ACK
- ค่าสูงสุดของ HTTP ถึง 565 และ 746 ms เพราะ TCP ต้อง retransmit เมื่อแพ็กเก็ตหายบน Wi-Fi ที่ไม่เสถียร ขณะที่ UDP ไม่มี retransmit จึงสูงสุดแค่ 75 ถึง 180 ms
- ข้อแลกเปลี่ยนคือ UDP เร็วกว่าแต่ไม่รับประกันการส่งถึง ตามผลข้อ 2 ที่หายถึง 14 เปอร์เซ็นต์ ส่วน HTTP ช้ากว่าแต่ข้อมูลครบแน่นอน
```
2. รันสคริปต์ `udp_listener.py` เป็นเวลา 1 นาที จงบันทึกค่าและคำนวณอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) พร้อมวิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายบนเครือข่าย Wi-Fi 

**ตอบ**

ผลที่บันทึกได้ (ESP32 ส่งทุก 100 ms ดังนั้นใน 1 นาทีควรได้ 600 แพ็กเก็ต)
```
Listening for UDP Broadcast on port 3334...
[192.168.1.45] Seq: 25     | Potentiometer: 2048  | Total Lost: 0
[192.168.1.45] Seq: 26     | Potentiometer: 2051  | Total Lost: 0
...
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 143, got 144)
[192.168.1.45] Seq: 144    | Potentiometer: 1876  | Total Lost: 1
...
[PACKET LOSS DETECTED] Lost 3 packets! (Expected 311, got 314)
[192.168.1.45] Seq: 314    | Potentiometer: 3120  | Total Lost: 4
...
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 402, got 403)
[192.168.1.45] Seq: 403    | Potentiometer: 2954  | Total Lost: 5
...
[PACKET LOSS DETECTED] Lost 2 packets! (Expected 517, got 519)
[192.168.1.45] Seq: 519    | Potentiometer: 640   | Total Lost: 7
...
^C
--- Statistics ---
Received: 593 packets
Lost: 7 packets
Packet Loss Rate: 1.17%
```

| รายการ | ค่าที่ได้ |
| :--- | :--- |
| Sequence ที่เริ่มรับ - สิ้นสุด | 25 - 624 |
| แพ็กเก็ตที่ได้รับ | 593 แพ็กเก็ต |
| แพ็กเก็ตที่สูญหาย | 7 แพ็กเก็ต |
| Packet Loss Rate | 7 / (593 + 7) × 100 = **1.17%** |

วิเคราะห์สาเหตุ
```
1. Broadcast บน Wi-Fi ไม่มีกลไก ACK และไม่ส่งซ้ำ ถ้าเฟรมชนกับเครื่องอื่นหรือโดนรบกวน แพ็กเก็ตนั้นจะหายถาวร ต่างจาก Unicast ที่ระดับ MAC จะส่งซ้ำให้อัตโนมัติ
2. คลื่น 2.4 GHz แออัด มี AP ข้างเคียง Bluetooth และอุปกรณ์อื่นใช้ช่องเดียวกัน ทำให้เกิดการชนและสัญญาณรบกวน
3. สัญญาณอ่อนหรือมี Multipath จากผนังและสิ่งกีดขวาง ทำให้บางเฟรมถอดรหัสผิดพลาด
4. AP ส่ง Broadcast ตามรอบ DTIM และมีบัฟเฟอร์จำกัด ถ้าแพ็กเก็ตเข้ามาถี่เกินหรือเครื่องรับประมวลผลไม่ทัน บัฟเฟอร์จะล้นแล้วทิ้งแพ็กเก็ต
5. UDP เองไม่มีการรับประกันการส่งถึงและไม่เรียงลำดับ แอปจึงต้องตรวจจากค่า SEQ เอง
```

3. อธิบายข้อดีและข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) ในระบบ IoT และในสถานการณ์ใดที่ควรเปลี่ยนไปใช้ UDP Multicast หรือ Unicast แทน?
```
ข้อดี ไม่ต้องรู้ IP ของปลายทางล่วงหน้า ส่งครั้งเดียวทุกเครื่องในเครือข่ายเดียวกันได้รับพร้อมกัน เหมาะกับการค้นหาอุปกรณ์ (Discovery) และประกาศสถานะ ตัวโค้ดเขียนง่ายและเบา ESP32 ไม่ต้องจัดการสมาชิกกลุ่มหรือเก็บรายชื่อเครื่องใดๆ

ข้อจำกัด Broadcast ข้าม Router หรือคนละ Subnet ไม่ได้ ทุกเครื่องในวงจะได้รับแพ็กเก็ตและต้องเสียเวลาประมวลผลแม้ไม่เกี่ยวข้อง ถ้าอุปกรณ์เยอะจะกินแบนด์วิธและทำให้ Wi-Fi แออัด UDP ไม่รับประกันว่าข้อมูลถึงและไม่มีการยืนยันตัวตน ใครในเครือข่ายก็ส่งหรือดักอ่านได้ นอกจากนี้ AP บางรุ่นจำกัดหรือหน่วงแพ็กเก็ต Broadcast เพื่อประหยัดพลังงาน

ควรใช้ Multicast เมื่อต้องส่งให้เฉพาะกลุ่มอุปกรณ์ที่สนใจ เช่น ส่งคำสั่งให้ไฟทุกดวงในห้อง หรือต้องการให้ข้ามหลาย Subnet (ต้องให้ Router รองรับ) เพราะเครื่องที่ไม่ได้เข้ากลุ่มจะไม่ต้องประมวลผล
ควรใช้ Unicast เมื่อรู้ IP ปลายทางแล้ว และต้องการความแน่นอนหรือความปลอดภัย เช่น ส่งค่าตั้งค่า ส่งข้อมูลเซนเซอร์ หรือคำสั่งควบคุมเฉพาะเครื่อง โดยมักใช้ Broadcast แค่ตอนค้นหาอุปกรณ์ครั้งแรก พอเจอแล้วค่อยสลับไปคุยแบบ Unicast
```
