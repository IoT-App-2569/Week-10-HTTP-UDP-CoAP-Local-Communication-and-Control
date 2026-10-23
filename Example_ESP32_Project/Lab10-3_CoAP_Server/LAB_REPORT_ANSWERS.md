# รายงานผลการทดลองและเฉลยคำถามท้ายการทดลอง: Lab 10.3
## การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา

---

### 1. โครงสร้างโปรเจกต์ (Project Structure)
```
Lab10-3_CoAP_Server_and_Lightweight_IoT_Control/
├── CMakeLists.txt
├── sdkconfig.defaults
├── test_coap.py
└── main/
    ├── CMakeLists.txt
    ├── idf_component.yml
    └── Lab10-3_CoAP_Server.c
```

### 2. แผนภาพและพอร์ตการทำงาน (CoAP Architecture)
* **โพรโทคอล:** CoAP (Constrained Application Protocol - RFC 7252)
* **ทรานสปอร์ตเลเยอร์:** UDP พอร์ต **5683** (CoAP Default Port)
* **รายการทรัพยากร (Endpoints / Resources):**
  * `GET /.well-known/core` : Resource Directory (RFC 6690 CoRE Link Format)
  * `GET /sensor/pot` : อ่านค่าตัวต้านทานปรับค่าได้ (Potentiometer Raw ADC Value)
  * `PUT /actuator/led` : สั่งการเปิด-ปิดหลอดไฟ LED (Payload: `1` หรือ `0`)

---

### 3. ผลการทดสอบผ่านสคริปต์ `test_coap.py` (Verification Logs)
```text
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>;rt="sensor";title="Analog Potentiometer",</actuator/led>;rt="actuator";title="LED Control"

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 2045 (Code: 2.05 Content)

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
```

---

### 4. บันทึกผลการทดลองและเฉลยคำถามท้ายบท (Lab Questions & Answers)

#### ข้อที่ 1: นำผลการ Query `/.well-known/core` มาแสดงในรายงาน พร้อมอธิบายรูปแบบ CoRE Link Format (RFC 6690) ว่าแสดงข้อมูลทรัพยากรอย่างไร

**ผลลัพธ์ที่ได้จากการ Query:**
```text
</sensor/pot>;rt="sensor";title="Analog Potentiometer",</actuator/led>;rt="actuator";title="LED Control"
```

**คำอธิบายโครงสร้าง CoRE Link Format (RFC 6690):**
รูปแบบ **CoRE Link Format** ถูกกำหนดขึ้นเพื่อให้โฮสต์หรือเซิร์ฟเวอร์ขนาดเล็กสามารถประกาศรายการทรัพยากร (Resources) ที่มีอยู่ให้ไคลเอนต์ค้นหาอัตโนมัติ (Discovery) ในรูปแบบข้อความกะทัดรัด โดยมีไวยากรณ์หลักดังนี้:
1. **URI Path ในเครื่องหมายวงเล็บแหลม (`< ... >`):**
   * ระบุเส้นทางของ Endpoint เช่น `</sensor/pot>` หรือ `</actuator/led>`
2. **การคั่นทรัพยากรแต่ละตัวด้วยเครื่องหมายจุลภาค (`,`):**
   * ใช้แบ่งแยกระหว่างทรัพยากรตัวแรกและทรัพยากรตัวถัดไป
3. **แอ็ตทริบิวต์กำกับ (Link Target Attributes) ด้วยเครื่องหมายอัฒภาค (`;`):**
   * **`rt="sensor"` (Resource Type):** กำหนดประเภทเชิงความหมาย (Semantic Description) ของทรัพยากร เพื่อให้โปรแกรมทราบว่าเป็นเซนเซอร์หรือแอคทูเอเตอร์
   * **`title="..."` (Title):** สตริงคำอธิบายที่มนุษย์อ่านเข้าใจได้ (Human-readable string)
   * **`ct=...` (Content-Format - เสริม):** ระบุประเภทของข้อมูลเพย์โหลด เช่น `ct=0` สำหรับ `text/plain`, `ct=50` สำหรับ `application/json`, `ct=60` สำหรับ `application/cbor`
   * **`obs` (Observable - เสริม):** แสดงว่าทรัพยากรนี้รองรับการบอกรับข้อมูลแบบ Pub/Sub ตามมาตรฐาน RFC 7641

---

#### ข้อที่ 2: อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ

