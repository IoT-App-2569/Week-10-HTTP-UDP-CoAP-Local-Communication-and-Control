# รายงานผลการทดลอง: ใบงานที่ 10.3

**เรื่อง:** การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา

## 1. บันทึกผลการทดลอง

### 1.1 ข้อมูลระบบและ Resource Discovery

โปรเจกต์ CoAP Server เชื่อมต่อ Wi-Fi ด้วยข้อมูลเดียวกับแลปก่อนหน้า และเปิดให้บริการบน IP ของ ESP32 ที่ `10.14.195.186` ผ่าน UDP พอร์ต `5683`

| รายการ | ค่า |
| :--- | :--- |
| IP Address ของ ESP32 | `10.14.195.186` |
| CoAP Transport | UDP |
| CoAP Port | `5683` |
| LED | GPIO 2 |
| Potentiometer | GPIO 34 / ADC1 Channel 6 |
| Wi-Fi SSID | `Pornprom` |

คำสั่งที่ใช้ค้นหา Resource:

```text
GET coap://10.14.195.186/.well-known/core
```

ผลลัพธ์ Resource Discovery ในรูปแบบ CoRE Link Format:

```text
</sensor/pot>;rt="sensor",</actuator/led>;rt="actuator"
```

| Resource URI | Method | หน้าที่ | ผลลัพธ์ |
| :--- | :---: | :--- | :--- |
| `/.well-known/core` | GET | ค้นหารายการ Resource ของ ESP32 | พบ `/sensor/pot` และ `/actuator/led` |
| `/sensor/pot` | GET | อ่านค่า Potentiometer | อ่านค่าได้ `2385` |
| `/actuator/led` | PUT | ควบคุม LED ด้วยค่า `1` หรือ `0` | เปลี่ยนสถานะ LED สำเร็จ |

### 1.2 ผลการทดสอบ CoAP

การทดสอบใช้ Client เชื่อมต่อไปยัง ESP32 ที่ `10.14.195.186` โดยส่งคำขออ่านค่าเซนเซอร์และสั่งงาน LED

| การทดสอบ | Payload | Response Code | ผลลัพธ์ |
| :--- | :--- | :---: | :--- |
| GET `/sensor/pot` | - | `2.05 Content` | ได้ค่า Potentiometer `2385` |
| PUT `/actuator/led` เปิดไฟ | `1` | `2.04 Changed` | LED GPIO 2 ติด |
| PUT `/actuator/led` ปิดไฟ | `0` | `2.04 Changed` | LED GPIO 2 ดับ |

ตัวอย่างผลจาก `test_coap.py`:

```text
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>;rt="sensor",</actuator/led>;rt="actuator"

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 2385 (Code: 2.05 Content)

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
```

## 2. คำตอบคำถามท้ายบท

### ข้อ 1: CoRE Link Format (RFC 6690)

ผลการ Query `/.well-known/core` คือ:

```text
</sensor/pot>;rt="sensor",</actuator/led>;rt="actuator"
```

รูปแบบ CoRE Link Format แสดง Resource แต่ละรายการด้วย `<URI>` และคั่นรายการด้วยเครื่องหมายจุลภาค `,` ส่วน Attribute จะเขียนต่อท้ายด้วยเครื่องหมาย `;` เช่น `rt="sensor"` เพื่อระบุชนิดของ Resource

ในระบบนี้:

- `</sensor/pot>` คือ Resource สำหรับอ่านค่า Potentiometer
- `rt="sensor"` ระบุว่า Resource นี้เป็นกลุ่มเซนเซอร์
- `</actuator/led>` คือ Resource สำหรับควบคุม LED
- `rt="actuator"` ระบุว่า Resource นี้เป็นกลุ่มอุปกรณ์สั่งงาน

กลไกนี้ช่วยให้ Client ค้นหา URI และความสามารถของ ESP32 ได้โดยไม่ต้องกำหนดรายการ Resource ไว้ล่วงหน้า

### ข้อ 2: ความแตกต่างระหว่าง CON และ NON

| คุณสมบัติ | CON (Confirmable) | NON (Non-confirmable) |
| :--- | :--- | :--- |
| การยืนยันการรับ | ผู้รับตอบกลับด้วย ACK | ไม่มี ACK |
| การส่งซ้ำ | ส่งซ้ำเมื่อหมดเวลาและไม่ได้รับ ACK | ไม่มีการส่งซ้ำโดยอัตโนมัติ |
| ความน่าเชื่อถือ | สูงกว่า | ต่ำกว่า |
| ความหน่วงและ Overhead | สูงกว่าเมื่อมีการส่งซ้ำ | ต่ำกว่า |
| งานที่เหมาะสม | คำสั่งควบคุม LED | Telemetry ที่ยอมให้ข้อมูลบางส่วนสูญหาย |

เมื่อเครือข่ายมีสัญญาณรบกวน คำขอแบบ CON จะตรวจจับการไม่ได้รับ ACK และพยายามส่งซ้ำ ทำให้มีโอกาสได้รับคำสั่งครบถ้วนมากกว่า แต่ RTT และการใช้พลังงานจะเพิ่มขึ้น ส่วน NON ส่งเพียงครั้งเดียวจึงมีความหน่วงต่ำกว่า แต่แพ็กเก็ตอาจสูญหายโดยไม่มีการแจ้งเตือน

สำหรับการทดลองนี้ คำสั่ง PUT ที่ใช้ควบคุม LED ควรใช้แบบ CON เพื่อให้มั่นใจว่า ESP32 ได้รับคำสั่งและส่ง Response Code `2.04 Changed` กลับมา

### ข้อ 3: เหตุผลที่ CoAP เหมาะกับ Thread, Zigbee IP และ NB-IoT

CoAP เหมาะกับเครือข่าย IoT ที่มีข้อจำกัดด้านพลังงานและแบนด์วิดท์ด้วยเหตุผลดังนี้:

1. ทำงานบน UDP จึงไม่ต้องสร้าง TCP Connection หรือทำ TCP Handshake ก่อนส่งข้อมูล
2. ใช้ Header แบบไบนารีขนาดเล็ก โดย Header พื้นฐานมีขนาดประมาณ 4 ไบต์ จึงลด Overhead เมื่อเทียบกับ HTTP
3. รองรับรูปแบบ RESTful ผ่าน Method เช่น GET และ PUT ทำให้จัดโครงสร้าง Resource เช่น `/sensor/pot` และ `/actuator/led` ได้ชัดเจน
4. รองรับทั้ง CON และ NON เพื่อเลือกความน่าเชื่อถือหรือความเร็วให้เหมาะกับลักษณะข้อมูล
5. รองรับ Resource Discovery และ Observe ซึ่งเหมาะกับอุปกรณ์ IoT ที่ต้องค้นหาและติดตามข้อมูลจาก Resource
6. ใช้แบนด์วิดท์และพลังงานน้อยกว่า HTTP จึงเหมาะกับ Thread, Zigbee IP และ NB-IoT ที่มีข้อจำกัดของเฟรมข้อมูลหรือการใช้พลังงาน

อย่างไรก็ตาม HTTP ยังเหมาะกับ Web Browser และระบบองค์กรที่มีโครงสร้างพื้นฐาน HTTP อยู่แล้ว ส่วน CoAP เหมาะกับการสื่อสารระหว่างอุปกรณ์ IoT และสามารถใช้ Proxy แปลงการสื่อสารระหว่าง CoAP กับ HTTP ได้
