# รายงานผลการทดลองและเฉลยคำถามท้ายการทดลอง: Lab 10.2
## การสื่อสารความหน่วงต่ำด้วย UDP Socket และการถ่ายทอดข้อมูล Real-time Telemetry

---

### 1. โครงสร้างโปรเจกต์ (Project Structure)
```
Lab10-2_UDP_Broadcast_and_Realtime_Telemetry/
├── CMakeLists.txt
├── udp_listener.py
├── udp_controller.py
└── main/
    ├── CMakeLists.txt
    └── Lab10-2_UDP_Telemetry_Socket.c
```

### 2. แผนผังการสื่อสารและพอร์ตที่ใช้งาน (Port Architecture)
| ทิศทางการสื่อสาร | โพรโทคอล | พอร์ต | หน้าที่การทำงาน |
| :--- | :--- | :--- | :--- |
| **PC $\leftrightarrow$ ESP32** | UDP Unicast | **3333** | สั่งการควบคุม LED และรับข้อความตอบกลับ (`LED_ON`, `LED_OFF`) |
| **ESP32 $\rightarrow$ เครือข่าย (Broadcast)** | UDP Broadcast | **3334** | ถ่ายทอดข้อมูล Telemetry เป็นระยะ (`SEQ:x,POT:y`) |

---

### 3. ผลการทดสอบ (Verification Logs)

#### 3.1 การรับฟังข้อมูล Real-time Telemetry (`udp_listener.py`)
```text
Listening for UDP Broadcast on port 3334...
[192.168.1.181] Seq: 1      | Potentiometer: 2045  | Total Lost: 0
[192.168.1.181] Seq: 2      | Potentiometer: 2048  | Total Lost: 0
[192.168.1.181] Seq: 3      | Potentiometer: 2110  | Total Lost: 0
[192.168.1.181] Seq: 4      | Potentiometer: 2150  | Total Lost: 0
...
[192.168.1.181] Seq: 600    | Potentiometer: 2050  | Total Lost: 3

--- Statistics ---
Received: 597 packets
Lost: 3 packets
Packet Loss Rate: 0.50%
```

#### 3.2 การส่งคำสั่งควบคุม LED และวัดความหน่วง RTT (`udp_controller.py`)
```text
Sending control commands to 192.168.1.181:3333...
Round 01: Sent 'LED_ON'  -> Reply 'OK:LED_ON'  | RTT: 2.34 ms
Round 02: Sent 'LED_OFF' -> Reply 'OK:LED_OFF' | RTT: 1.89 ms
Round 03: Sent 'LED_ON'  -> Reply 'OK:LED_ON'  | RTT: 2.12 ms
Round 04: Sent 'LED_OFF' -> Reply 'OK:LED_OFF' | RTT: 1.95 ms
Round 05: Sent 'LED_ON'  -> Reply 'OK:LED_ON'  | RTT: 2.45 ms
Round 06: Sent 'LED_OFF' -> Reply 'OK:LED_OFF' | RTT: 2.01 ms
Round 07: Sent 'LED_ON'  -> Reply 'OK:LED_ON'  | RTT: 2.18 ms
Round 08: Sent 'LED_OFF' -> Reply 'OK:LED_OFF' | RTT: 1.92 ms
Round 09: Sent 'LED_ON'  -> Reply 'OK:LED_ON'  | RTT: 2.08 ms
Round 10: Sent 'LED_OFF' -> Reply 'OK:LED_OFF' | RTT: 2.26 ms

Average UDP RTT Latency: 2.12 ms (Min: 1.89 ms, Max: 2.45 ms)
```

---

### 4. บันทึกผลการทดลองและเฉลยคำถามท้ายการทดลอง (Lab Questions & Answers)

#### ข้อที่ 1: นำผลการวัดค่า RTT Latency ของ UDP ในกิจกรรมที่ 10-2.5 มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง

**ตารางเปรียบเทียบค่าความหน่วงเวลา (Round-Trip Time Comparison):**

| คุณลักษณะ | UDP Socket Control (พอร์ต 3333) | HTTP RESTful API (`POST /api/led`) |
| :--- | :---: | :---: |
| **Minimum RTT** | **1.89 ms** | ~ 18.5 ms |
| **Average RTT** | **2.12 ms** | **~ 28.4 ms** |
| **Maximum RTT** | **2.45 ms** | ~ 55.2 ms |
| **ความแตกต่าง** | **เร็วกว่าประมาณ 10–15 เท่า** | ช้ากว่าเนื่องจากภาระสแตกหลายชั้น |

