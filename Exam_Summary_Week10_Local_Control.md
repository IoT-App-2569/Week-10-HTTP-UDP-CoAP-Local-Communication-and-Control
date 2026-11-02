# สรุปเนื้อหาและใจความสำคัญเตรียมสอบ: Week 10
## การสื่อสารและการควบคุมอุปกรณ์ภายในเครือข่ายท้องถิ่น (Local Communication & Control: HTTP, UDP, CoAP & Device Discovery)

---

> **เกี่ยวกับเอกสารฉบับนี้:**
> เอกสารฉบับนี้จัดทำขึ้นเพื่อใช้สรุปทบทวนเนื้อหาและเตรียมตัวสอบ วิชาการพัฒนาอุปกรณ์ระบบไอโอที (IoT Systems Development) ประจำสัปดาห์ที่ 10 โดยสังเคราะห์ข้อมูลจากเอกสารคำสอน (Lecture Notes 01–04), ใบงานการทดลอง (Labsheets 06–09), ซอร์สโค้ดตัวอย่างบน ESP-IDF (C / FreeRTOS) และรายงานผลการทดลองจริง (Benchmark & Engineering Answers) จัดรูปแบบด้วย Markdown มาตรฐาน สามารถคัดลอก (Copy) ไปวางใน **Microsoft Word** เพื่อแปลงเป็นหัวข้อ ตาราง และสารบัญอัตโนมัติได้อย่างสมบูรณ์

---

## สารบัญ (Table of Contents)

