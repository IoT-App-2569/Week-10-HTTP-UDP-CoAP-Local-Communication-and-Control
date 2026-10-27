# Week 10: HTTP, UDP, and CoAP Local Communication and Control
**รหัสนักศึกษา:** 67030134  
**วิชา:** Mobile Device Software Development (MDSD)  
**หัวข้อ:** การสื่อสารและควบคุมอุปกรณ์ IoT ในเครือข่ายท้องถิ่นด้วย HTTP RESTful API, UDP Socket และ CoAP

---

## โครงสร้างโฟลเดอร์ปฏิบัติการ (`67030134/`)
```text
67030134/
├── Report/
│   ├── image-1.png      # ผลการ Ping ตรวจสอบ mDNS (esp32-node.local)
│   ├── image-2.png      # ผลการทดสอบ cURL -i อ่าน Header และ JSON จาก HTTP REST Server
│   ├── image-3.png      # ผลการทดสอบอัตโนมัติด้วย test_rest_api.py
│   ├── image-4.png      # ผลการดักฟัง UDP Telemetry Broadcast ด้วย udp_listener.py
│   ├── image-5.png      # ผลการส่งคำสั่งควบคุม LED และวัด RTT Latency ด้วย udp_controller.py
│   ├── image-6.png      # ผลการทำ CoAP Resource Discovery (/.well-known/core)
│   ├── image-7.png      # ผลการทดสอบ CoAP GET Sensor และ PUT Actuator
│   └── image-8.png      # ผลการ Benchmark เปรียบเทียบประสิทธิภาพทั้ง 3 โปรโตคอล
├── report10-1.md        # รายงานผลการทดลอง ใบงานที่ 10.1 (mDNS & HTTP REST Server)
├── report10-2.md        # รายงานผลการทดลอง ใบงานที่ 10.2 (UDP Broadcast & Telemetry)
├── report10-3.md        # รายงานผลการทดลอง ใบงานที่ 10.3 (CoAP Server & Lightweight Control)
├── report10-4.md        # รายงานผลการทดลอง ใบงานที่ 10.4 (Protocol Benchmark & Forensics)
└── readme.md            # เอกสารสรุปภาพรวมการส่งงานของรหัส 67030134
```

---

## สรุปผลการทดลองทั้ง 4 ใบงาน

### 1. ใบงานที่ 10.1: mDNS Discovery และ HTTP RESTful API Server
- พัฒนาเว็บเซิร์ฟเวอร์บน ESP32 ด้วย `esp_http_server` พร้อมประกาศบริการผ่าน mDNS (`esp32-node.local`)
- รองรับ Endpoints: `GET /api/status`, `POST /api/led` และจำลอง **Virtual Potentiometer** ผ่าน `POST /api/pot`
- วิเคราะห์ Protocol Overhead ของ HTTP Header (71 ไบต์) เทียบกับ Payload (47 ไบต์) คิดเป็น Overhead สูงถึง 60.17%

### 2. ใบงานที่ 10.2: UDP Socket Communication และ Real-time Telemetry
- ถ่ายทอดข้อมูลแอนะล็อกเซนเซอร์ด้วยความถี่ 10 Hz ผ่าน UDP Broadcast (`255.255.255.255:3333`)
- พัฒนาสคริปต์ `udp_listener.py` รับข้อมูลโดยมี Packet Loss Rate 0.00%
- ทดสอบสั่งเปิด-ปิดไฟ LED และวัดค่า Round-Trip Latency เฉลี่ยได้ต่ำเพียง **11.24 ms**

### 3. ใบงานที่ 10.3: CoAP Server และการควบคุมอุปกรณ์ IoT น้ำหนักเบา
- พัฒนา CoAP Server บนพอร์ต UDP 5683 รองรับทรัพยากร `/sensor/pot` และ `/actuator/led`
- ทดสอบ Resource Discovery ตามมาตรฐาน CoRE Link Format (RFC 6690) ผ่าน `GET /.well-known/core`
- ควบคุมสถานะหลอดไฟและอ่านค่าเซนเซอร์ด้วยไลบรารี `aiocoap` บน Python

### 4. ใบงานที่ 10.4: Protocol Benchmark และการวิเคราะห์เชิงวิศวกรรม
- ทำการ Benchmark เปรียบเทียบโปรโตคอลทั้ง 3 แบบ 50 รอบการทดสอบ:
  - **UDP Socket:** ความหน่วงเวลาต่ำสุดเฉลี่ย **9.44 ms**, Jitter 3.88 ms, Overhead ต่ำสุด
  - **CoAP:** ความหน่วงเวลาปานกลางเฉลี่ย **21.85 ms**, Header ขนาดเล็กเพียง 4 ไบต์ และมีระบบยืนยัน (CON)
  - **HTTP REST:** ความหน่วงเวลาเฉลี่ย **182.45 ms**, ใช้งานสะดวกกับบราวเซอร์แต่มี Overhead สูงสุด
- วิเคราะห์และสังเคราะห์กรณีศึกษาการเลือกใช้โปรโตคอลในงานจริง (สมาร์ตฟาร์ม, แดชบอร์ดโรงงาน, แขนกลอุตสาหกรรม)
