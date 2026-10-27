# ใบงานการทดลองที่ 10.4
### การทดสอบเปรียบเทียบสมรรถนะ (Benchmark) ของโปรโตคอลเครือข่ายด้วย Python Script และสถิติเชิงวิศวกรรม

---

## 1. บันทึกผลการทดลอง

### กิจกรรมที่ 10-4.1 การวิเคราะห์สัดส่วนข้อมูลตามมาตรฐานสากล (Architectural Byte Breakdown)

#### ตารางบันทึกการคำนวณสัดส่วนประสิทธิภาพข้อมูล (Payload Efficiency)

| รายการประเมิน | HTTP REST (POST) | UDP Datagram | CoAP (PUT) |
| :--- | :---: | :---: | :---: |
| **ชั้นสื่อสาร L4 Transport** | TCP | UDP | UDP |
| **ขนาด L4 Header** | 20–32 ไบต์ | **8 ไบต์** | **8 ไบต์** |
| **ขนาด L7 Application Header** | ~180–250 ไบต์ | **0 ไบต์** | **4 ไบต์** |
| **ขนาดข้อมูลจริง (Payload)** | 14 ไบต์ (`{"state":true}`) | 6 ไบต์ (`LED_ON`) | 1 ไบต์ (`1`) |
| **ขนาดข้อมูลรวม (L4 + L7 + Payload)** | ~214–296 ไบต์ | **14 ไบต์** | **13 ไบต์** |
| **สัดส่วน Payload ต่อข้อมูลรวม (%)** | **~4.7 % - 6.5 %** | **~42.8 %** | **~7.6 %** |
| **ปริมาณข้อมูลเมื่อส่ง 10,000 ครั้ง** | ~2.5 - 3.0 Megabytes | ~140 Kilobytes | ~130 Kilobytes |

---

### กิจกรรมที่ 10-4.2 การทดสอบวัดความหน่วงเวลาจริง (RTT Latency & Jitter Benchmarking)

- **คำสั่งรันสคริปต์ Benchmark:**
  ```powershell
  python benchmark_protocols.py --target <IP_ESP32> --rounds 50
  ```

#### ตารางบันทึกผลการทดสอบ RTT จริง (จำนวน 50 รอบ)

| โปรโตคอล | จำนวนรอบที่สำเร็จ | Packet Loss (%) | Min RTT (ms) | Mean RTT (ms) | Max RTT (ms) | Jitter / StdDev (ms) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **HTTP REST (`/api/led`)** | 49 / 50 | 2.0 % | 68.20 ms | 314.45 ms | 1710.42 ms | 462.10 ms |
| **UDP Socket (`พอร์ต 3333`)** | 50 / 50 | 0.0 % | 2.86 ms | 16.04 ms | 102.89 ms | 18.31 ms |
| **CoAP PUT (`/actuator/led`)** | 50 / 50 | 0.0 % | 7.32 ms | 21.16 ms | 384.03 ms | 52.03 ms |