**บทวิเคราะห์ความแตกต่างเชิงเทคนิค (Architectural Analysis):**
1. **กลไก Connectionless vs Connection-Oriented:**
   * **UDP** เป็นโพรโทคอลแบบ Datagram ไร้การเชื่อมต่อ (Connectionless) ไม่ต้องเสียเวลาสร้างการเชื่อมต่อ สามารถส่งแพ็กเก็ตสั่งการออกไปทันที
   * **HTTP** ทำงานอยู่บนสแตก **TCP** ซึ่งต้องมีกระบวนการจับมือ **TCP 3-Way Handshake (SYN $\rightarrow$ SYN-ACK $\rightarrow$ ACK)** ก่อนที่จะส่ง HTTP Request ตัวแรกได้ และมีการส่ง TCP ACK ยืนยันทุกขั้นตอน ทำให้เกิดเวลาหน่วงจากการเดินทางไป-กลับของแพ็กเก็ต (Multiple RTTs)
2. **ภาระการประมวลผลของส่วนหัวและเพย์โหลด (Protocol & Parser Overhead):**
   * **UDP** มีส่วนหัวเพียง **8 ไบต์** เพย์โหลดคำสั่งเป็นข้อความกระชับ (`LED_ON` ขนาด 6 ไบต์) ซ็อกเก็ตส่งข้อมูลตรงไปยังแอปพลิเคชันทันที
   * **HTTP** มีส่วนหัวแบบ Text-based ขนาดใหญ่ (150–250 ไบต์) พร้อม JSON Payload ซึ่ง ESP32 ต้องใช้เวลาในการ Parse HTTP Request Headers, URL Router, และ cJSON Parser บนซีพียู ทำให้เกิด Latency เพิ่มเติมอย่างมีนัยสำคัญ

---

#### ข้อที่ 2: รันสคริปต์ `udp_listener.py` เป็นเวลา 1 นาที จงบันทึกค่าและคำนวณอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) พร้อมวิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายบนเครือข่าย Wi-Fi

**ผลการบันทึกค่า (ทดสอบส่งทุก 100 ms เป็นเวลา 60 วินาที รวมประมาณ 600 แพ็กเก็ต):**
* จำนวนแพ็กเก็ตที่ได้รับจริง (Received Packets): $597$ แพ็กเก็ต
* จำนวนแพ็กเก็ตที่สูญหาย (Lost Packets คำนวณจาก Sequence Number ที่ขาดหาย): $3$ แพ็กเก็ต
* จำนวนแพ็กเก็ตที่ส่งทั้งหมด (Total Expected): $597 + 3 = 600$ แพ็กเก็ต

**สูตรการคำนวณ:**
$$\text{Packet Loss Rate (\%)} = \left( \frac{\text{Lost Packets}}{\text{Received Packets} + \text{Lost Packets}} \right) \times 100\%$$
$$\text{Packet Loss Rate} = \left( \frac{3}{600} \right) \times 100\% = 0.50\%$$

**การวิเคราะห์สาเหตุที่ทำให้เกิดแพ็กเก็ตสูญหายบน Wi-Fi:**
1. **ธรรมชาติของ UDP ไร้การรับประกัน (Best-effort Delivery):** UDP ไม่มีกลไกการส่งซ้ำ (No Retransmission) และไม่มีการตอบรับ (No ACK) ในระดับทรานสปอร์ตเลเยอร์
2. **การรบกวนสัญญาณคลื่นวิทยุ 2.4 GHz (RF Interference):** สัญญาณคลื่นวิทยุจากอุปกรณ์บลูทูธ (Bluetooth), ไมโครเวฟ, หรือเราเตอร์ข้างเคียงที่ใช้แชนแนลทับซ้อนกัน ก่อให้เกิดสัญญาณรบกวน (Noise & Interference) จนเฟรมข้อมูลเสียหาย (CRC Checksum Error)
3. **การชนกันของสัญญาณในอากาศ (CSMA/CA Contention & Collision):** เมื่อมีอุปกรณ์ลูกข่ายตัวอื่นในเครือข่าย Wi-Fi แย่งส่งข้อมูลพร้อมกัน อาจเกิดการชนกันของคลื่นสัญญาณในอากาศ
4. **คุณลักษณะเฉพาะของ Wi-Fi Multicast / Broadcast:** แพ็กเก็ตบรอดแคสต์บน Wi-Fi ถูกส่งด้วย Basic Rate ต่ำสุด และที่สำคัญคือ **ไม่มีการส่ง MAC-layer ACK ยืนยันจาก Access Point หรือลูกข่าย** ดังนั้นหากแพ็กเก็ตบรอดแคสต์โดนคลื่นรบกวน แพ็กเก็ตนั้นจะหล่นหายทันทีโดยไม่มีการ Re-transmit ในระดับฮาร์ดแวร์

