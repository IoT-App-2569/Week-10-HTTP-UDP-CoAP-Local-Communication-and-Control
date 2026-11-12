# สรุปผลงานปฏิบัติการประจำสัปดาห์ที่ 10 (Week 10)
## เรื่อง: Local Control Protocols (HTTP, UDP, CoAP) & Edge Device Discovery

* **รหัสนักศึกษา:** 67030181
* **วิชา:** การสื่อสารและควบคุมในระดับเครือข่ายท้องถิ่นสำหรับอุปกรณ์ไอโอที (IoT Local Communication & Control)
* **บอร์ดทดลอง:** ESP32 / ESP32-C3 บนเฟรมเวิร์ก ESP-IDF (C / FreeRTOS)

---

### 1. โครงสร้างไฟล์ทั้งหมดในโฟลเดอร์ `67030181/`

ในโฟลเดอร์นี้ประกอบด้วยรายงานสรุปผลการทดลองพร้อมคำตอบท้ายบท และโปรเจกต์โค้ด ESP-IDF ครบทั้ง 4 แล็บ:

```text
67030281/
├── README.md                              <- หน้ารวมสรุปผลการทดลองและคำตอบทุกแล็บ
├── Lab10-1_Report.md                      <- รายงานผลและคำตอบท้ายบท แล็บ 10.1 (mDNS & HTTP REST Server)
├── Lab10-2_Report.md                      <- รายงานผลและคำตอบท้ายบท แล็บ 10.2 (UDP Socket & Real-time Telemetry)
├── Lab10-3_Report.md                      <- รายงานผลและคำตอบท้ายบท แล็บ 10.3 (CoAP Server & Lightweight Control)
├── Lab10-4_Report.md                      <- รายงานผลและคำตอบท้ายบท แล็บ 10.4 (Protocol Benchmark & Synthesis)
│
├── Lab10-1_HTTP_REST_Server/              <- ซอร์สโค้ดโปรเจกต์ ESP-IDF แล็บ 10.1
│   ├── CMakeLists.txt
│   ├── sdkconfig.defaults
│   └── main/
│       ├── CMakeLists.txt
│       ├── idf_component.yml             (espressif/mdns, espressif/cjson)
│       └── Lab10-1_HTTP_REST_Server.c
│
├── Lab10-2_UDP_Telemetry_Socket/          <- ซอร์สโค้ดโปรเจกต์ ESP-IDF แล็บ 10.2
│   ├── CMakeLists.txt
│   ├── udp_listener.py                   (สคริปต์ตรวจจับ Packet Loss)
│   ├── udp_controller.py                 (สคริปต์ส่งคำสั่งและวัด RTT)
│   └── main/
│       ├── CMakeLists.txt
│       └── Lab10-2_UDP_Telemetry_Socket.c
│
├── Lab10-3_CoAP_Server/                   <- ซอร์สโค้ดโปรเจกต์ ESP-IDF แล็บ 10.3
│   ├── CMakeLists.txt
│   ├── sdkconfig.defaults                (เปิด DTLS Cookie สำหรับ libcoap)
│   ├── test_coap.py                      (สคริปต์ทดสอบผ่าน aiocoap)
│   ├── benchmark_protocols.py
│   └── main/
│       ├── CMakeLists.txt
│       ├── idf_component.yml             (espressif/coap)
│       └── Lab10-3_CoAP_Server.c
│
└── Lab10-4_Protocol_Benchmark/            <- สคริปต์ Python ทดสอบสมรรถนะเปรียบเทียบ แล็บ 10.4
    └── benchmark_protocols.py
```

---

### 2. การต่อวงจรฮาร์ดแวร์

ทุกแล็บในสัปดาห์นี้ใช้การต่อวงจรรูปแบบเดียวกัน:
* **หลอดไฟ LED (Actuator):** ต่อขาบวก (Anode) เข้าขา **GPIO 2** ของ ESP32
* **ตัวต้านทานปรับค่าได้ Potentiometer 10k (Sensor):** ต่อขากลาง (Wiper) เข้าขา **GPIO 34** (ADC1 Channel 6) ส่วนขาซ้าย/ขวาต่อไฟเลี้ยง **3.3V** และ **GND**

---

### 3. สรุปคำตอบท้ายบทของแต่ละแล็บ

#### [แล็บ 10.1: mDNS Discovery และ HTTP REST Server](Lab10-1_Report.md)
1. **วิเคราะห์ Header vs Payload:** จากการรัน `curl -i` พบว่า HTTP Response Header มีขนาด 94 ไบต์ แต่ตัวข้อมูล JSON มีขนาดแค่ 46 ไบต์ ทำให้ Header กินพื้นที่ไปถึง 67% เปลืองแบนด์วิดท์มาก
2. **ปัญหาเมื่อไม่มี mDNS หรือบล็อกพอร์ต 5353:** เครื่องจะหาชื่อ `esp32-node.local` ไม่เจอ เข้าเว็บไม่ได้ วิธีแก้คือปรับ Wi-Fi ใน Windows ให้เป็น Private network หรือเรียกผ่าน IP ของบอร์ดโดยตรง
3. **ทำไมต้อง Delete และ Free cJSON เสมอ:** เพราะ cJSON ใช้ `malloc()` จองแรมบน Heap ถ้าไม่คืนแรม แรมจะรั่ว (Memory Leak) สะสมจนบอร์ดเกิด Out of Memory แล้วรีสตาร์ทตัวเองทันที

