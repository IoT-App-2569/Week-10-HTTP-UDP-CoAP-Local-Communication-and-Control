# รายงานผลการทดลอง: ใบงานที่ 10.3

**เรื่อง:** การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา

---

## 1. บันทึกผลการทดลอง

### 1.1 Resource Discovery

```text
GET coap://<ESP32_IP>/.well-known/core

(วางผลการ Query จริงตรงนี้)
```

| Resource URI | Method | หน้าที่ | ผลลัพธ์ |
| :--- | :---: | :--- | :--- |
| `/.well-known/core` | GET | ค้นหา Resource |  |
| `/sensor/pot` | GET | อ่าน Potentiometer |  |
| `/actuator/led` | PUT | ควบคุม LED |  |

### 1.2 ผลการทดสอบ CoAP

| การทดสอบ | Payload | Response Code | ผลลัพธ์ |
| :--- | :--- | :---: | :--- |
| GET `/sensor/pot` | - |  |  |
| PUT `/actuator/led` เปิดไฟ | `1` |  |  |
| PUT `/actuator/led` ปิดไฟ | `0` |  |  |

## 2. คำตอบคำถามท้ายบท

### ข้อ 1: CoRE Link Format (RFC 6690)

ผล Query `/.well-known/core` ต้องแสดงรายการ Resource ในรูปแบบ Link เช่น `</sensor/pot>` และ `</actuator/led>` โดยสามารถระบุ Attribute เช่น `rt` (Resource Type), `if` (Interface Description) และ `ct` (Content Format)

รูปแบบนี้ทำให้ Client ค้นหา URI และความสามารถของอุปกรณ์ได้โดยไม่ต้องกำหนดรายการ Resource ไว้ล่วงหน้า

### ข้อ 2: CON และ NON

| คุณสมบัติ | CON | NON |
| :--- | :--- | :--- |
| การยืนยัน | มี ACK | ไม่มี ACK |
| การส่งซ้ำ | ทำได้เมื่อ Timeout | ไม่มีโดยอัตโนมัติ |
| ความน่าเชื่อถือ | สูงกว่า | ต่ำกว่า |
| ความหน่วงและ Overhead | สูงกว่า | ต่ำกว่า |
| เหมาะกับ | คำสั่งสำคัญ | Telemetry |

CON เหมาะกับคำสั่งที่ต้องทราบว่าปลายทางได้รับข้อมูล ส่วน NON เหมาะกับข้อมูลที่ส่งซ้ำได้หรือยอมรับการสูญหายบางส่วน เมื่อมีสัญญาณรบกวน CON จะตรวจจับ Timeout และส่งซ้ำได้ แต่มีความหน่วงเพิ่มขึ้น

### ข้อ 3: เหตุผลที่ CoAP เหมาะกับ Thread, Zigbee IP และ NB-IoT

CoAP ทำงานบน UDP จึงไม่ต้องสร้าง TCP Connection มี Header แบบ Binary ขนาดเล็ก และใช้พลังงานกับแบนด์วิดท์ต่ำกว่า HTTP นอกจากนี้ยังคงรูปแบบ RESTful, รองรับ CON/NON, Resource Discovery และ Observe

คุณสมบัติเหล่านี้เหมาะกับเครือข่ายที่มีอัตรารับส่งข้อมูลต่ำ พลังงานจำกัด และมีอุปกรณ์จำนวนมาก เช่น Thread, Zigbee IP และ NB-IoT อย่างไรก็ตาม HTTP ยังเหมาะกว่าสำหรับ Web Browser และระบบองค์กรที่มีโครงสร้างพื้นฐาน HTTP อยู่แล้ว