1. [ภาพรวมโครงสร้างบทเรียนและโครงสร้างไฟล์ในโปรเจกต์](#1-ภาพรวมโครงสร้างบทเรียนและโครงสร้างไฟล์ในโปรเจกต์)
2. [บทที่ 1: สถาปัตยกรรม Local Control และการค้นหาอุปกรณ์ (Device Discovery)](#2-บทที่-1-สถาปัตยกรรม-local-control-และการค้นหาอุปกรณ์-device-discovery)
   - [2.1 เปรียบเทียบ Cloud Control กับ Local Control](#21-เปรียบเทียบ-cloud-control-กับ-local-control)
   - [2.2 ปัญหาการค้นหาอุปกรณ์ในวงแลน (Device Discovery Problem)](#22-ปัญหาการค้นหาอุปกรณ์ในวงแลน-device-discovery-problem)
   - [2.3 การส่งข้อมูลแบบ Unicast, Broadcast, และ Multicast](#23-การส่งข้อมูลแบบ-unicast-broadcast-และ-multicast)
   - [2.4 การค้นหาด้วยวิธีบรอดแคสต์ (Broadcast Discovery)](#24-การค้นหาด้วยวิธีบรอดแคสต์-broadcast-discovery)
   - [2.5 การค้นหาด้วยวิธีมัลติแคสต์ (Multicast Discovery)](#25-การค้นหาด้วยวิธีมัลติแคสต์-multicast-discovery)
   - [2.6 ตารางเปรียบเทียบ Broadcast vs Multicast](#26-ตารางเปรียบเทียบ-broadcast-vs-multicast)
   - [2.7 โปรโตคอล Multicast DNS (mDNS) และ DNS-SD (RFC 6762 / 6763)](#27-โปรโตคอล-multicast-dns-mdns-และ-dns-sd-rfc-6762--6763)
3. [บทที่ 2: เว็บเซิร์ฟเวอร์ HTTP และสถาปัตยกรรม RESTful API บน ESP-IDF](#3-บทที่-2-เว็บเซิร์ฟเวอร์-http-และสถาปัตยกรรม-restful-api-บน-esp-idf)
   - [3.1 บทบาทของ HTTP ในงานควบคุมอุปกรณ์ IoT](#31-บทบาทของ-http-ในงานควบคุมอุปกรณ์-iot)
   - [3.2 การคงสถานะการเชื่อมต่อ: HTTP 1.0 vs HTTP 1.1 Keep-Alive](#32-การคงสถานะการเชื่อมต่อ-http-10-vs-http-11-keep-alive)
   - [3.3 การออกแบบ RESTful API สำหรับอุปกรณ์ IoT](#33-การออกแบบ-restful-api-สำหรับอุปกรณ์-iot)
   - [3.4 สถาปัตยกรรม `esp_http_server` บน ESP-IDF](#34-สถาปัตยกรรม-esp_http_server-บน-esp-idf)
   - [3.5 กฎเหล็กการบริหารจัดการหน่วยความจำ cJSON เพื่อป้องกัน Memory Leak](#35-กฎเหล็กการบริหารจัดการหน่วยความจำ-cjson-เพื่อป้องกัน-memory-leak)
   - [3.6 การยกระดับความปลอดภัยด้วย HTTPS (HTTP over TLS)](#36-การยกระดับความปลอดภัยด้วย-https-http-over-tls)
4. [บทที่ 3: การสื่อสารระดับซ็อกเก็ต UDP และข้อมูล Telemetry แบบเรียลไทม์](#4-บทที่-3-การสื่อสารระดับซ็อกเก็ต-udp-และข้อมูล-telemetry-แบบเรียลไทม์)
   - [4.1 คุณลักษณะเด่นของ UDP (RFC 768)](#41-คุณลักษณะเด่นของ-udp-rfc-768)
   - [4.2 ตารางเปรียบเทียบความแตกต่างระหว่าง TCP และ UDP](#42-ตารางเปรียบเทียบความแตกต่างระหว่าง-tcp-และ-udp)
   - [4.3 ทำไมงาน IoT และ Local Control ยังจำเป็นต้องใช้ UDP](#43-ทำไมงาน-iot-และ-local-control-ยังจำเป็นต้องใช้-udp)
   - [4.4 การเขียนโปรแกรม BSD Socket บน LwIP (ESP-IDF)](#44-การเขียนโปรแกรม-bsd-socket-บน-lwip-esp-idf)
   - [4.5 การจัดการปัญหา Packet Loss ในระดับ Application Layer](#45-การจัดการปัญหา-packet-loss-ในระดับ-application-layer)
5. [บทที่ 4: โปรโตคอล CoAP สำหรับระบบสมองกลฝังตัวทรัพยากรจำกัด (RFC 7252)](#5-บทที่-4-โปรโตคอล-coap-สำหรับระบบสมองกลฝังตัวทรัพยากรจำกัด-rfc-7252)
   - [5.1 จุดกำเนิดและแนวคิดของ CoAP](#51-จุดกำเนิดและแนวคิดของ-coap)
   - [5.2 ตารางเปรียบเทียบคุณสมบัติระหว่าง HTTP และ CoAP](#52-ตารางเปรียบเทียบคุณสมบัติระหว่าง-http-และ-coap)
   - [5.3 โครงสร้างแพ็กเก็ตและส่วนหัวขนาด 4 ไบต์ (4-Byte Fixed Header)](#53-โครงสร้างแพ็กเก็ตและส่วนหัวขนาด-4-ไบต์-4-byte-fixed-header)
   - [5.4 ประเภทของข้อความ CoAP (Message Types: CON, NON, ACK, RST)](#54-ประเภทของข้อความ-coap-message-types-con-non-ack-rst)
   - [5.5 รหัสคำขอและรหัสตอบกลับ (CoAP Request & Response Codes)](#55-รหัสคำขอและรหัสตอบกลับ-coap-request--response-codes)
   - [5.6 กลไกขั้นสูง: CoAP Observe (RFC 7641) และ CoRE Link Format (RFC 6690)](#56-กลไกขั้นสูง-coap-observe-rfc-7641-และ-core-link-format-rfc-6690)
6. [บทที่ 5: สรุปสาระสำคัญจากการทดลองปฏิบัติการ (Lab 10.1 - 10.4)](#6-บทที่-5-สรุปสาระสำคัญจากการทดลองปฏิบัติการ-lab-101---104)
   - [6.1 การต่อวงจรฮาร์ดแวร์พื้นฐาน (Hardware Wiring)](#61-การต่อวงจรฮาร์ดแวร์พื้นฐาน-hardware-wiring)
   - [6.2 แล็บ 10.1: mDNS Discovery & HTTP REST Server](#62-แล็บ-101-mdns-discovery--http-rest-server)
   - [6.3 แล็บ 10.2: UDP Broadcast & Real-time Telemetry](#63-แล็บ-102-udp-broadcast--real-time-telemetry)
   - [6.4 แล็บ 10.3: CoAP Server & Lightweight IoT Control](#64-แล็บ-103-coap-server--lightweight-iot-control)
   - [6.5 แล็บ 10.4: ผลการทดสอบเปรียบเทียบสมรรถนะ (Benchmark Summary)](#65-แล็บ-104-ผลการทดสอบเปรียบเทียบสมรรถนะ-benchmark-summary)
7. [บทที่ 6: คลังข้อสอบและแนวทางคำตอบเชิงวิเคราะห์ทางวิศวกรรม (Exam Question Bank)](#7-บทที่-6-คลังข้อสอบและแนวทางคำตอบเชิงวิเคราะห์ทางวิศวกรรม-exam-question-bank)
   - [7.1 การคำนวณสัดส่วนประสิทธิภาพข้อมูล (Payload Efficiency Ratio)](#71-การคำนวณสัดส่วนประสิทธิภาพข้อมูล-payload-efficiency-ratio)
   - [7.2 การเปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix)](#72-การเปรียบเทียบจุดเด่นและจุดด้อย-pros--cons-matrix)
   - [7.3 กรณีศึกษาโจทย์วิเคราะห์การตัดสินใจเลือกโปรโตคอล (Case Studies)](#73-กรณีศึกษาโจทย์วิเคราะห์การตัดสินใจเลือกโปรโตคอล-case-studies)
   - [7.4 คำถาม-คำตอบเจาะลึกที่พบบ่อยในห้องสอบ](#74-คำถาม-คำตอบเจาะลึกที่พบบ่อยในห้องสอบ)
8. [บทที่ 7: อภิธานศัพท์ คำย่อ และตัวเลขสำคัญที่ต้องจำเข้าห้องสอบ (Cheat Sheet)](#8-บทที่-7-อภิธานศัพท์-คำย่อ-และตัวเลขสำคัญที่ต้องจำเข้าห้องสอบ-cheat-sheet)

---

## 1. ภาพรวมโครงสร้างบทเรียนและโครงสร้างไฟล์ในโปรเจกต์

โครงสร้างโฟลเดอร์ในสัปดาห์ที่ 10 ประกอบด้วยเอกสารทฤษฎี เอกสารใบงานปฏิบัติการ โค้ดโปรเจกต์บน ESP32 และสคริปต์ทดสอบสมรรถนะ ดังนี้:

```text
Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control/
│
├── เอกสารบรรยายเชิงทฤษฎี (Lecture Modules)
│   ├── 01-Local-Control-Architecture-and-Discovery.md        (สถาปัตยกรรม Local Control, Broadcast, Multicast, mDNS)
│   ├── 02-HTTP-Server-and-RESTful-API-on-ESP-IDF.md          (HTTP Web Server, REST API, cJSON, HTTPS)
│   ├── 03-UDP-Socket-Communication-and-Realtime-Telemetry.md (BSD Socket บน UDP, Low Latency, Telemetry)
│   ├── 04-CoAP-Protocol-for-Constrained-IoT.md               (CoAP RFC 7252, 4-Byte Header, CON/NON, Observe)
│   └── 05-Glossary.md                                        (อภิธานศัพท์และคำย่อทางเทคนิค)
│
├── เอกสารใบงานปฏิบัติการ (Laboratory Sheets)
│   ├── 06-Labsheet-10-1-mDNS-Discovery-and-HTTP-REST-Server.md
│   ├── 07-Labsheet-10-2-UDP-Broadcast-and-Realtime-Telemetry.md
│   ├── 08-Labsheet-10-3-CoAP-Server-and-Lightweight-IoT-Control.md
│   └── 09-Labsheet-10-4-Protocol-Benchmark-and-Network-Forensics.md
│
├── เครื่องมือทดสอบและสคริปต์
│   └── benchmark_protocols.py                                (เครื่องมือวัด RTT, Jitter, Packet Loss, Overhead)
│
└── Example_ESP32_Project/ (โปรเจกต์ต้นแบบและเฉลยรายงานผลการทดลอง)
    ├── Lab10-1_HTTP_REST_Server/      (โค้ด C + LAB_REPORT_ANSWERS.md)
    ├── Lab10-2_UDP_Telemetry_Socket/  (โค้ด C + LAB_REPORT_ANSWERS.md)
    ├── Lab10-3_CoAP_Server/           (โค้ด C + test_coap.py + LAB_REPORT_ANSWERS.md)
    └── Lab10-4_Protocol_Benchmark/    (benchmark_protocols.py + LAB_REPORT_ANSWERS.md)
```

---

## 2. บทที่ 1: สถาปัตยกรรม Local Control และการค้นหาอุปกรณ์ (Device Discovery)

### 2.1 เปรียบเทียบ Cloud Control กับ Local Control

ในระบบ IoT การสั่งการอุปกรณ์ (เช่น การเปิด-ปิดหลอดไฟ หรือการสั่งงานรีเลย์) แบ่งออกเป็น 2 รูปแบบหลัก:

1. **Cloud Control (การควบคุมผ่านระบบคลาวด์):**
   - คำสั่งต้องเดินทางจากสมาร์ตโฟนขึ้นสู่ Cloud Server บนอินเทอร์เน็ต แล้วจึงส่งกลับลงมายังอุปกรณ์ที่บ้าน
   - **ข้อจำกัดสำคัญ 3 ประการ:**
     - **ความหน่วงเวลาสูง (Latency):** มีค่าความหน่วงเฉลี่ย 50 ms จนถึงหลายวินาที ขึ้นอยู่กับความเร็วอินเทอร์เน็ต
     - **พึ่งพาอินเทอร์เน็ตตลอดเวลา (Internet Dependency):** หากอินเทอร์เน็ตของผู้ให้บริการหลุด ระบบภายในบ้านจะหยุดทำงานทันที (Device Paralysis)
     - **ความเสี่ยงด้านความเป็นส่วนตัว (Privacy & Security):** ข้อมูลสถานะ เสียง หรือภาพภายในบ้านต้องถูกส่งออกนอกอาคาร

2. **Local Control (การควบคุมภายในเครือข่ายท้องถิ่น):**
   - การสื่อสารระหว่าง Client (แอปบนมือถือ/คอมพิวเตอร์) และ Server (ESP32) เดินทางอยู่เฉพาะในเครือข่ายแลน (LAN) ผ่าน Wi-Fi หรืออีเทอร์เน็ต
   - **ประโยชน์หลัก 4 ประการ (มักออกสอบ):**
     1. **Real-time & Low Latency:** ความหน่วงเวลาต่ำมาก (โดยทั่วไป < 5–10 ms) สั่งงานแล้วอุปกรณ์ตอบสนองทันที
     2. **High Availability & Fault Tolerance:** ทำงานได้ 100% แม้สายอินเทอร์เน็ตภายนอกจะขาด
     3. **Data Locality & Privacy:** ข้อมูลทั้งหมดอยู่เฉพาะในบ้าน ไม่รั่วไหลออกสู่อินเทอร์เน็ตภายนอก
     4. **Bandwidth Saving:** ประหยัดแบนด์วิดท์ของอินเทอร์เน็ตเกตเวย์

---

### 2.2 ปัญหาการค้นหาอุปกรณ์ในวงแลน (Device Discovery Problem)

- เมื่ออุปกรณ์ ESP32 บูตและเชื่อมต่อ Wi-Fi มักจะได้รับหมายเลข IP Address แบบสุ่มผ่าน DHCP จากเราเตอร์ (เช่น `192.168.1.105`)
- ผู้ใช้หรือแอปพลิเคชันบนสมาร์ตโฟน **ไม่ทราบว่า ESP32 ได้รับหมายเลข IP ใด**
- **ข้อจำกัดของโปรโตคอลระดับ L2/L3 ดั้งเดิม:**
  - **ARP (Address Resolution Protocol):** รู้ IP อยู่แล้ว แต่ต้องการค้นหา MAC Address ปลายทาง
  - **RARP (Reverse ARP):** รู้ MAC Address อยู่แล้ว แต่ต้องการค้นหา IP
  - ทั้ง ARP และ RARP จำเป็นต้องทราบข้อมูลฝั่งใดฝั่งหนึ่งล่วงหน้า จึงไม่สามารถนำมาใช้ค้นหาอุปกรณ์ใหม่ที่ไม่ทราบทั้ง IP และ MAC ได้
- **ทางออก:** ต้องใช้เทคนิค **Broadcast Discovery** หรือ **Multicast Discovery**

---

### 2.3 การส่งข้อมูลแบบ Unicast, Broadcast, และ Multicast

| รูปแบบการส่ง | ลักษณะการกระจายข้อมูล | จุดเด่น | ข้อจำกัด |
| :--- | :--- | :--- | :--- |
| **Unicast (1 ต่อ 1)** | ส่งจากโฮสต์ต้นทางไปยังโฮสต์ปลายทางตัวเดียว | ประหยัดแบนด์วิดท์, ปลอดภัย | ต้องทราบ IP ปลายทางอย่างแม่นยำ ไม่สามารถใช้ค้นหาอุปกรณ์เริ่มต้นได้ |
| **Broadcast (1 ต่อ All)** | ส่งไปยังทุกโฮสต์ในเครือข่ายพร้อมกัน | ใช้งานง่าย ทุกอุปกรณ์ในวงแลนได้รับแน่นอน | สร้างภาระให้เครือข่าย (Broadcast Storm) และรบกวน CPU ของทุกโฮสต์ |
| **Multicast (1 ต่อ Many)** | ส่งเฉพาะกลุ่มโฮสต์ที่ลงทะเบียนสนใจไว้ | ไม่รบกวนโฮสต์ที่ไม่เกี่ยวข้อง, ประหยัดแบนด์วิดท์ | ต้องมีโปรโตคอลจัดการกลุ่ม (IGMP) และตั้งค่าซ็อกเก็ตซับซ้อนกว่า |

---

### 2.4 การค้นหาด้วยวิธีบรอดแคสต์ (Broadcast Discovery)

- ทำงานบนโปรโตคอล **UDP** เสมอ (TCP ไม่รองรับ Broadcast)
- **แอดเดรสบรอดแคสต์แบ่งเป็น 2 ชั้น:**
  - **Layer 2 (MAC Address):** `FF:FF:FF:FF:FF:FF`
  - **Layer 3 (IP Address):**
    1. **Limited Broadcast (`255.255.255.255`):** ส่งถึงทุกอุปกรณ์ที่เกาะอยู่บนเราเตอร์เดียวกัน แต่เราเตอร์จะไม่ส่งต่อข้าม Subnet
    2. **Subnet-directed Broadcast (เช่น `192.168.1.255` สำหรับ `/24`):** เป็นการกำหนดบิต Host ID เป็น 1 ทั้งหมด ส่งเฉพาะโฮสต์ในวงเครือข่ายย่อยนั้น
- **ขั้นตอนการทำ Broadcast Discovery บน ESP-IDF:**
  1. ฝั่ง Sender เปิด Socket แบบ `SOCK_DGRAM`
  2. ตั้งค่า Socket Option: `setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt))`
  3. ใช้คำสั่ง `sendto()` ยิงข้อความคำถามไปยัง IP `255.255.255.255` พอร์ตเป้าหมาย
  4. ฝั่ง Receiver ซึ่ง `bind()` รออยู่ที่พอร์ตนั้น ตอบกลับด้วยแพ็กเก็ต **Unicast** เพื่อแจ้ง IP และข้อมูลบริการ
- **ข้อเสีย:** รบกวน CPU ของอุปกรณ์อื่นทุกตัวในเครือข่ายที่ต้องตื่นมาประมวลผลแล้วทิ้งแพ็กเก็ต

---

### 2.5 การค้นหาด้วยวิธีมัลติแคสต์ (Multicast Discovery)

- ส่งข้อมูลเฉพาะโฮสต์ที่เข้าร่วมกลุ่ม ผ่านโปรโตคอล **IGMP (Internet Group Management Protocol)**
- **ช่วงไอพีแอดเดรสมัลติแคสต์ (Class D: `224.0.0.0` ถึง `239.255.255.255`):**
  - `224.0.0.0` – `224.0.0.255`: Reserved สำหรับโปรโตคอลเครือข่ายท้องถิ่น (เช่น `224.0.0.251` สำหรับ mDNS)
  - `239.0.0.0` – `239.255.255.255`: Administrative Scope (ใช้งานเฉพาะภายในองค์กร/วงแลน)
- **การตั้งค่า Socket สำหรับ Multicast Sender บน ESP-IDF:**
  1. ระบุ Interface ขาออก: `setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &iaddr, sizeof(struct in_addr))`
  2. กำหนดค่า Time-To-Live (TTL = 1 เพื่อจำกัดไม่ให้ออกนอกเราเตอร์): `setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl))`
  3. เข้าร่วมกลุ่ม: `setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imreq, sizeof(struct ip_mreq))`

---

### 2.6 ตารางเปรียบเทียบ Broadcast vs Multicast

| มิติการเปรียบเทียบ | การบรอดแคสต์ (Broadcast) | การมัลติแคสต์ (Multicast) |
| :--- | :--- | :--- |
| **เป้าหมายผู้รับ** | ทุกโฮสต์ในเครือข่าย (One-to-All) | เฉพาะกลุ่มโฮสต์ที่ลงทะเบียนสนใจ (One-to-Many) |
| **การจัดการกลุ่ม** | ไม่มีการจัดการกลุ่ม | ใช้โปรโตคอล **IGMP** สมัครและถอนตัว |
| **ภาระบนแบนด์วิดท์** | สูง เสี่ยงต่อการเกิด Broadcast Storm | ต่ำ ควบคุมการส่งได้มีประสิทธิภาพ |
| **ผลกระทบต่อ CPU ของโฮสต์อื่น** | สูง (ทุกโฮสต์ต้องถูก Interrupt เพื่อแกะดูแพ็กเก็ต) | ต่ำมาก (การ์ดแลนทิ้งแพ็กเก็ตระดับฮาร์ดแวร์หากไม่ได้ Join กลุ่ม) |
| **ความเหมาะสมกับงาน IoT** | เหมาะสำหรับแล็บขนาดเล็กมาก | **เหมาะสมที่สุดสำหรับระบบ Smart Home & IoT มาตรฐาน** |

---

### 2.7 โปรโตคอล Multicast DNS (mDNS) และ DNS-SD (RFC 6762 / 6763)

mDNS นำข้อดีของ DNS แบบดั้งเดิมมารวมเข้ากับ Multicast เพื่อให้สามารถค้นหาอุปกรณ์และเรียกชื่อโฮสต์ได้โดยไม่ต้องมี DNS Server ส่วนกลาง (Zero-Configuration Networking)

- **คุณสมบัติทางเทคนิคที่ต้องจำ:**
  - ทำงานบนโปรโตคอล **UDP พอร์ต 5353**
  - มัลติแคสต์แอดเดรสมาตรฐาน: **`224.0.0.251`** (IPv4) หรือ **`ff02::fb`** (IPv6)
  - โดเมนเฉพาะ: ลงท้ายด้วย **`.local`** (เช่น `esp32-node.local`)

- **โครงสร้างระเบียน (DNS Records) ใน mDNS และ DNS-SD (ออกสอบบ่อย):**
  1. **PTR (Pointer Record):** ทำหน้าที่ **Service Discovery** ชี้จาก "ประเภทบริการ (Service Type)" เช่น `_http._tcp` ไปยัง "ชื่ออุปกรณ์ (Instance Name)" เช่น `esp32_smart_light._http._tcp.local`
  2. **SRV (Service Record):** ระบุว่าบริการนั้นรันอยู่บน **Hostname ใด** และเปิดให้บริการอยู่ที่ **Port ใด** (เช่น โฮสต์ `esp32-node.local` พอร์ต `80`)
  3. **TXT (Text Record):** เก็บ **Metadata เพิ่มเติม (Key-Value Pairs)** เช่น `path=/api/status` หรือ `board=esp32` ช่วยให้แอปพลิเคชันทราบคุณสมบัติโดยไม่ต้องต่อเข้าไปถาม
  4. **A Record (Address Record):** ทำหน้าที่แปลงชื่อ Hostname (เช่น `esp32-node.local`) ไปเป็นหมายเลข **IPv4 Address จริง** (เช่น `192.168.1.181`) ส่วน **AAAA Record** ใช้สำหรับ IPv6

---

## 3. บทที่ 2: เว็บเซิร์ฟเวอร์ HTTP และสถาปัตยกรรม RESTful API บน ESP-IDF

### 3.1 บทบาทของ HTTP ในงานควบคุมอุปกรณ์ IoT

- **ข้อจำกัดของ Raw TCP Socket:** ต้องคิดรูปแบบข้อมูล (Framing) เอง ขาดมาตรฐานสากล เว็บบราวเซอร์ทั่วไปไม่สามารถเชื่อมต่อได้โดยตรง
- **ข้อดีของ HTTP (RFC 7230):**
  - เป็นโปรโตคอลระดับแอปพลิเคชัน (Application Layer) บน TCP
  - มีโครงสร้าง **Request / Response** ชัดเจน (Method, URL, Headers, Status Code, Body)
  - เข้ากันได้กับเว็บบราวเซอร์ทุกชนิด และรองรับภาษาคอมพิวเตอร์ทุกภาษา

---

### 3.2 การคงสถานะการเชื่อมต่อ: HTTP 1.0 vs HTTP 1.1 Keep-Alive

- **HTTP 1.0 (Non-persistent Connection):**
  - ทุก Request / Response จะต้องเริ่มทำ TCP 3-Way Handshake ใหม่ และตัดการเชื่อมต่อ (Close) ทันทีที่ส่งเสร็จ
  - สิ้นเปลืองเวลา RTT และพลังงานของชิปไมโครคอนโทรลเลอร์อย่างยิ่ง
- **HTTP 1.1 (Persistent Connection / HTTP Keep-Alive):**
  - รักษาการเชื่อมต่อ TCP เดิมไว้ให้ใช้งานซ้ำได้หลาย Request
  - ลดภาระ Handshake และลดเวลาตอบสนองของระบบลงได้อย่างมหาศาล

---

### 3.3 การออกแบบ RESTful API สำหรับอุปกรณ์ IoT

สถาปัตยกรรม **REST (Representational State Transfer)** นิยมใช้ URI ระบุทรัพยากร (Resource) และใช้ **JSON** เป็นสื่อกลางแลกเปลี่ยนข้อมูล

| HTTP Method | URI Endpoint | วัตถุประสงค์ | ข้อมูล Request (Payload) | ตัวอย่าง Response (JSON) |
| :---: | :--- | :--- | :---: | :--- |
| **GET** | `/api/status` | สอบถามสถานะเซนเซอร์และฮาร์ดแวร์ | ไม่มี | `{"pot_raw": 15, "free_heap": 213848, "led": false}` |
| **POST** | `/api/led` | สั่งเปลี่ยนสถานะหลอดไฟ LED | `{"state": true}` | `{"result": "success", "led": true}` |

---

### 3.4 สถาปัตยกรรม `esp_http_server` บน ESP-IDF

- เป็นคอมโพเนนต์ทางการของ Espressif ออกแบบมาสำหรับระบบสมองกลฝังตัว
- **ฟีเจอร์สำคัญ:**
  - **Multi-tasking Event-driven:** ทำงานแยกเป็น FreeRTOS Task อิสระ
  - **URI Dispatcher:** จับคู่ URI + Method ไปยัง Callback Handler อัตโนมัติ
  - **LRU Connection Purge (`config.lru_purge_enable = true`):** ปลดปล่อยการเชื่อมต่อ Socket ที่เก่าที่สุด (Least Recently Used) ทันทีเมื่อ Connection Slots เต็ม เพื่อป้องกันปัญหาหน่วยความจำหมด
  - **Session Persistence:** รองรับ HTTP/1.1 Keep-Alive ในตัว

---

### 3.5 กฎเหล็กการบริหารจัดการหน่วยความจำ cJSON เพื่อป้องกัน Memory Leak

เนื่องจาก ESP32 มีหน่วยความจำแรมจำกัด (เหลือสำหรับ Free Heap ประมาณ 200–250 KB) การใช้งานไลบรารี `cJSON` ต้องปฏิบัติตามหลักวิศวกรรมอย่างเคร่งครัด:

1. **การจองหน่วยความจำแบบไดนามิก (Heap Dynamic Allocation):**
   - คำสั่ง `cJSON_CreateObject()` และ `cJSON_Parse()` ทำการจองหน่วยความจำบน Heap สำหรับทุก Node ต้นไม้ JSON
2. **การคืนค่าหน่วยความจำ (Mandatory Deallocation):**
   - **ต้องเรียก `cJSON_Delete(root)` เสมอ** เพื่อล้างหน่วยความจำทุก Node ของ JSON
   - ข้อความสตริงที่ได้จาก `cJSON_Print()` หรือ `cJSON_PrintUnformatted()` ต้องปลดปล่อยด้วย **`cJSON_free(str)`** หรือ `free(str)` เสมอ
3. **ผลกระทบหากลืมคืนหน่วยความจำ (Memory Leak):**
   - Heap จะลดลงเรื่อยๆ ทุกครั้งที่มี Request เข้ามา
   - นำไปสู่ข้อผิดพลาด `ESP_ERR_NO_MEM` และเกิด Crash (Guru Meditation Error / Panic) ทำให้บอร์ดรีสตาร์ตตัวเอง

---

### 3.6 การยกระดับความปลอดภัยด้วย HTTPS (HTTP over TLS)

- HTTP ส่งข้อมูลแบบ Plaintext เสี่ยงต่อการถูกดักจับและดัดแปลงคำสั่ง (Man-in-the-Middle)
- ESP-IDF รองรับคอมโพเนนต์ **`esp_https_server`** ทำงานร่วมกับไลบรารีเข้ารหัสลับ **MbedTLS**
- เปิดให้บริการบนพอร์ตมาตรฐาน **443** โดยบรรจุ SSL Certificate และ Private Key ลงในเฟิร์มแวร์

---

## 4. บทที่ 3: การสื่อสารระดับซ็อกเก็ต UDP และข้อมูล Telemetry แบบเรียลไทม์

### 4.1 คุณลักษณะเด่นของ UDP (RFC 768)

1. **Connectionless:** ไม่ต้องสร้างการเชื่อมต่อล่วงหน้า (ไม่มี 3-Way Handshake) ส่งข้อมูลได้ทันที
2. **Minimal Header Overhead:** มีขนาดส่วนหัวคงที่เพียง **8 ไบต์** เท่านั้น (Source Port, Destination Port, Length, Checksum)
3. **No Retransmission / Best-Effort Delivery:** ส่งแบบพยายามสูงสุด ไม่มีการรอ ACK, ไม่มีการส่งซ้ำ, ไม่มีการควบคุมความแออัด (No Congestion Control)
4. **รองรับ Broadcast และ Multicast:** สามารถกระจายข้อมูลไปยังหลายโฮสต์พร้อมกันในคำสั่งเดียว

---

### 4.2 ตารางเปรียบเทียบความแตกต่างระหว่าง TCP และ UDP

| มิติการเปรียบเทียบ | Transmission Control Protocol (TCP) | User Datagram Protocol (UDP) |
| :--- | :--- | :--- |
| **ความน่าเชื่อถือ (Reliability)** | **สูงมาก**: รับประกันการส่งถึงตามลำดับ, มี ACK, มี Retransmission | **ไม่รับประกัน**: ข้อมูลอาจสูญหายหรือสลับลำดับได้ (Best-effort) |
| **การเชื่อมต่อ (Connection)** | **Connection-Oriented**: ทำ 3 Handshakes เพื่อเปิด และ 4 Handshakes เพื่อปิด | **Connectionless**: ยิงแพ็กเก็ตออกได้ทันทีโดยไม่ต้องเปิดเซสชัน |
| **รูปแบบการส่ง (Addressing)** | รองรับเฉพาะ **Unicast (1 ต่อ 1)** เท่านั้น | รองรับทั้ง **Unicast, Broadcast, และ Multicast** |
| **ขนาดส่วนหัว (Header)** | **20–60 ไบต์** (20 ไบต์พื้นฐาน + Options) | **8 ไบต์คงที่** |
| **ความเร็วและความหน่วง** | มีความหน่วงสูงกว่าเนื่องจากต้องรอ ACK และจัดการ Buffer | **ความหน่วงต่ำที่สุด (Ultra-low Latency)** ส่งข้อมูลทันที |
| **การใช้พลังงาน** | สูง ต้องเปิดวิทยุ Wi-Fi ค้างไว้เพื่อแลกเปลี่ยนแพ็กเก็ตควบคุม | **ต่ำมาก** ยิงแพ็กเก็ตสั้นๆ เสร็จแล้วปิดวิทยุเข้าสู่ Sleep Mode ได้ทันที |
| **งานที่เหมาะสม** | ถ่ายโอนไฟล์, หน้าเว็บ, การส่งคำสั่งเชิงธุรกิจที่ห้ามตกหล่น | **Real-time Telemetry, สตรีมมิ่งเซนเซอร์, Audio/Video, Device Discovery** |

---

### 4.3 ทำไมงาน IoT และ Local Control ยังจำเป็นต้องใช้ UDP

- **Ultra-Low Latency:** เวลาในการเดินทางลดลงเหลือเพียงเวลาที่คลื่นวิทยุเดินทางจริง (RTT ระดับ 1–3 ms)
- **ประหยัดพลังงานแบตเตอรี่ (Battery Friendly):** ใช้เวลาทำงานของ CPU และวิทยุ Wi-Fi สั้นที่สุด
- **หลีกเลี่ยง Head-of-Line Blocking:** ในงานควบคุมความถี่สูง (เช่น แขนกล หรือโดรน) ข้อมูลเก่าที่ตกหล่นไปไม่มีประโยชน์ที่จะต้องส่งซ้ำ ข้อมูลใหม่จะถูกส่งมาแทนที่ทันที

---

### 4.4 การเขียนโปรแกรม BSD Socket บน LwIP (ESP-IDF)

- **การสร้าง UDP Server รับคำสั่งควบคุม (Unicast):**
  1. สร้าง Socket: `int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);`
  2. เปิดใช้งาน `SO_REUSEADDR`: `setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))`
  3. ผูกพอร์ต: `bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));`
  4. รับข้อมูล: `recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &addr_len);`
  5. ตอบรับกลับ: `sendto(sock, reply, strlen(reply), 0, (struct sockaddr *)&source_addr, addr_len);`

- **การสร้าง UDP Broadcast Telemetry:**
  - กำหนดแอดเดรสปลายทางเป็น `255.255.255.255`
  - เปิดโหมด Broadcast: `setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast))`

---

### 4.5 การจัดการปัญหา Packet Loss ในระดับ Application Layer

เมื่อเลือกใช้ UDP แต่ต้องการความปลอดภัยในการสั่งการ สามารถแก้ไขได้ 2 เทคนิค:

1. **Application-level ACK & Retransmission Timeout:**
   - Client ส่งคำสั่งควบคุม (เช่น `"LED_ON"`) พร้อมจับเวลา Timer (เช่น 1 วินาที)
   - Server ได้รับแล้วต้องตอบกลับ `"STATUS:OK"`
   - หาก Timeout โดยไม่ได้รับข้อความตอบรับ Client จะส่งคำสั่งซ้ำ
2. **Sequence Number กำกับแพ็กเก็ตสตรีมมิ่ง (Telemetry):**
   - แนบหมายเลขลำดับ (Sequence Number) ไว้หน้าข้อมูล เช่น `{"seq": 105, "pot": 2045}`
   - ฝั่งรับสามารถตรวจจับการกระโดดข้ามของตัวเลขเพื่อคำนวณ **Packet Loss Rate (%)** และทิ้งแพ็กเก็ตที่มาถึงสลับลำดับได้ทันที

---

## 5. บทที่ 4: โปรโตคอล CoAP สำหรับระบบสมองกลฝังตัวทรัพยากรจำกัด (RFC 7252)

### 5.1 จุดกำเนิดและแนวคิดของ CoAP

- อุปกรณ์ IoT ส่วนใหญ่เป็น **Resource-Constrained Devices** (แรมไม่กี่ร้อย KB, ใช้แบตเตอรี่, แบนด์วิดท์ต่ำ)
- HTTP/TCP หนักเกินไป ขณะที่ UDP ธรรมดาไม่มีโครงสร้าง REST และไม่มีมาตรฐานระบุทรัพยากร
- IETF จึงพัฒนา **CoAP (Constrained Application Protocol - RFC 7252)**:
  - **"RESTful over UDP"**
  - นำรูปแบบสถาปัตยกรรม REST (GET, POST, PUT, DELETE, URI) มารันบน UDP
  - มีส่วนหัวขนาดเล็กแบบไบนารีเพียง **4 ไบต์**

---

### 5.2 ตารางเปรียบเทียบคุณสมบัติระหว่าง HTTP และ CoAP

| คุณสมบัติ | HTTP (HyperText Transfer Protocol) | CoAP (Constrained Application Protocol) |
| :--- | :--- | :--- |
| **มาตรฐาน** | RFC 7230 / HTTP 1.1 | **RFC 7252** |
| **Transport Layer** | **TCP** (Connection-Oriented) | **UDP** (Connectionless) พอร์ตมาตรฐาน **5683** |
| **ขนาดส่วนหัว (Header)** | **ใหญ่มาก (100–300+ ไบต์)** ข้อความ ASCII | **เล็กมาก (4 ไบต์คงที่)** ข้อมูลเข้ารหัสแบบไบนารี |
| **รูปแบบการเชื่อมต่อ** | Persistent Connection (รักษาเซสชันยาว) | Short-lived Datagram (ส่งเสร็จหลับได้ทันที) |
| **ความน่าเชื่อถือ** | ควบคุมโดย TCP Handshake & ACK | **เลือกได้ในระดับแพ็กเก็ต: CON หรือ NON** |
| **Resource Discovery** | ไม่มีมาตรฐานในตัว ต้องพึ่งพา mDNS/SSDP | **มีในตัวผ่าน URI `/.well-known/core` (RFC 6690)** |
| **การแจ้งเตือนเหตุการณ์** | ต้องทำ Polling ซ้ำๆ หรือใช้ WebSocket | **มีในตัวผ่าน CoAP Observe (RFC 7641)** |
| **ความปลอดภัย** | TLS (HTTPS) พอร์ต 443 | **DTLS (CoAPS) พอร์ต 5684** |

---

### 5.3 โครงสร้างแพ็กเก็ตและส่วนหัวขนาด 4 ไบต์ (4-Byte Fixed Header)

โครงสร้าง CoAP Header ตาม RFC 7252 มีขนาดคงที่ 32 บิต (4 ไบต์) ดังนี้:

```text
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Ver| T |  TKL  |      Code     |          Message ID           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Token (if any, TKL bytes) ...                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Options (if any) ...                                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|1 1 1 1 1 1 1 1|    Payload (if any) ...                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**คำอธิบายฟิลด์ใน Header (ข้อสอบชอบถามความหมายของแต่ละฟิลด์):**
1. **Ver (Version - 2 บิต):** เวอร์ชันของ CoAP ปัจจุบันมีค่าเป็น `1` (`01` ในฐานสอง)
2. **T (Type - 2 บิต):** ชนิดความน่าเชื่อถือของข้อความ (`0` = CON, `1` = NON, `2` = ACK, `3` = RST)
3. **TKL (Token Length - 4 บิต):** ความยาวของ Token (0 ถึง 8 ไบต์) ใช้สำหรับจับคู่ Request กับ Response
4. **Code (8 บิต):** รหัสคำขอหรือรหัสสถานะตอบกลับ แบ่งเป็น Class 3 บิต และ Detail 5 บิต (แสดงในรูป `c.dd`)
5. **Message ID (16 บิต):** หมายเลขกำกับแพ็กเก็ต ใช้ตรวจจับแพ็กเก็ตซ้ำ (Duplicate Detection) และจับคู่ ACK กับ CON
6. **Payload Marker (`0xFF` หรือ `1111 1111`):** ไบต์คั่นบอกตำแหน่งเริ่มต้นของเนื้อหา Payload จริง

---

### 5.4 ประเภทของข้อความ CoAP (Message Types: CON, NON, ACK, RST)

1. **CON (Confirmable - Type 0):**
   - ข้อความที่ **ต้องการการตอบรับ (ACK)**
   - หากผู้รับได้รับ จะต้องส่งแพ็กเก็ต ACK กลับมาโดยใช้ Message ID เดียวกัน
   - หากผู้ส่งไม่ได้รับ ACK ภายในกำหนดเวลา จะส่งซ้ำโดยอัตโนมัติด้วยกลไก **Exponential Backoff**
   - *เหมาะกับ:* คำสั่งควบคุมวิกฤต เช่น สั่งเปิด-ปิดวาล์ว หรือรีเลย์
2. **NON (Non-confirmable - Type 1):**
   - ข้อความที่ **ไม่ต้องการการตอบรับ** (Fire-and-forget)
   - ไม่มีการส่งซ้ำ ทำงานรวดเร็วเหมือน UDP ทั่วไป
   - *เหมาะกับ:* การส่งข้อมูลเซนเซอร์ที่มีการอัปเดตสม่ำเสมอ เช่น ค่าอุณหภูมิส่งทุก 1 วินาที
3. **ACK (Acknowledgement - Type 2):**
   - ข้อความตอบรับยืนยันว่าได้รับแพ็กเก็ต CON แล้ว
4. **RST (Reset - Type 3):**
   - ข้อความปฏิเสธ แจ้งว่าไม่สามารถประมวลผลแพ็กเก็ตนั้นได้ หรือยกเลิกเซสชัน

---

### 5.5 รหัสคำขอและรหัสตอบกลับ (CoAP Request & Response Codes)

- **รหัสคำขอ (Request Method Codes):**
  - `0.01` : **GET** (อ่านค่าทรัพยากร)
  - `0.02` : **POST** (สร้างหรือส่งข้อมูลประมวลผล)
  - `0.03` : **PUT** (ปรับปรุงหรือควบคุมสถานะทรัพยากร)
  - `0.04` : **DELETE** (ลบทรัพยากร)
- **รหัสสถานะตอบกลับ (Response Codes):**
  - `2.04` : **Changed** (สำเร็จ เทียบเท่ากับ HTTP 204 No Content ใช้ตอบกลับเมื่อคำสั่ง PUT สำเร็จ)
  - `2.05` : **Content** (สำเร็จ เทียบเท่ากับ HTTP 200 OK ส่งข้อมูลกลับ)
  - `4.04` : **Not Found** (ไม่พบ Resource ตาม URI ที่ระบุ)
  - `5.00` : **Internal Server Error** (เซิร์ฟเวอร์เกิดข้อผิดพลาดภายใน)

---

### 5.6 กลไกขั้นสูง: CoAP Observe (RFC 7641) และ CoRE Link Format (RFC 6690)

1. **CoAP Observe (RFC 7641):**
   - ปรับเปลี่ยนโมเดล Request/Response ให้ทำงานแบบ **Publish / Subscribe**
   - Client ส่งคำขอ GET พร้อมแนบตัวเลือก Observe ไปยังเซิร์ฟเวอร์เพียง 1 ครั้ง
   - เมื่อสถานะของ Resource เปลี่ยนแปลง เซิร์ฟเวอร์จะ Push ข้อมูลใหม่มาให้ Client ทันที
   - ไม่ต้องให้ Client เสียเวลาทำ Polling ซ้ำๆ ช่วยประหยัดแบนด์วิดท์และพลังงานมหาศาล
2. **CoRE Link Format (RFC 6690):**
   - มาตรฐานการค้นหารายการทรัพยากร (Resource Directory)
   - ไคลเอนต์สามารถส่งคำสั่ง `GET /.well-known/core` เพื่อขอรายชื่อ Endpoints ทั้งหมด
   - ตัวอย่างผลลัพธ์:
     ```text
     </sensor/pot>;rt="sensor";title="Analog Potentiometer",</actuator/led>;rt="actuator";title="LED Control"
     ```
   - **องค์ประกอบไวยากรณ์:**
     - `<path>` : ระบุ Endpoint URI
     - `rt="type"` : ระบุ Resource Type เชิงความหมาย (Semantic)
     - `title="name"` : ชื่อคำอธิบายสำหรับผู้ใช้

---

## 6. บทที่ 5: สรุปสาระสำคัญจากการทดลองปฏิบัติการ (Lab 10.1 - 10.4)

### 6.1 การต่อวงจรฮาร์ดแวร์พื้นฐาน (Hardware Wiring)

การทดลองทุกใบงานใช้การต่อวงจรร่วมกันดังนี้:

| อุปกรณ์ | ขาอุปกรณ์ | ขาเชื่อมต่อ ESP32 | หน้าที่การทำงาน |
| :--- | :--- | :--- | :--- |
| **LED Indicator** | ขั้ว Anode (+) | **GPIO 2** | ตัวแทน Actuator สั่งเปิด-ปิดไฟ (Active High) |
| **Potentiometer 10k** | Wiper (ขากลาง) | **GPIO 34** (ADC1_CH6) | ตัวแทน Sensor แอนะล็อก อ่านค่าแรงดัน 0–3.3V (Raw 0–4095) |
| **Power & Ground** | VCC / GND | **3.3V / GND** | แหล่งจ่ายไฟเลี้ยงอ้างอิงของวงจร |

---

### 6.2 แล็บ 10.1: mDNS Discovery & HTTP REST Server

- **เป้าหมาย:** พัฒนา Web Server ด้วย `esp_http_server` และเปิด mDNS บนชื่อ `esp32-node.local`
- **Dependency ที่ต้องใช้ (IDF Component Manager):**
  - `espressif/mdns`
  - `espressif/cjson`
- **Endpoints ที่พัฒนา:**
  - `GET /api/status` : อ่านค่าสถานะ LED, ค่า Potentiometer, และ Free Heap
  - `POST /api/led` : ควบคุมเปิด-ปิดหลอดไฟ LED ผ่าน JSON Body `{"state": true}` หรือ `{"state": false}`
- **การทดสอบ:** ใช้ `curl.exe` หรือเบราว์เซอร์เรียกผ่าน `http://esp32-node.local/api/status`

---

### 6.3 แล็บ 10.2: UDP Broadcast & Real-time Telemetry

- **เป้าหมาย:** สื่อสารความหน่วงต่ำระดับมิลลิวินาทีด้วย BSD Socket API บน LwIP
- **โครงสร้างพอร์ต:**
  - **พอร์ต 3333 (Unicast Control Server):** รับคำสั่ง `"LED_ON"` / `"LED_OFF"` และส่งคำตอบกลับ `"STATUS:OK"` เพื่อวัดค่า RTT
  - **พอร์ต 3334 (Broadcast Telemetry):** บรอดแคสต์ค่า Potentiometer พร้อม Sequence Number ออกสู่เครือข่ายด้วยความถี่ 10–50 Hz
- **สคริปต์ไคลเอนต์:** สคริปต์ Python ทำหน้าที่ดักฟังพอร์ต 3334 คำนวณ Packet Loss Rate (%) และส่งคำสั่งควบคุมกลับมาที่พอร์ต 3333

---

### 6.4 แล็บ 10.3: CoAP Server & Lightweight IoT Control

- **เป้าหมาย:** สร้าง CoAP Server บนพอร์ตมาตรฐาน UDP 5683 ด้วยคอมโพเนนต์ `libcoap`
- **Dependency & Config:**
  - เพิ่มคอมโพเนนต์ `espressif/coap`
  - เปิดการใช้งาน DTLS Cookie ใน `sdkconfig.defaults` เพื่อป้องกัน Linker Error:
    ```ini
    CONFIG_MBEDTLS_SSL_PROTO_DTLS=y
    CONFIG_MBEDTLS_SSL_COOKIE_C=y
    ```
- **Endpoints ที่พัฒนา:**
  - `GET /.well-known/core` : รายการทรัพยากรแบบ CoRE Link Format
  - `GET /sensor/pot` : อ่านค่าตัวเลข Potentiometer
  - `PUT /actuator/led` : สั่งงาน LED ด้วยเพย์โหลด `1` หรือ `0` ตอบรับด้วยรหัส `2.04 Changed`

---

### 6.5 แล็บ 10.4: ผลการทดสอบเปรียบเทียบสมรรถนะ (Benchmark Summary)

จากการทดสอบจริงด้วยสคริปต์ `benchmark_protocols.py` จำนวน $N = 50$ รอบต่อโปรโตคอล ได้ผลลัพธ์ทางสถิติดังนี้:

#### ตารางผลการทดสอบ RTT และความเสถียร (Latency & Reliability Metrics)

| โปรโตคอลการสื่อสาร | จำนวนรอบสำเร็จ | Packet Loss (%) | Min RTT (ms) | Mean RTT (ms) | Max RTT (ms) | Jitter / StdDev (ms) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **HTTP REST (`/api/led`)** | 50 / 50 | 0.0 % | 18.25 | **26.40** | 45.10 | 5.82 |
| **UDP Socket (`พอร์ต 3333`)** | 50 / 50 | 0.0 % | 1.82 | **2.15** | 3.65 | **0.34** |
| **CoAP PUT (`/actuator/led`)** | 50 / 50 | 0.0 % | 3.10 | **4.20** | 7.80 | 0.85 |

#### ตารางการใช้ทรัพยากรหน่วยความจำแรมบนชิป ESP32 (RAM Footprint)

| สถานะการทำงานของ ESP32 | หน่วยความจำแรมคงเหลือ (Free Heap) | ปริมาณแรมที่ใช้ไปโดยประมาณ |
| :--- | :---: | :---: |
| **Baseline หลังต่อ Wi-Fi สำเร็จ** (ก่อนรันเซิร์ฟเวอร์) | ~ 248 KB | อ้างอิง (0 KB) |
| **HTTP REST Server (`esp_http_server`)** | ~ 216 KB | ~ 32 KB |
| **UDP Socket Server (LwIP Raw Socket)** | **~ 241 KB** | **~ 7 KB (ประหยัดที่สุด)** |
| **CoAP Server (`espressif/coap` / `libcoap`)** | ~ 205 KB | ~ 43 KB |

---

## 7. บทที่ 6: คลังข้อสอบและแนวทางคำตอบเชิงวิเคราะห์ทางวิศวกรรม (Exam Question Bank)

### 7.1 การคำนวณสัดส่วนประสิทธิภาพข้อมูล (Payload Efficiency Ratio)

สูตรคำนวณประสิทธิภาพข้อมูลเพย์โหลด:
$$\text{Payload Efficiency Ratio} = \frac{\text{Payload Size (Bytes)}}{\text{Total Transmitted Bytes}} \times 100\%$$

#### ตารางวิเคราะห์ขนาดเฟรมข้อมูลต่อ 1 คำสั่งควบคุมหลอดไฟ LED

| ชั้นสื่อสารและคุณสมบัติ | HTTP REST (POST) | UDP Socket | CoAP (PUT) |
| :--- | :--- | :--- | :--- |
| **Transport Layer (L4)** | TCP (20–32 bytes) | UDP (8 bytes) | UDP (8 bytes) |
| **Application Header (L7)** | ~ 180–220 bytes (Text) | 0 bytes | 4–8 bytes (Binary) |
| **Command Payload Size** | 14 bytes (`{"state":true}`) | 6 bytes (`LED_ON`) | 1 byte (`1`) |
| **ขนาดแพ็กเก็ตส่งออกรวม** | **~ 220–260 bytes** | **~ 14 bytes** | **~ 13–17 bytes** |
| **Payload Efficiency Ratio (%)** | **~ 5.4 %** | **~ 42.8 %** | **~ 7.7 – 25.0 %** |
| **ปริมาณข้อมูลเมื่อส่ง 10,000 ครั้ง** | **~ 2.5 – 3.0 Megabytes** | **~ 140 Kilobytes** | **~ 130 Kilobytes** |

> **ประเด็นวิเคราะห์สำหรับห้องสอบ:**
> HTTP มีสัดส่วน Header ขนาดใหญ่กว่าข้อมูลจริงหลายสิบเท่าตัว (Overhead มหาศาล) ในขณะที่ CoAP และ UDP ช่วยประหยัดปริมาณข้อมูลในอากาศได้มากกว่า HTTP ถึง **15–20 เท่า** ช่วยยืดอายุแบตเตอรี่และลดค่าบริการซิมการ์ด IoT ได้อย่างมีนัยสำคัญ

---

### 7.2 การเปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix)

#### 1. HTTP (HyperText Transfer Protocol)
- **จุดเด่น (Pros):**
  - มีความเป็นสากลสูงสุด ทุกแพลตฟอร์มและเว็บบราวเซอร์รองรับโดยกำเนิด
  - การใช้งานร่วมกับ JSON และ RESTful API ทำให้อ่านเข้าใจง่าย พัฒนาและดีบั๊กสะดวกรวดเร็ว
  - มีมาตรฐานความปลอดภัยเข้มงวดด้วย HTTPS/TLS
- **จุดด้อย (Cons):**
  - ภาระส่วนหัว (Overhead) สูงมาก เปลืองแบนด์วิดท์
  - Latency สูงจากกระบวนการ TCP Handshake
  - สิ้นเปลืองพลังงานและแรมบนไมโครคอนโทรลเลอร์สูง ไม่เหมาะกับอุปกรณ์แบตเตอรี่

#### 2. UDP (User Datagram Protocol)
- **จุดเด่น (Pros):**
  - ความหน่วงต่ำที่สุดในระดับ Real-time (< 3 ms)
  - ความแปรปรวน (Jitter) ต่ำมาก ตอบสนองสม่ำเสมอ
  - ใช้หน่วยความจำแรมน้อยที่สุด (~ 7 KB)
  - รองรับการกระจายข้อมูลแบบ Broadcast และ Multicast ไปยังหลายโฮสต์พร้อมกันในแพ็กเก็ตเดียว
- **จุดด้อย (Cons):**
  - ไม่รับประกันการส่งถึง ไม่มีกลไก ACK หรือ Retransmission ในตัว ข้อมูลอาจสูญหายหรือสลับลำดับได้
  - ไม่มีมาตรฐานจัดการทรัพยากรในระดับ Application Layer ผู้พัฒนาต้องออกแบบโปรโตคอลเองทั้งหมด

#### 3. CoAP (Constrained Application Protocol - RFC 7252)
- **จุดเด่น (Pros):**
  - ผสานสถาปัตยกรรม RESTful (GET, PUT, POST, DELETE) เข้ากับความเร็วและเบาของ UDP
  - ส่วนหัวกะทัดรัดคงที่เพียง 4 ไบต์ ประหยัดแบนด์วิดท์สูงสุด
  - มีความยืดหยุ่น สามารถเลือกได้ว่าจะส่งแบบเชื่อถือได้ (CON มี ACK/ส่งซ้ำ) หรือส่งแบบรวดเร็ว (NON)
  - มีระบบค้นหาทรัพยากร (`/.well-known/core`) และระบบแจ้งเตือนแบบ Pub/Sub (CoAP Observe) ในตัว
  - ประหยัดพลังงานแบตเตอรี่สูงสุด เหมาะกับเครือข่ายเซนเซอร์ LLNs
- **จุดด้อย (Cons):**
  - เว็บบราวเซอร์ทั่วไปไม่สามารถเชื่อมต่อได้โดยตรง ต้องอาศัย Reverse Proxy แปลงเป็น HTTP
  - ซอฟต์แวร์สแตกและเครื่องมือทดสอบบนฝั่งคอมพิวเตอร์ยังมีน้อยกว่า HTTP

---

### 7.3 กรณีศึกษาโจทย์วิเคราะห์การตัดสินใจเลือกโปรโตคอล (Case Studies)

#### กรณีศึกษาที่ 1:
> **โจทย์:** ระบบเซนเซอร์วัดความชื้นในดินและวาล์วน้ำเพื่อการเกษตรอัจฉริยะ ทำงานด้วยแบตเตอรี่โซลาร์เซลล์ขนาดเล็ก ติดตั้งกลางแจ้งห่างไกล
- **โปรโตคอลที่เลือก:** **CoAP (ร่วมกับเครือข่าย NB-IoT หรือ Thread / 6LoWPAN)**
- **เหตุผลทางวิศวกรรม:**
  1. โหนดใช้พลังงานแบตเตอรี่และโซลาร์เซลล์ขนาดเล็ก ต้องการโหมดหลับลึก (Deep Sleep) ให้เร็วที่สุด CoAP ทำงานบน UDP ไม่มีภาระ TCP Handshake ตื่นมายิง 1 แพ็กเก็ตแล้วหลับได้ทันทีภายในไม่กี่มิลลิวินาที
  2. ขนาด Header เพียง 4 ไบต์ ประหยัดค่าบริการรับส่งข้อมูลผ่านซิมเซลลูลาร์ NB-IoT ได้มากกว่า HTTP ถึง 20 เท่า
  3. คำสั่งเปิด-ปิดวาล์วน้ำมีความสำคัญสูง สามารถเลือกส่งแพ็กเก็ตแบบ **CON (Confirmable)** เพื่อให้มั่นใจว่าวาล์วได้รับคำสั่งจริงและส่งซ้ำหากแพ็กเก็ตตกหล่น

#### กรณีศึกษาที่ 2:
> **โจทย์:** เว็บแดชบอร์ดสำหรับฝ่ายซ่อมบำรุงในโรงงาน เพื่อเปิดดูสถานะและตั้งค่าพารามิเตอร์ของอุปกรณ์ผ่านเว็บบราวเซอร์บนแท็บเล็ต/สมาร์ตโฟน
- **โปรโตคอลที่เลือก:** **HTTP RESTful API (ร่วมกับ mDNS Service Discovery)**
- **เหตุผลทางวิศวกรรม:**
  1. ไคลเอนต์คือเว็บบราวเซอร์ (Chrome, Edge, Safari) บนมือถือ ซึ่งรองรับ HTTP และ JSON โดยตรง ไม่ต้องติดตั้งแอปพลิเคชันเสริม
  2. การใช้ **mDNS** ช่วยให้ช่างพิมพ์ชื่อ Hostname จำง่าย เช่น `http://machine-01.local` บน URL bar เพื่อเข้าหน้าเว็บได้ทันทีโดยไม่ต้องจำหมายเลข IP
  3. สถาปัตยกรรม RESTful API ยืดหยุ่น รองรับการดูค่าสถานะ (`GET`) และการส่งค่าคอนฟิกใหม่ (`POST`) ผ่านหน้าต่างเว็บได้อย่างปลอดภัย

#### กรณีศึกษาที่ 3:
> **โจทย์:** ระบบควบคุมแขนกลอุตสาหกรรมความเร็วสูง ที่ต้องการส่งพิกัดตำแหน่งแกนหมุน (X, Y, Z) ด้วยความถี่ 100 ครั้งต่อวินาที (100 Hz) ภายในเครือข่าย LAN ปิด (intranet)
- **โปรโตคอลที่เลือก:** **UDP Socket (Unicast หรือ Multicast)**
- **เหตุผลทางวิศวกรรม:**
  1. ความถี่ 100 Hz มีรอบการส่งข้อมูลทุกๆ 10 ms ต้องการ Ultra-low Latency (< 2 ms) และค่า Jitter ต่ำสุด ซึ่ง UDP ให้ความเร็วสูงและไม่มีภาระการรอ ACK
  2. ข้อมูลพิกัดตำแหน่งเป็น Temporal Data หากข้อมูลชุดใดตกหล่นไป ไม่มีความจำเป็นต้องส่งซ้ำ (No Retransmission) ข้อมูลพิกัดใหม่จะตามมาในอีก 10 ms หากใช้ TCP จะเกิดปัญหา **Head-of-Line Blocking** ทำให้แขนกลกระตุกอย่างรุนแรง
  3. ในเครือข่าย LAN ปิด มีสัญญาณรบกวนต่ำและไม่มีปัญหาความแออัด อัตรา Packet Loss ต่ำมาก UDP จึงเป็นตัวเลือกที่ดีที่สุด

---

### 7.4 คำถาม-คำตอบเจาะลึกที่พบบ่อยในห้องสอบ

#### คำถามที่ 1: หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือถูกปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?
- **ผลกระทบ:** คอมพิวเตอร์จะไม่สามารถแปลงชื่อโดเมน `esp32-node.local` เป็น IP ได้ เมื่อพิมพ์ ping หรือเข้าเว็บบราวเซอร์จะฟ้องข้อผิดพลาด `ERR_NAME_NOT_RESOLVED`
- **แนวทางแก้ไข:**
  1. เข้าใช้งานด้วยหมายเลข IP Address ตรงๆ (Direct IP เช่น `http://192.168.1.181`)
  2. ล็อกหมายเลข IP ของ ESP32 บนเราเตอร์ด้วย Static DHCP Reservation
  3. กำหนด Local DNS บนเราเตอร์หรือเซิร์ฟเวอร์ DNS ภายใน (dnsmasq / Pi-hole)
  4. เพิ่มคู่ IP และชื่อโฮสต์ลงในไฟล์ `hosts` ของระบบปฏิบัติการ (`C:\Windows\System32\drivers\etc\hosts`)
  5. ปลดล็อก Firewall ของคอมพิวเตอร์ให้เปิดรับพอร์ต UDP 5353 และตั้งค่าเครือข่ายเป็น Private Network

#### คำถามที่ 2: เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?
- **คำตอบ:** ฟังก์ชันตระกูล `cJSON_Create...()` และ `cJSON_Parse()` จะจองหน่วยความจำแบบไดนามิกบนฮีป (Heap) สำหรับทุก Node โครงสร้าง หากไม่คืนหน่วยความจำ ทุกๆ HTTP Request ที่ส่งเข้ามาจะทำให้ Free Heap ลดลงเรื่อยๆ จนเกิดปัญหา **Memory Leak** และในที่สุดแรมจะหมด (`ESP_ERR_NO_MEM`) ทำให้ไมโครคอนโทรลเลอร์เกิด Panic / Guru Meditation Crash และรีสตาร์ตตัวเอง

#### คำถามที่ 3: ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread (IEEE 802.15.4) หรือ NB-IoT มากกว่า HTTP?
- **คำตอบ:**
  1. **ขนาด Header เล็กมาก:** CoAP มีขนาดส่วนหัวคงที่เพียง 4 ไบต์ ขณะที่เฟรมของ Thread (IEEE 802.15.4 / 6LoWPAN) มีขนาด MTU จำกัดเพียง 127 ไบต์ หากใช้ HTTP Header (100–300 ไบต์) แพ็กเก็ตจะถูกแตกย่อยหลายทอด (Fragmentation) เปลืองพลังงานและล้มเหลวง่าย แต่ CoAP บรรจุลงใน 1 เฟรมได้สบาย
  2. **ทำงานบน UDP (Connectionless):** อุปกรณ์บนเครือข่าย NB-IoT ใช้แบตเตอรี่และต้องเข้า Deep Sleep ให้เร็วที่สุด CoAP ยิงแพ็กเก็ตเสร็จแล้วหลับได้ทันที ไม่ต้องตื่นมารักษาการเชื่อมต่อแบบ TCP Handshake
  3. **ฟังก์ชัน CoAP Observe (RFC 7641):** สามารถแจ้งเตือนข้อมูลแบบ Push ได้โดยไม่ต้องทำ Polling ซ้ำๆ ซึ่งผลาญแบนด์วิดท์ของเครือข่ายแคบ
  4. **ความปลอดภัยที่เบากว่า:** ใช้ DTLS ร่วมกับ PSK/ECC ที่กินทรัพยากรน้อยกว่า TLS ของ HTTP

---

## 8. บทที่ 7: อภิธานศัพท์ คำย่อ และตัวเลขสำคัญที่ต้องจำเข้าห้องสอบ (Cheat Sheet)

### หมายเลขพอร์ตมาตรฐานที่ต้องจำ

| โปรโตคอล | Transport Layer | หมายเลขพอร์ต | วัตถุประสงค์การใช้งาน |
| :--- | :---: | :---: | :--- |
| **HTTP** | TCP | **80** | เว็บเซิร์ฟเวอร์ RESTful API ทั่วไป |
| **HTTPS** | TCP | **443** | เว็บเซิร์ฟเวอร์ความปลอดภัยสูง (HTTP over TLS) |
| **CoAP** | UDP | **5683** | โปรโตคอล CoAP มาตรฐาน (RFC 7252) |
| **CoAPS** | UDP | **5684** | โปรโตคอล CoAP เข้ารหัสความปลอดภัย (CoAP over DTLS) |
| **mDNS** | UDP | **5353** | Multicast DNS Service Discovery (RFC 6762) |
| **UDP Lab (Control)** | UDP | **3333** | พอร์ตรับคำสั่งควบคุม LED Unicast ในแล็บ 10.2 |
| **UDP Lab (Telemetry)**| UDP | **3334** | พอร์ตบรอดแคสต์ข้อมูลเซนเซอร์ในแล็บ 10.2 |

---

### หมายเลขไอพีพิเศษที่ต้องจำ

- **Limited Broadcast Address:** `255.255.255.255`
- **mDNS Multicast Address (IPv4):** `224.0.0.251`
- **mDNS Multicast Address (IPv6):** `ff02::fb`
- **ช่วงไอพีแอดเดรส Multicast (Class D):** `224.0.0.0` ถึง `239.255.255.255`

---

### สรุปขนาดส่วนหัว (Header Sizes)

- **TCP Header:** ขั้นต่ำ **20 ไบต์** (สูงสุด 60 ไบต์เมื่อมี Options)
- **UDP Header:** คงที่ **8 ไบต์** (Source Port, Dest Port, Length, Checksum)
- **CoAP Header:** คงที่ **4 ไบต์** (Ver, Type, TKL, Code, Message ID)
- **HTTP Header:** ข้อความ ASCII ขนาดประมาณ **100 – 300+ ไบต์**

---

### อภิธานศัพท์สำคัญ (Key Terms)

- **mDNS (Multicast DNS - RFC 6762):** ระบบแปลงชื่อโฮสต์ `.local` เป็น IP ภายในวงแลนโดยไม่ต้องมีเครื่องแม่ข่าย DNS
- **DNS-SD (DNS Service Discovery - RFC 6763):** กลไกการประกาศและค้นหาประเภทบริการเครือข่าย เช่น `_http._tcp`
- **LwIP (Lightweight IP):** ซอฟต์แวร์สแตกโปรโตคอล TCP/IP ขนาดกะทัดรัดสำหรับระบบสมองกลฝังตัวใน ESP-IDF
- **BSD Socket API:** มาตรฐานฟังก์ชันเชื่อมต่อเครือข่ายระดับล่าง (`socket`, `bind`, `sendto`, `recvfrom`)
- **cJSON:** ไลบรารีภาษา C ใน ESP-IDF สำหรับการสร้างและพาร์สข้อมูล JSON
- **CON (Confirmable):** แพ็กเก็ต CoAP ที่ต้องการการตอบรับ (ACK) และมีการส่งซ้ำเมื่อสูญหาย
- **NON (Non-confirmable):** แพ็กเก็ต CoAP แบบส่งทิ้ง (Fire-and-forget) ไม่ต้องการการตอบรับ
- **CoAP Observe (RFC 7641):** กลไกการบอกรับข้อมูลการเปลี่ยนแปลงของทรัพยากรแบบ Publish/Subscribe
- **CoRE Link Format (RFC 6690):** รูปแบบข้อความแสดงรายชื่อทรัพยากรเมื่อร้องขอผ่าน `/.well-known/core`
- **RTT (Round-Trip Time):** เวลาไป-กลับของแพ็กเก็ตข้อมูลตั้งแต่ผู้ส่งยิงคำสั่งจนได้รับคำตอบรับ
- **Jitter:** ค่าเบี่ยงเบนมาตรฐาน ($\sigma$) ของเวลาตอบสนอง สะท้อนถึงความสม่ำเสมอและเสถียรภาพของเครือข่าย
