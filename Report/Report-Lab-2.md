# รายงานผลการทดลอง: ใบงานที่ 10.2

**เรื่อง:** การสื่อสารความหน่วงต่ำด้วย UDP Socket และการถ่ายทอดข้อมูล Real-time Telemetry

---

## 1. บันทึกผลการทดลอง

### 1.1 ผลการวัด RTT

| รายการ | ผลการทดลอง |
| :--- | :--- |
| IP Address ของ ESP32 |  |
| จำนวนรอบ | 10 รอบ |
| Average RTT |  ms |
| Minimum RTT |  ms |
| Maximum RTT |  ms |

**ผลจาก `udp_controller.py`:**

```text
(วางผลการรันจริงตรงนี้)
```

### 1.2 ผลการรับ Telemetry และ Packet Loss

| รายการ | ค่า |
| :--- | ---: |
| Received |  |
| Lost |  |
| Packets ที่ควรได้รับ |  |
| Packet Loss Rate |  % |

```text
Packet Loss Rate (%) = Lost / (Received + Lost) × 100
```

## 2. คำตอบคำถามท้ายการทดลอง

### ข้อ 1: เปรียบเทียบ UDP กับ HTTP RESTful

UDP มี RTT ต่ำกว่า HTTP RESTful โดยทั่วไป เพราะเป็น Connectionless ใช้ Header ขนาดเล็ก และไม่มี TCP Handshake หรือ HTTP Request/Response Header จำนวนมาก เหมาะกับ Telemetry แบบ Real-time

HTTP RESTful มี Overhead และความหน่วงสูงกว่า แต่มีโครงสร้างชัดเจน ใช้งานง่าย และอาศัย TCP ซึ่งช่วยรับประกันการส่งข้อมูล เหมาะกับข้อมูลที่ต้องการความถูกต้องและการตอบกลับที่เชื่อถือได้

**ผลที่วัดได้จริง:** UDP `_____ ms`, HTTP `_____ ms`

### ข้อ 2: วิเคราะห์ Packet Loss

Packet Loss อาจเกิดจากสัญญาณรบกวน ระยะห่างจาก Access Point สิ่งกีดขวาง จำนวนผู้ใช้งานเครือข่าย ภาระของ ESP32 หรือ Firewall ที่กรอง UDP Broadcast

UDP ไม่มีการยืนยันการรับและไม่มีการส่งซ้ำโดยอัตโนมัติ จึงอาจสูญหายได้โดยผู้ส่งไม่ทราบ หากต้องการลดผลกระทบควรตรวจ Sequence Number, ส่งข้อมูลซ้ำ หรือเปลี่ยนคำสั่งสำคัญเป็น Unicast ที่มี ACK

### ข้อ 3: Broadcast, Multicast และ Unicast

**UDP Broadcast (`255.255.255.255`)** ส่งข้อมูลไปยังทุกอุปกรณ์ใน Broadcast Domain จึงเหมาะกับการประกาศ Telemetry หรือค้นหาอุปกรณ์ แต่สร้าง Traffic กับอุปกรณ์ที่ไม่ต้องการข้อมูล และมักไม่ข้าม Router

**UDP Multicast** เหมาะเมื่อมีผู้รับหลายเครื่องแต่ต้องการจำกัดเฉพาะสมาชิกในกลุ่ม ช่วยลด Traffic เมื่อเทียบกับ Broadcast แต่ต้องอาศัยการรองรับ Multicast จาก Router และ Access Point

**UDP Unicast** เหมาะกับการสื่อสารระหว่างผู้ส่งกับผู้รับรายเดียว เช่น คำสั่งควบคุม LED และ ACK เพราะควบคุมปลายทางได้ชัดเจนและไม่รบกวนอุปกรณ์อื่น