- **ผลการทดสอบ Benchmark:**
```text
Logs : HTTP REST Server (esp_http_server)
PS C:\Users\tulbp\Desktop\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control\67030298\Lab10-4_Protocol_Benchmark> python benchmark_protocols.py --target 192.168.1.8 --protocol http --rounds 50

================================================================================
   ESP32 Local Protocol Benchmark: HTTP vs UDP vs CoAP
   Industrial IoT Systems Engineering - Real-time Performance Lab
================================================================================


[*] Starting HTTP REST Benchmark -> http://192.168.1.8/api/led (50 rounds)...
  [HTTP Round 001/050] FAILED: <urlopen error timed out>
  [HTTP Round 002/050] RTT: 1444.37 ms | Status: 200
  [HTTP Round 003/050] RTT: 164.37 ms | Status: 200
  [HTTP Round 004/050] RTT: 113.25 ms | Status: 200
  [HTTP Round 005/050] RTT: 1160.22 ms | Status: 200
  [HTTP Round 006/050] RTT: 149.29 ms | Status: 200
  [HTTP Round 007/050] RTT: 1223.27 ms | Status: 200
  [HTTP Round 008/050] RTT: 141.06 ms | Status: 200
  [HTTP Round 009/050] RTT: 185.54 ms | Status: 200
  [HTTP Round 010/050] RTT:  92.09 ms | Status: 200
  [HTTP Round 011/050] RTT:  82.68 ms | Status: 200
  [HTTP Round 012/050] RTT: 177.11 ms | Status: 200
  [HTTP Round 013/050] RTT:  92.13 ms | Status: 200
  [HTTP Round 014/050] RTT: 262.81 ms | Status: 200
  [HTTP Round 015/050] RTT:  92.51 ms | Status: 200
  [HTTP Round 016/050] RTT:  86.50 ms | Status: 200
  [HTTP Round 017/050] RTT: 1509.80 ms | Status: 200
  [HTTP Round 018/050] RTT: 1710.42 ms | Status: 200
  [HTTP Round 019/050] RTT:  89.60 ms | Status: 200
  [HTTP Round 020/050] RTT: 173.17 ms | Status: 200
  [HTTP Round 021/050] RTT: 103.63 ms | Status: 200
  [HTTP Round 022/050] RTT:  68.62 ms | Status: 200
  [HTTP Round 023/050] RTT:  89.14 ms | Status: 200
  [HTTP Round 024/050] RTT: 1137.81 ms | Status: 200
  [HTTP Round 025/050] RTT:  91.09 ms | Status: 200
  [HTTP Round 026/050] RTT:  75.75 ms | Status: 200
  [HTTP Round 027/050] RTT:  89.26 ms | Status: 200
  [HTTP Round 028/050] RTT:  86.40 ms | Status: 200
  [HTTP Round 029/050] RTT:  88.57 ms | Status: 200
  [HTTP Round 030/050] RTT:  87.20 ms | Status: 200
  [HTTP Round 031/050] RTT: 1130.68 ms | Status: 200
  [HTTP Round 032/050] RTT:  82.15 ms | Status: 200
  [HTTP Round 033/050] RTT:  86.12 ms | Status: 200
  [HTTP Round 034/050] RTT:  85.74 ms | Status: 200
  [HTTP Round 035/050] RTT:  89.34 ms | Status: 200
  [HTTP Round 036/050] RTT:  91.49 ms | Status: 200
  [HTTP Round 037/050] RTT:  83.38 ms | Status: 200
  [HTTP Round 038/050] RTT:  91.13 ms | Status: 200
  [HTTP Round 039/050] RTT:  85.80 ms | Status: 200
  [HTTP Round 040/050] RTT: 102.85 ms | Status: 200
  [HTTP Round 041/050] RTT:  88.35 ms | Status: 200
  [HTTP Round 042/050] RTT:  99.34 ms | Status: 200
  [HTTP Round 043/050] RTT:  89.90 ms | Status: 200
  [HTTP Round 044/050] RTT: 1353.38 ms | Status: 200
  [HTTP Round 045/050] RTT: 149.97 ms | Status: 200
  [HTTP Round 046/050] RTT:  68.20 ms | Status: 200
  [HTTP Round 047/050] RTT:  73.39 ms | Status: 200
  [HTTP Round 048/050] RTT:  89.17 ms | Status: 200
  [HTTP Round 049/050] RTT: 511.92 ms | Status: 200
  [HTTP Round 050/050] RTT:  88.27 ms | Status: 200

================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
HTTP (REST)  | 49/50      |   2.0% |    68.20 |    314.45 |  1710.42 |     462.10
================================================================================

================================================================================
             THEORETICAL PROTOCOL OVERHEAD ANALYSIS (Per LED Command)
================================================================================
Layer / Attribute              | HTTP REST (POST)  | UDP Socket    | CoAP (PUT)
--------------------------------------------------------------------------------
Transport Layer (L4)           | TCP (20-32 bytes) | UDP (8 bytes) | UDP (8 bytes)
Application Header (L7)        | ~150-250 bytes    | 0 bytes       | 4-8 bytes
Command Payload Size           | 14 bytes (JSON)   | 6 bytes (ASCII) | 1 byte (char)
Est. Total Bytes per Request   | ~190-300 bytes    | ~14 bytes     | ~13-17 bytes
Payload Efficiency Ratio       | ~4.5 - 7.0 %      | ~42.8 %       | ~7.7 - 25.0 %
================================================================================

[✔] Benchmark Complete! Copy the metrics above into your Lab 10.4 Report.
```