#### [แล็บ 10.2: UDP Socket และ Real-time Telemetry Broadcast](Lab10-2_Report.md)
1. **เปรียบเทียบ RTT Latency:** UDP มีความหน่วงเฉลี่ยแค่ **4.55 ms** เร็วกว่า HTTP (58.65 ms) ถึง **13 เท่า** เพราะไม่มีขั้นตอน TCP Handshake และไม่ต้อง Parse ข้อความยาวๆ
2. **วิเคราะห์ Packet Loss:** รันดักฟัง 1 นาที (600 แพ็กเก็ต) พบแพ็กเก็ตหล่นหายประมาณ **1.33%** เพราะ Wi-Fi Broadcast จะไม่มีการส่ง ACK และไม่มีการส่งซ้ำ บวกกับสัญญาณรบกวนคลื่น 2.4 GHz
3. **ข้อดี-ข้อจำกัดของ Broadcast:** สะดวกตอนค้นหาอุปกรณ์ครั้งแรก แต่ถ้ายิงตลอดจะรบกวนเครื่องอื่นทั้งวงแลน (Broadcast Storm) และกินแบตเตอรี่ ควรเปลี่ยนเป็น **Multicast** เมื่อต้องการส่งหาเฉพาะกลุ่ม หรือเปลี่ยนเป็น **Unicast** เมื่อทราบ IP แล้ว

#### [แล็บ 10.3: CoAP Server และการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา](Lab10-3_Report.md)
1. **CoRE Link Format (RFC 6690):** ได้ผลลัพธ์เป็น `</sensor/pot>;rt="sensor";if="sensor",</actuator/led>;rt="actuator";if="control"` ช่วยให้อุปกรณ์คุยกันเองได้อัตโนมัติ (M2M) รู้ว่ามี URL อะไรและทำหน้าที่อะไรได้บ้าง
2. **ความแตกต่าง CON vs NON:** แบบ CON มีการตอบ ACK และส่งซ้ำให้อัตโนมัติถ้าข้อมูลหลุด มั่นใจได้ 100% เหมาะกับ Actuator ส่วนแบบ NON ส่งขาเดียวไม่รอคำตอบ เหมาะกับสตรีมข้อมูลเซนเซอร์
3. **ทำไม CoAP ถึงเหมาะกับ Thread / NB-IoT มากกว่า HTTP:** เพราะ CoAP มี Header คงที่แค่ 4 ไบต์ ยัดลงในเฟรม IEEE 802.15.4 (MTU 127 ไบต์) ได้ในแพ็กเก็ตเดียว ไม่เกิดการแตกกระจายของแพ็กเก็ต และส่งเสร็จเข้าโหมด Deep Sleep ได้ทันที ประหยัดถ่านได้หลายปี

#### [แล็บ 10.4: ตาราง Benchmark เปรียบเทียบทั้ง 3 โปรโตคอล](Lab10-4_Report.md)

| ค่าที่วัดได้จากการทดสอบจริง | HTTP REST (TCP) | UDP Raw Socket | CoAP (RFC 7252) |
| :--- | :---: | :---: | :---: |
| **ขนาดส่วนหัวรวม (L4 + L7)** | ~200–280 ไบต์ | **8 ไบต์** | **12 ไบต์** |
| **สัดส่วนข้อมูลจริง (Payload Ratio)** | ~4.7 % - 6.5 % | **~42.8 %** | **~7.7 %** |
| **ความหน่วงเฉลี่ย (Mean RTT)** | 56.42 ms | **4.68 ms** | **12.93 ms** |
| **ความแกว่งของเวลา (Jitter)** | 12.35 ms | **0.98 ms** | 8.85 ms |
| **การกินพื้นที่แรม Free Heap** | ~ 32 KB | **~ 7 KB** | ~ 43 KB |
| **การรับประกันการส่งข้อมูล** | รับประกัน (TCP ACK) | ไม่รับประกัน (Best-effort) | เลือกได้ (CON / NON) |

* **สรุปการเลือกโปรโตคอลในกรณีศึกษา:**
  * *เซนเซอร์ความชื้นดินและวาล์วน้ำโซลาร์เซลล์:* **เลือก CoAP** (ส่งเซนเซอร์แบบ NON ประหยัดไฟ สั่งวาล์วน้ำแบบ CON เพื่อความแน่นอน)
  * *เว็บแดชบอร์ดซ่อมบำรุงในโรงงาน:* **เลือก HTTP REST (mDNS)** (เปิดดูได้ทันทีผ่าน Web Browser ทุกชนิด ไม่ต้องลงโปรแกรมเพิ่ม)
  * *ระบบควบคุมแขนกลความเร็วสูง 100 Hz ในแลนปิด:* **เลือก UDP Raw Socket** (ความหน่วงต่ำสุด 4 ms, Jitter ต่ำกว่า 1 ms, ไม่กระตุก)

---

### 4. วิธีการสั่งคอมไพล์และทดสอบ (Build & Flash)

1. **คอมไพล์และแฟลชโค้ด ESP-IDF:**
   เข้าไปที่โฟลเดอร์ของแล็บที่ต้องการ เช่น แล็บ 10.1:
   ```powershell
   cd 67030281/Lab10-1_HTTP_REST_Server
   idf.py set-target esp32
   idf.py build
   idf.py flash -p COMx monitor
   ```
2. **รันสคริปต์ Benchmark บนคอมพิวเตอร์:**
   ```powershell
   pip install aiocoap
   python 67030281/Lab10-4_Protocol_Benchmark/benchmark_protocols.py --target 192.168.1.181 --rounds 50
   ```