| คุณสมบัติ | Confirmable Message (CON) | Non-confirmable Message (NON) |
| :--- | :--- | :--- |
| **การตอบรับ (ACK)** | ผู้รับ **ต้อง** ส่งแพ็กเก็ต ACK ตอบกลับโดยใช้ Message ID เดียวกัน | ผู้รับ **ไม่ต้อง** ส่ง ACK ยืนยัน |
| **การส่งซ้ำ (Retransmission)** | มีการส่งซ้ำแบบ **Exponential Backoff** (ปกติเริ่มที่ 2s จนถึง 4 ครั้ง) | **ไม่มีการส่งซ้ำ** (Fire-and-forget) |
| **ความน่าเชื่อถือ (Reliability)** | **สูง** (เชื่อถือได้เทียบเท่า TCP แต่ใช้ UDP) | **ต่ำ** (เสี่ยงต่อข้อมูลสูญหาย) |
| **พฤติกรรมเมื่อสัญญาณรบกวน** | หากแพ็กเก็ตตกหล่น สแตก CoAP จะส่งซ้ำจนกว่าจะได้รับ ACK คำสั่งไม่สูญหาย | หากแพ็กเก็ตตกหล่น ข้อมูลจะหายไปทันทีโดยไม่มีการแจ้งเตือน |
| **การใช้งานที่เหมาะสม** | คำสั่งควบคุมวิกฤต เช่น สั่งเปิด-ปิดวาล์วแก๊ส, ควบคุมรีเลย์ | ข้อมูล Telemetry ที่ส่งต่อเนื่องถี่ๆ เช่น อุณหภูมิส่งทุก 1 วินาที |

**บทสรุป:** การออกแบบ CoAP ช่วยให้ผู้พัฒนาระบบ IoT สามารถเลือกได้ในระดับแพ็กเก็ต (Per-message selection) ว่าต้องการความเร็วสูงสุด (NON) หรือความปลอดภัยแน่นอนสูงสุด (CON) ภายใต้โปรโตคอลขนาดเล็กตัวเดียวกัน

---

#### ข้อที่ 3: ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?

**เหตุผลสำคัญทางวิศวกรรมเครือข่าย IoT:**

1. **ขนาดของส่วนหัวแพ็กเก็ตที่เล็กมาก (Extremely Low Header Overhead):**
   * ส่วนหัวพื้นฐานของ CoAP มีขนาดคงที่เพียง **4 ไบต์** (2-bit Ver, 2-bit Type, 4-bit Token Length, 8-bit Code, 16-bit Message ID) 
   * ในขณะที่ส่วนหัว HTTP มีขนาดมากกว่า **100–250 ไบต์**
   * บนเครือข่ายเช่น **Thread (IEEE 802.15.4 / 6LoWPAN)** ซึ่งมีขนาด MTU ของ Physical Frame เพียง **127 ไบต์** หากใช้ HTTP จะเกิดการแตกแพ็กเก็ต (6LoWPAN Fragmentation) หลายทอด ทำให้สิ้นเปลืองพลังงานและเกิดความล้มเหลวในการส่งสูง แต่ CoAP สามารถบรรจุลงในเฟรมเดียวได้สบาย
2. **ทำงานบนสแตก UDP แทน TCP (Battery-Friendly & Duty Cycling):**
   * เครือข่าย **NB-IoT** และเครือข่ายพลังงานต่ำ อุปกรณ์เซนเซอร์ส่วนใหญ่ทำงานด้วยแบตเตอรี่ และต้องเข้าสู่โหมดหลับลึก (Deep Sleep / eDRX / PSM) 
   * หากใช้ TCP (HTTP) อุปกรณ์ต้องตื่นมาแลกเปลี่ยนแพ็กเก็ต Handshake (SYN, SYN-ACK, ACK) และรักษาสถานะการเชื่อมต่อ ทำให้เปลืองพลังงานวิทยุ (Radio TX/RX Energy)
   * การใช้ CoAP บน UDP ทำให้อุปกรณ์สามารถตื่น ส่ง 1 แพ็กเก็ต และกลับเข้า Deep Sleep ได้ภายในหลักมิลลิวินาที ประหยัดพลังงานแบตเตอรี่ได้หลายเท่าตัว
3. **การเข้ารหัสแบบไบนารีและฟังก์ชัน Observe (RFC 7641):**
   * CoAP ใช้การระบุออปชันและรหัสสถานะเป็นเลขฐานสอง (Binary Encoding) แทนข้อความสตริง ASCII
   * มีกลไก **Observe** ในตัว ทำให้เซิร์ฟเวอร์สามารถ Push ข้อมูลแจ้งเตือนไปยังไคลเอนต์ได้ทันทีเมื่อเกิดเหตุการณ์ โดยไม่ต้องให้ไคลเอนต์คอยส่งคำขอ HTTP Polling ซ้ำๆ ซึ่งผลาญแบนด์วิดท์ของเครือข่ายแคบ
4. **ความปลอดภัยที่เบากว่าด้วย DTLS:**
   * CoAP ปกป้องความปลอดภัยด้วย **DTLS (Datagram TLS)** รองรับ Pre-Shared Key (PSK) และ ECC Cipher Suites ซึ่งใช้ทรัพยากรคำนวณและหน่วยความจำน้อยกว่าสแตก TLS บน TCP ของ HTTP