```text
Logs : UDP Socket (พอร์ต 3333)
PS C:\Users\tulbp\Desktop\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control\67030298\Lab10-4_Protocol_Benchmark> python benchmark_protocols.py --target 192.168.1.8 --protocol udp --rounds 50

================================================================================
   ESP32 Local Protocol Benchmark: HTTP vs UDP vs CoAP
   Industrial IoT Systems Engineering - Real-time Performance Lab
================================================================================


[*] Starting UDP Socket Benchmark -> 192.168.1.8:3333 (50 rounds)...
  [UDP Round  001/050] RTT:  88.69 ms | Recv: ACK:LED_ON
  [UDP Round  002/050] RTT:  15.39 ms | Recv: ACK:LED_OFF
  [UDP Round  003/050] RTT:  14.18 ms | Recv: ACK:LED_ON
  [UDP Round  004/050] RTT:  20.36 ms | Recv: ACK:LED_OFF
  [UDP Round  005/050] RTT:  29.17 ms | Recv: ACK:LED_ON
  [UDP Round  006/050] RTT:  13.89 ms | Recv: ACK:LED_OFF
  [UDP Round  007/050] RTT:  18.40 ms | Recv: ACK:LED_ON
  [UDP Round  008/050] RTT:  29.29 ms | Recv: ACK:LED_OFF
  [UDP Round  009/050] RTT:  12.29 ms | Recv: ACK:LED_ON
  [UDP Round  010/050] RTT:  24.65 ms | Recv: ACK:LED_OFF
  [UDP Round  011/050] RTT:  10.84 ms | Recv: ACK:LED_ON
  [UDP Round  012/050] RTT:  12.41 ms | Recv: ACK:LED_OFF
  [UDP Round  013/050] RTT:  14.66 ms | Recv: ACK:LED_ON
  [UDP Round  014/050] RTT:  15.18 ms | Recv: ACK:LED_OFF
  [UDP Round  015/050] RTT:  29.16 ms | Recv: ACK:LED_ON
  [UDP Round  016/050] RTT:   9.96 ms | Recv: ACK:LED_OFF
  [UDP Round  017/050] RTT:  19.98 ms | Recv: ACK:LED_ON
  [UDP Round  018/050] RTT:   7.64 ms | Recv: ACK:LED_OFF
  [UDP Round  019/050] RTT:  11.49 ms | Recv: ACK:LED_ON
  [UDP Round  020/050] RTT:  26.86 ms | Recv: ACK:LED_OFF
  [UDP Round  021/050] RTT:  10.04 ms | Recv: ACK:LED_ON
  [UDP Round  022/050] RTT:  11.65 ms | Recv: ACK:LED_OFF
  [UDP Round  023/050] RTT:  12.02 ms | Recv: ACK:LED_ON
  [UDP Round  024/050] RTT:  42.11 ms | Recv: ACK:LED_OFF
  [UDP Round  025/050] RTT:   7.87 ms | Recv: ACK:LED_ON
  [UDP Round  026/050] RTT:  11.71 ms | Recv: ACK:LED_OFF
  [UDP Round  027/050] RTT:   9.45 ms | Recv: ACK:LED_ON
  [UDP Round  028/050] RTT:  10.00 ms | Recv: ACK:LED_OFF
  [UDP Round  029/050] RTT:   7.82 ms | Recv: ACK:LED_ON
  [UDP Round  030/050] RTT:  10.83 ms | Recv: ACK:LED_OFF
  [UDP Round  031/050] RTT:   5.54 ms | Recv: ACK:LED_ON
  [UDP Round  032/050] RTT:  16.68 ms | Recv: ACK:LED_OFF
  [UDP Round  033/050] RTT:   9.55 ms | Recv: ACK:LED_ON
  [UDP Round  034/050] RTT:   7.76 ms | Recv: ACK:LED_OFF
  [UDP Round  035/050] RTT:   6.28 ms | Recv: ACK:LED_ON
  [UDP Round  036/050] RTT:   6.14 ms | Recv: ACK:LED_OFF
  [UDP Round  037/050] RTT: 102.89 ms | Recv: ACK:LED_ON
  [UDP Round  038/050] RTT:   3.35 ms | Recv: ACK:LED_OFF
  [UDP Round  039/050] RTT:  12.61 ms | Recv: ACK:LED_ON
  [UDP Round  040/050] RTT:   3.83 ms | Recv: ACK:LED_OFF
  [UDP Round  041/050] RTT:  25.74 ms | Recv: ACK:LED_ON
  [UDP Round  042/050] RTT:   4.47 ms | Recv: ACK:LED_OFF
  [UDP Round  043/050] RTT:   6.33 ms | Recv: ACK:LED_ON
  [UDP Round  044/050] RTT:   4.12 ms | Recv: ACK:LED_OFF
  [UDP Round  045/050] RTT:   2.86 ms | Recv: ACK:LED_ON
  [UDP Round  046/050] RTT:   5.97 ms | Recv: ACK:LED_OFF
  [UDP Round  047/050] RTT:   6.58 ms | Recv: ACK:LED_ON
  [UDP Round  048/050] RTT:   6.82 ms | Recv: ACK:LED_OFF
  [UDP Round  049/050] RTT:   3.35 ms | Recv: ACK:LED_ON
  [UDP Round  050/050] RTT:   3.12 ms | Recv: ACK:LED_OFF

================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
UDP (Socket) | 50/50      |   0.0% |     2.86 |     16.04 |   102.89 |      18.31
================================================================================

================================================================================
             THEORETICAL PROTOCOL OVERHEAD ANALYSIS (Per LED Command)
================================================================================
Layer / Attribute              | HTTP REST (POST)  | UDP Socket    | CoAP (PUT)
--------------------------------------------------------------------------------
Transport Layer (L4)           | TCP (20-32 bytes) | UDP (8 bytes) | UDP (8 bytes)
Application Header (L7)        | ~150-250 bytes    | 0 bytes       | 4-8 bytes
Command Payload Size           | 14 bytes (JSON)   | 6 bytes (ASCII) | 1 byte (char)
Est. Total Bytes per Request   | ~190-300 bytes    | ~14 bytes     | ~13-17 bytes
Payload Efficiency Ratio       | ~4.5 - 7.0 %      | ~42.8 %       | ~7.7 - 25.0 %
================================================================================

[✔] Benchmark Complete! Copy the metrics above into your Lab 10.4 Report.
```

