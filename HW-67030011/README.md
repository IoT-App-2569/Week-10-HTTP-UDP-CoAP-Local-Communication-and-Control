# การส่งงานสัปดาห์ที่ 10: HTTP / UDP / CoAP Local Communication & Control

* **รหัสนักศึกษา:** 67030011
* **Git Branch:** `HW-67030011`
* **สภาพแวดล้อม:** ESP-IDF v6.0.2 (target `esp32`), Python 3 + `aiocoap`

---

## สารบัญงานและรายงานผลการทดลอง

### 1. ใบงาน 10.1 — mDNS Discovery & HTTP REST Server
- [06-Labsheet-10-1-mDNS-Discovery-and-HTTP-REST-Server.md](../06-Labsheet-10-1-mDNS-Discovery-and-HTTP-REST-Server.md)
- `esp_http_server` + `espressif/mdns` (`esp32-node.local`) + `espressif/cjson`, เพิ่ม `student_id` ใน `/api/status`
- วิเคราะห์ขนาด Response Header ของ `esp_http_server` (71 ไบต์) จากซอร์ส `httpd_txrx.c` และตอบคำถามครบ 3 ข้อ

### 2. ใบงาน 10.2 — UDP Broadcast & Realtime Telemetry
- [07-Labsheet-10-2-UDP-Broadcast-and-Realtime-Telemetry.md](../07-Labsheet-10-2-UDP-Broadcast-and-Realtime-Telemetry.md)
- UDP Control Server พอร์ต 3333 + Telemetry Broadcast 10 Hz พอร์ต 3334, ตอบคำถามครบ 3 ข้อ

### 3. ใบงาน 10.3 — CoAP Server
- [08-Labsheet-10-3-CoAP-Server-and-Lightweight-IoT-Control.md](../08-Labsheet-10-3-CoAP-Server-and-Lightweight-IoT-Control.md)
- `libcoap` (`espressif/coap`) + แอตทริบิวต์ CoRE Link Format (`ct`, `rt`, `if`) ให้ `/.well-known/core`, ถอดรหัสแพ็กเก็ต PUT ทีละไบต์, ตอบคำถามครบ 3 ข้อ

### 4. ใบงาน 10.4 — Protocol Benchmark & Network Forensics
- [09-Labsheet-10-4-Protocol-Benchmark-and-Network-Forensics.md](../09-Labsheet-10-4-Protocol-Benchmark-and-Network-Forensics.md)
- ตาราง Byte Breakdown จากแพ็กเก็ตจริง (HTTP 172/213, UDP 14, CoAP 29 ไบต์), ตารางหน่วยความจำจาก `idf.py size`, Pros & Cons และ Case Study ครบ 3 กรณี

---

## รายการซอร์สโค้ด

| ใบงาน | โปรเจกต์ | ไฟล์หลัก |
| :--- | :--- | :--- |
| 10.1 | [Lab10-1_HTTP_REST_Server/](Lab10-1_HTTP_REST_Server/) | [main/Lab10-1_HTTP_REST_Server.c](Lab10-1_HTTP_REST_Server/main/Lab10-1_HTTP_REST_Server.c) |
| 10.2 | [Lab10-2_UDP_Telemetry_Socket/](Lab10-2_UDP_Telemetry_Socket/) | [main/Lab10-2_UDP_Telemetry_Socket.c](Lab10-2_UDP_Telemetry_Socket/main/Lab10-2_UDP_Telemetry_Socket.c), `udp_listener.py`, `udp_controller.py` |
| 10.3 | [Lab10-3_CoAP_Server/](Lab10-3_CoAP_Server/) | [main/Lab10-3_CoAP_Server.c](Lab10-3_CoAP_Server/main/Lab10-3_CoAP_Server.c), `test_coap.py`, `benchmark_protocols.py` |

### วิธี Build / Flash
```bash
cd HW-67030011/Lab10-1_HTTP_REST_Server
cp main/wifi_credentials.h.example main/wifi_credentials.h   # แล้วใส่ SSID / Password (Wi-Fi 2.4 GHz)
idf.py set-target esp32
idf.py -p <PORT> flash monitor
```
> `main/wifi_credentials.h` อยู่ใน `.gitignore` เพื่อไม่ให้รหัส Wi-Fi ขึ้น GitHub
