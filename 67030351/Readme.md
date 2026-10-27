# สรุปงานการทดลอง สัปดาห์ที่ 10 (Week 10)
**รหัสนักศึกษา** 67030351  
**หัวข้อ** Local Communication & Control (HTTP, UDP, CoAP)

---

## ตำแหน่งไฟล์รายงานทั้งหมด

ไฟล์รายงานจัดเก็บอยู่ในโฟลเดอร์ `Report/` และมีสำเนาอยู่ที่ Root Directory

* **ใบงานที่ 10.1 (HTTP REST & mDNS)** [Report/Report-Lab01.md](Report/Report-Lab01.md)
* **ใบงานที่ 10.2 (UDP Telemetry)** [Report/Report-Lab02.md](Report/Report-Lab02.md)
* **ใบงานที่ 10.3 (CoAP Server)** [Report/Report-Lab03.md](Report/Report-Lab03.md)
* **ใบงานที่ 10.4 (Protocol Benchmark)** [Report/Report-Lab04.md](Report/Report-Lab04.md)

---

## สรุปผลการทดลองแบบย่อ

| ใบงาน | หัวข้อหลัก | ผลการทดลองสำคัญ |
| :--- | :--- | :--- |
| **Lab 10.1** | HTTP REST & mDNS | Resolve ชื่อ `esp32-node.local` ได้ IP `192.168.1.50`, ควบคุม LED และอ่าน Potentiometer ผ่าน JSON สำเร็จ (Header Overhead สูง ~60.7%) |
| **Lab 10.2** | UDP Socket & Telemetry | บรอดแคสต์ Telemetry พอร์ต 3334 หมุนค่า Pot 0 ถึง 2346 สำเร็จ, สถิติ Packet Loss Rate 19.06%, Latency ต่ำ 1–5 ms |
| **Lab 10.3** | CoAP Server (RFC 7252) | ค้นหา Resource ผ่าน `/.well-known/core` ได้ `</sensor/pot>,</actuator/led>`, สั่งเปิด-ปิด LED สำเร็จ (Code 2.04 Changed) |
| **Lab 10.4** | Benchmark เปรียบเทียบ | UDP เร็วสุด (4.82 ms), CoAP ประหยัดข้อมูลสุด (Header 4 ไบต์, 12.93 ms), HTTP ช้าสุด (38.45 ms, Header ใหญ่สุด) |

---

## การต่อฮาร์ดแวร์

* **LED (Actuator)** ต่อขา GPIO 2
* **Potentiometer (Sensor)** ต่อขา GPIO 34 (ADC1 Channel 6)