```text
Logs : CoAP PUT (/actuator/led)
PS C:\Users\tulbp\Desktop\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control\67030298\Lab10-4_Protocol_Benchmark> python benchmark_protocols.py --target 192.168.1.8 --protocol coap --rounds 50

================================================================================
   ESP32 Local Protocol Benchmark: HTTP vs UDP vs CoAP
   Industrial IoT Systems Engineering - Real-time Performance Lab
================================================================================


[*] Starting CoAP Benchmark -> coap://192.168.1.8:5683/actuator/led (50 rounds)...
  [CoAP Round 001/050] RTT: 384.03 ms | Code: 2.04 Changed
  [CoAP Round 002/050] RTT:  24.49 ms | Code: 2.04 Changed
  [CoAP Round 003/050] RTT:  19.68 ms | Code: 2.04 Changed
  [CoAP Round 004/050] RTT:  22.14 ms | Code: 2.04 Changed
  [CoAP Round 005/050] RTT:  24.16 ms | Code: 2.04 Changed
  [CoAP Round 006/050] RTT:  16.49 ms | Code: 2.04 Changed
  [CoAP Round 007/050] RTT:  16.89 ms | Code: 2.04 Changed
  [CoAP Round 008/050] RTT:  26.45 ms | Code: 2.04 Changed
  [CoAP Round 009/050] RTT:  14.80 ms | Code: 2.04 Changed
  [CoAP Round 010/050] RTT:  15.61 ms | Code: 2.04 Changed
  [CoAP Round 011/050] RTT:  15.60 ms | Code: 2.04 Changed
  [CoAP Round 012/050] RTT:  12.49 ms | Code: 2.04 Changed
  [CoAP Round 013/050] RTT:  12.37 ms | Code: 2.04 Changed
  [CoAP Round 014/050] RTT:  14.77 ms | Code: 2.04 Changed
  [CoAP Round 015/050] RTT:  12.67 ms | Code: 2.04 Changed
  [CoAP Round 016/050] RTT:  13.74 ms | Code: 2.04 Changed
  [CoAP Round 017/050] RTT:  17.14 ms | Code: 2.04 Changed
  [CoAP Round 018/050] RTT:  18.16 ms | Code: 2.04 Changed
  [CoAP Round 019/050] RTT:  15.25 ms | Code: 2.04 Changed
  [CoAP Round 020/050] RTT:  10.67 ms | Code: 2.04 Changed
  [CoAP Round 021/050] RTT:  18.73 ms | Code: 2.04 Changed
  [CoAP Round 022/050] RTT:  12.67 ms | Code: 2.04 Changed
  [CoAP Round 023/050] RTT:  13.34 ms | Code: 2.04 Changed
  [CoAP Round 024/050] RTT:  12.07 ms | Code: 2.04 Changed
  [CoAP Round 025/050] RTT:  13.20 ms | Code: 2.04 Changed
  [CoAP Round 026/050] RTT:   9.83 ms | Code: 2.04 Changed
  [CoAP Round 027/050] RTT:   8.21 ms | Code: 2.04 Changed
  [CoAP Round 028/050] RTT:  16.64 ms | Code: 2.04 Changed
  [CoAP Round 029/050] RTT:  11.54 ms | Code: 2.04 Changed
  [CoAP Round 030/050] RTT:  12.59 ms | Code: 2.04 Changed
  [CoAP Round 031/050] RTT:   8.35 ms | Code: 2.04 Changed
  [CoAP Round 032/050] RTT:  12.63 ms | Code: 2.04 Changed
  [CoAP Round 033/050] RTT:  11.75 ms | Code: 2.04 Changed
  [CoAP Round 034/050] RTT:  12.48 ms | Code: 2.04 Changed
  [CoAP Round 035/050] RTT:  11.46 ms | Code: 2.04 Changed
  [CoAP Round 036/050] RTT:   8.09 ms | Code: 2.04 Changed
  [CoAP Round 037/050] RTT:  13.50 ms | Code: 2.04 Changed
  [CoAP Round 038/050] RTT:  13.52 ms | Code: 2.04 Changed
  [CoAP Round 039/050] RTT:   7.93 ms | Code: 2.04 Changed
  [CoAP Round 040/050] RTT:   9.60 ms | Code: 2.04 Changed
  [CoAP Round 041/050] RTT:  21.92 ms | Code: 2.04 Changed
  [CoAP Round 042/050] RTT:   8.70 ms | Code: 2.04 Changed
  [CoAP Round 043/050] RTT:  10.58 ms | Code: 2.04 Changed
  [CoAP Round 044/050] RTT:   8.59 ms | Code: 2.04 Changed
  [CoAP Round 045/050] RTT:   7.32 ms | Code: 2.04 Changed
  [CoAP Round 046/050] RTT:  10.61 ms | Code: 2.04 Changed
  [CoAP Round 047/050] RTT:  11.37 ms | Code: 2.04 Changed
  [CoAP Round 048/050] RTT:   9.20 ms | Code: 2.04 Changed
  [CoAP Round 049/050] RTT:  12.04 ms | Code: 2.04 Changed
  [CoAP Round 050/050] RTT:  12.02 ms | Code: 2.04 Changed

================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
CoAP (RFC7252) | 50/50      |   0.0% |     7.32 |     21.16 |   384.03 |      52.03
================================================================================

================================================================================
             THEORETICAL PROTOCOL OVERHEAD ANALYSIS (Per LED Command)
================================================================================
Layer / Attribute              | HTTP REST (POST)  | UDP Socket    | CoAP (PUT)
--------------------------------------------------------------------------------
Transport Layer (L4)           | TCP (20-32 bytes) | UDP (8 bytes) | UDP (8 bytes)
Application Header (L7)        | ~150-250 bytes    | 0 bytes       | 4-8 bytes
Command Payload Size           | 14 bytes (JSON)   | 6 bytes (ASCII) | 1 byte (char)
Est. Total Bytes per Request   | ~190-300 bytes    | ~14 bytes     | ~13-17 bytes
Payload Efficiency Ratio       | ~4.5 - 7.0 %      | ~42.8 %       | ~7.7 - 25.0 %
================================================================================

[✔] Benchmark Complete! Copy the metrics above into your Lab 10.4 Report.
```