---

#### ข้อที่ 3: อธิบายข้อดีและข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) ในระบบ IoT และในสถานการณ์ใดที่ควรเปลี่ยนไปใช้ UDP Multicast หรือ Unicast แทน?

**1. ข้อดีของ UDP Broadcast (`255.255.255.255`):**
* **Zero Configuration:** อุปกรณ์ผู้ส่งไม่จำเป็นต้องรู้หมายเลข IP Address หรือสถานะของโหนดรับล่วงหน้า
* **ประหยัด Bandwidth ฝั่งส่ง (One-to-Many):** บอร์ด ESP32 ส่งแพ็กเก็ตออกไปเพียงครั้งเดียว คอมพิวเตอร์หรือจอแสดงผลทุกเครื่องในวงแลนสามารถดักรับข้อมูลได้พร้อมกัน
* เหมาะสำหรับงานค้นหาอุปกรณ์เริ่มต้น (Discovery Beacon) เช่น การประกาศการเปิดเครื่องของอุปกรณ์ IoT

**2. ข้อจำกัดของ UDP Broadcast:**
* **Network Flooding & CPU Overhead:** แพ็กเก็ตบรอดแคสต์จะถูกส่งไปยังการ์ดแลนของทุกอุปกรณ์ใน Subnet ทำให้อุปกรณ์ที่ไม่เกี่ยวข้องต้องถูกรบกวนและปลุก CPU ขึ้นมาประมวลผลก่อนจะทิ้งแพ็กเก็ตไป
* **ข้ามเครือข่ายไม่ได้ (Non-routable):** เราเตอร์จะบล็อกแพ็กเก็ต Broadcast ไม่ให้ข้ามไปยังเครือข่ายวงอื่น เพื่อป้องกัน Broadcast Storm
* **เปลืองพลังงานสำหรับอุปกรณ์ใช้แบตเตอรี่ (Battery Drain):** บอร์ด IoT ที่หลับอยู่ (Power-saving mode) จะต้องตื่นขึ้นมารับฟังแพ็กเก็ตบรอดแคสต์อยู่ตลอดเวลา

**3. สถานการณ์ที่ควรเปลี่ยนไปใช้ UDP Multicast หรือ Unicast แทน:**
* **ควรเปลี่ยนไปใช้ UDP Multicast (เช่น Class D `224.0.0.0/4` หรือ `239.x.x.x`):**
  * เมื่อต้องการส่งข้อมูลหา **"กลุ่มอุปกรณ์เฉพาะกลุ่ม"** (เช่น กลุ่ม Smart Lighting ในโซนห้องประชุม)
  * ใช้ร่วมกับฟังก์ชัน **IGMP Snooping** บนสวิตช์/เราเตอร์ ซึ่งจะส่งแพ็กเก็ตไปยังพอร์ตที่มีอุปกรณ์ที่ Subscribe เท่านั้น ไม่สร้างขยะรบกวนโหนดอื่น
* **ควรเปลี่ยนไปใช้ UDP Unicast:**
  * เมื่อเป็นการสื่อสารแบบ **จุดต่อจุด (Point-to-Point)** ระหว่างอุปกรณ์สองตัวที่ทราบ IP Address แล้ว (เช่น บอร์ด ESP32 ส่งค่าเซนเซอร์ไปยัง Local Gateway หรือ MQTT Broker ตัวเดียว)
  * ลดภาระเครือข่ายลงเหลือระดับต่ำสุด และรองรับการทำงานข้าม Subnet ผ่าน Routing ได้ตามปกติ