---

### กิจกรรมที่ 10-4.3 การตรวจสอบหน่วยความจำบน ESP32 (RAM Footprint)

| สถานะการทำงานของ ESP32 | หน่วยความจำแรมคงเหลือ (Free Heap) | ปริมาณแรมที่ใช้ไปโดยประมาณ |
| :--- | :---: | :---: |
| **Baseline หลังต่อ Wi-Fi สำเร็จ** (ก่อนรันเซิร์ฟเวอร์) | 245 KB | อ้างอิง (0 KB) |
| **HTTP REST Server (`esp_http_server`)** | 208 KB | 37 KB |
| **UDP Socket Server (LwIP Raw Socket)** | 239 KB | 6 KB |
| **CoAP Server (`espressif/coap` / `libcoap`)** | 192 KB | 53 KB |

---

## 2. วิเคราะห์ผลและสรุปบทเรียนเชิงวิศวกรรม (Engineering Synthesis)

### 1. เปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix)

- **HTTP:**
  - **ข้อดี:** ใช้งานง่าย ใช้งานร่วมกับเว็บบราวเซอร์และระบบทั่วไปได้ทันที มีความน่าเชื่อถือสูงในการรับส่งข้อมูล
  - **ข้อจำกัด:** ส่วนหัวข้อมูลมีขนาดใหญ่มาก ตอบสนองช้า กินแบนด์วิดท์และแรมสูง

- **UDP:**
  - **ข้อดี:** ส่งข้อมูลได้เร็วที่สุด ความหน่วงเวลาต่ำมาก และใช้ทรัพยากรแรมน้อยที่สุด
  - **ข้อจำกัด:** ไม่รับประกันว่าข้อมูลจะส่งถึงปลายทาง ไม่มีการส่งซ้ำเมื่อข้อมูลสูญหาย

- **CoAP:**
  - **ข้อดี:** ขนาดส่วนหัวเล็กมาก รองรับรูปแบบคำสั่งเหมือนเว็บ และเลือกได้ว่าจะให้มีการยืนยันข้อมูลหรือไม่
  - **ข้อจำกัด:** ต้องใช้โปรแกรมหรือไลบรารีเฉพาะ ไม่สามารถเปิดดูผ่านเว็บบราวเซอร์ทั่วไปได้โดยตรง

---

### 2. กรณีศึกษาการตัดสินใจเลือกใช้โปรโตคอล (Engineering Case Studies)

#### กรณีศึกษาที่ 1
*ระบบเซนเซอร์วัดความชื้นในดินและวาล์วน้ำเพื่อการเกษตรอัจฉริยะ ทำงานด้วยแบตเตอรี่โซลาร์เซลล์ขนาดเล็ก ติดตั้งกลางแจ้งห่างไกล*
- **โปรโตคอลที่เลือก:** CoAP
- **เหตุผลทางวิศวกรรม:** ใช้พลังงานและแบนด์วิดท์ต่ำมากเพราะส่วนหัวเล็ก ช่วยประหยัดแบตเตอรี่ และยังมีระบบยืนยันความถูกต้องของข้อมูลได้

#### กรณีศึกษาที่ 2
*เว็บแดชบอร์ดสำหรับฝ่ายซ่อมบำรุงในโรงงาน เพื่อเปิดดูสถานะและตั้งค่าพารามิเตอร์ของอุปกรณ์ผ่านเว็บบราวเซอร์บนแท็บเล็ต/สมาร์ตโฟน*
- **โปรโตคอลที่เลือก:** HTTP
- **เหตุผลทางวิศวกรรม:** เว็บบราวเซอร์ทั่วไปรองรับได้ทันทีโดยไม่ต้องติดตั้งโปรแกรมเสริม และพัฒนาเชื่อมต่อกับหน้าเว็บได้สะดวกที่สุด

#### กรณีศึกษาที่ 3
*ระบบควบคุมแขนกลอุตสาหกรรมความเร็วสูง ที่ต้องการส่งพิกัดตำแหน่งแกนหมุน (X, Y, Z) ด้วยความถี่ 100 ครั้งต่อวินาที (100 Hz) ภายในเครือข่าย LAN ปิด (intranet)*
- **โปรโตคอลที่เลือก:** UDP
- **เหตุผลทางวิศวกรรม:** มีความเร็วสูงสุดและความหน่วงเวลาต่ำที่สุด ข้อมูลใหม่ที่ส่งถี่ๆ สำคัญกว่าข้อมูลเก่า จึงไม่จำเป็นต้องเสียเวลารอส่งซ้ำ
