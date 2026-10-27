# ใบงานการทดลองที่ 10.4
### การทดสอบเปรียบเทียบสมรรถนะ (Benchmark) ของโปรโตคอลเครือข่ายด้วย Python Script และสถิติเชิงวิศวกรรม

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้ นักศึกษาจะได้สวมบทบาทเป็น **วิศวกรระบบไอโอที (IoT Systems Engineer)** เพื่อประเมินและคัดเลือกโปรโตคอลสื่อสารที่เหมาะสมที่สุดสำหรับอุปกรณ์ฝังตัว โดยนำระบบที่ได้พัฒนาขึ้นจริงในแล็บ 10.1 (HTTP REST Server), แล็บ 10.2 (UDP Socket), และแล็บ 10.3 (CoAP Server) มาทดสอบเปรียบเทียบเชิงประจักษ์ ผ่านสคริปต์ทดสอบสมรรถนะอัตโนมัติ **`benchmark_protocols.py`** เพื่อวัดผลทั้งในมิติของ **ขนาดส่วนหัว (Header Overhead)**, **ความหน่วงเวลาตอบสนอง (Round-Trip Latency)**, **การกระจายตัวของเวลา (Jitter)**, **ความเสถียร (Packet Loss)** และ **การใช้หน่วยความจำแรม (RAM Footprint)**

---

## 1. วัตถุประสงค์การทดลอง
1. เข้าใจโครงสร้างและภาระส่วนหัว (Overhead) ของโปรโตคอล HTTP, UDP, และ CoAP ในระดับ Transport Layer (L4) และ Application Layer (L7) ตามมาตรฐาน RFC
2. สามารถใช้สคริปต์ Python ในการทดสอบวัดเวลาตอบสนอง (Round-Trip Time: RTT) แบบวนซ้ำเพื่อเก็บข้อมูลทางสถิติ (Min, Max, Mean, Jitter)
3. สามารถวิเคราะห์ความสัมพันธ์ระหว่าง Protocol Overhead กับประสิทธิภาพการใช้แบนด์วิดท์ (Payload Efficiency Ratio)
4. สามารถเปรียบเทียบปริมาณการใช้ทรัพยากรแรม (Free Heap Memory) บนชิป ESP32 ของแต่ละโปรโตคอลได้
5. มีทักษะในการตัดสินใจเชิงวิศวกรรมเพื่อเลือกใช้โปรโตคอลที่คุ้มค่าและเหมาะสมกับลักษณะของงาน IoT แต่ละประเภท

---

## 2. กรอบทฤษฎีการวิเคราะห์ภาระส่วนหัว (Protocol Overhead Breakdown)

ในการสั่งเปิดหรือปิดหลอดไฟ LED บน ESP32 เพียง 1 ครั้ง ข้อมูลจริงที่ต้องการสั่งการ (Payload) มีขนาดเพียง 1 ถึง 14 ไบต์ แต่ละโปรโตคอลจะสร้างภาระส่วนหัว (Header) ที่แตกต่างกันอย่างสิ้นเชิง:


<p align="center"><img src="Images/Protocol%20Overhead_academic.svg"> </p>
 


---

## 3. ขั้นตอนการทดลอง

### เริ่มต้นใช้งาน Benchmark จาก Repository (แนะนำ)

ใช้สคริปต์ที่เตรียมไว้ใน `Example_ESP32_Project\Lab10-4_Protocol_Benchmark` โดยต้องแฟลชเฟิร์มแวร์ของโปรโตคอลที่จะวัดลง ESP32 ก่อน สคริปต์ benchmark เป็น client ไม่ได้รัน HTTP/UDP/CoAP server แทนบอร์ด

1. เชื่อมต่อคอมพิวเตอร์และ ESP32 กับ Wi-Fi วงเดียวกัน เปิด Serial Monitor แล้วจด IP address ของบอร์ด เช่น `172.20.10.2`
2. เปิด PowerShell ที่ใช้ Python สำหรับงานทั่วไป (ไม่จำเป็นต้องใช้ ESP-IDF PowerShell) แล้วเปิด virtual environment ที่ต้องการใช้ หากยังไม่มี สร้างและเปิด environment แยกสำหรับ benchmark:

   ```powershell
   python -m venv "$env:USERPROFILE\iot-benchmark-venv"
   & "$env:USERPROFILE\iot-benchmark-venv\Scripts\Activate.ps1"
   ```

   ติดตั้ง dependency สำหรับการทดสอบ CoAP ใน environment เดียวกัน:

   ```powershell
   python -m pip install aiocoap
   ```

3. เข้าโฟลเดอร์ benchmark:

   ```powershell
   cd Example_ESP32_Project\Lab10-4_Protocol_Benchmark
   ```

4. แฟลช firmware ที่ต้องการทดสอบทีละโปรโตคอล แล้วรันคำสั่งที่ตรงกัน:

   ```powershell
   # หลังแฟลช firmware จาก Lab 10-1 (HTTP REST)
   python benchmark_protocols.py --target 172.20.10.2 --protocol http --rounds 50

   # หลังแฟลช firmware จาก Lab 10-2 (UDP Socket)
   python benchmark_protocols.py --target 172.20.10.2 --protocol udp --rounds 50

   # หลังแฟลช firmware จาก Lab 10-3 (CoAP)
   python benchmark_protocols.py --target 172.20.10.2 --protocol coap --rounds 50
   ```

   เปลี่ยน `172.20.10.2` เป็น IP ปัจจุบันที่เห็นใน Serial Monitor ทุกครั้งที่บอร์ดได้ IP ใหม่ บันทึกผลของแต่ละรอบลงตารางรายงานก่อนแฟลชโปรโตคอลถัดไป

**ข้อควรระวัง:** อย่าเลือก `--protocol all` ขณะบอร์ดรัน firmware ของโปรโตคอลเดียว เพราะ ESP32 ตัวอย่างแต่ละชุดเปิด server คนละ endpoint; คำขอของโปรโตคอลอื่นจะ timeout และทำให้ผลเปรียบเทียบคลาดเคลื่อน หาก CoAP แจ้งว่าไม่พบ `aiocoap` ให้ตรวจว่าเปิด virtual environment เดิมและใช้ `python -m pip install aiocoap` ด้วย Python ตัวเดียวกับที่ใช้รัน benchmark

### กิจกรรมที่ 10-4.1 การวิเคราะห์สัดส่วนข้อมูลตามมาตรฐานสากล (Architectural Byte Breakdown)
ให้นักศึกษาวิเคราะห์ขนาดของเฟรมข้อมูลตามมาตรฐาน RFC สำหรับการส่งคำสั่งเปิดไฟ LED
* **HTTP/1.1 (RFC 7230)**: อาศัย TCP 3-Way Handshake, ข้อความ Header เป็น Plaintext ASCII เช่น `POST /api/led HTTP/1.1\r\nContent-Type: application/json\r\n...`
* **UDP Raw Socket (RFC 768)**: ไม่ต้องมี Handshake, ไม่มีส่วนหัวในชั้น Application (0 ไบต์) ส่ง Payload ตรงเข้าพอร์ต
* **CoAP (RFC 7252)**: ทำงานบน UDP ใช้ Binary Header ขนาดคงที่เพียง 4 ไบต์ (Ver, Type, TKL, Code, Message ID)

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

$$\text{Payload Efficiency Ratio} = \frac{\text{Payload Size (Bytes)}}{\text{Total Transmitted Bytes}} \times 100\%$$

> [!TIP] **ประเด็นสังเกตเชิงวิศวกรรม**
> สำหรับอุปกรณ์ IoT ที่ใช้แบตเตอรี่และเชื่อมต่อผ่านซิมเซลลูลาร์ (NB-IoT / 4G) ที่คิดค่าบริการตามปริมาณข้อมูล การส่งคำสั่งด้วย CoAP หรือ UDP จะช่วยประหยัดค่าแบนด์วิดท์และลดระยะเวลาการเปิดใช้วิทยุ (Radio Airtime) ได้มากกว่า HTTP ถึง **15–20 เท่า**

---

### กิจกรรมที่ 10-4.2 การทดสอบวัดความหน่วงเวลา (RTT Latency & Jitter Benchmarking)

ในการทดลองนี้ เราจะใช้สคริปต์ Python **`benchmark_protocols.py`** ซึ่งเป็นเครื่องมือวัดสมรรถนะแบบอัตโนมัติ ส่งคำสั่งควบคุมซ้ำจำนวน **50 ถึง 100 รอบ** เพื่อคำนวณหาค่าเฉลี่ยทางสถิติ:
* **Minimum RTT**  เวลาตอบสนองที่เร็วที่สุด (ms)
* **Maximum RTT**  เวลาตอบสนองที่ช้าที่สุด (ms)
* **Average (Mean) RTT**  เวลาตอบสนองเฉลี่ย (ms)
* **Jitter (Standard Deviation $\sigma$)**  ความแปรปรวนของเวลา สะท้อนความสม่ำเสมอของเครือข่าย
* **Packet Loss Rate (%)**  สัดส่วนคำขอที่สูญหายหรือไม่ได้รับคำตอบภายใน Timeout (2 วินาที)

#### 1. ตรวจสอบการติดตั้งไลบรารีบนคอมพิวเตอร์
เปิด PowerShell บนเครื่องคอมพิวเตอร์:
```powershell
pip install aiocoap
```

#### 2. รันสคริปต์ Benchmark
สคริปต์ `benchmark_protocols.py` ได้ถูกจัดเตรียมไว้ในโฟลเดอร์โปรเจกต์ นักศึกษาสามารถรันคำสั่งโดยระบุ IP ของ ESP32  ซึ่งดูได้จาก terminal ของ ESP32  จากตัวอย่างด้านล่างนี้ เราได้  IP = 192.168.1.181)

```
I (823) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (903) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (1863) esp_netif_handlers: sta ip: 192.168.1.181, mask: 255.255.255.0, gw: 192.168.1.1
I (1863) COAP_LAB: Connected! IP Address: 192.168.1.181
I (1863) COAP_LAB: CoAP Server listening on port 5683...
I (1863) COAP_LAB: Ready! Test CoAP with: python test_coap.py
I (1873) main_task: Returned from app_main()
```



สคริปต์ `benchmark_protocols.py` สามารถคัดลอกได้จากหัวข้อ  **A. สคริปต์ python `benchmark_protocols.py` สำหรับทำ benchmark โพรโตคอลต่าง ๆ** ด้านล่างสุดของใบงาน (กดเพื่อคลี่ออก)


```powershell
# รันแบบ Interactive ให้เลือกเมนู
python benchmark_protocols.py --target 192.168.1.181 --rounds 50
```

หรือระบุโปรโตคอลที่ต้องการทดสอบโดยตรง:
```powershell
# ทดสอบ CoAP (สำหรับเฟิร์มแวร์ Lab 10.3)
python benchmark_protocols.py --target 192.168.1.181 --protocol coap --rounds 50

# ทดสอบ UDP Socket (สำหรับเฟิร์มแวร์ Lab 10.2)
python benchmark_protocols.py --target 192.168.1.181 --protocol udp --rounds 50

# ทดสอบ HTTP REST (สำหรับเฟิร์มแวร์ Lab 10.1)
python benchmark_protocols.py --target 192.168.1.181 --protocol http --rounds 50
```

**หมายเหตุ**
1. นักศึกษาสามารถทดสอบกับเฟิร์มแวร์ปัจจุบันที่แฟลชอยู่บนบอร์ด แล้วนำผลมาบันทึกลงตาราง
2. ต้องย้อนกลับไป flash โพรโตคอลในใบงาน 10-1 และ 10-2 ด้วย แต่ละใบงานให้ทดสอบตามโพรโตคอลนั้น

#### ตัวอย่างผลลัพธ์จากสคริปต์ เมื่อ ESP32 รันใบงานที่ 10-3
```text
  [*] Starting CoAP Benchmark -> coap://192.168.1.181:5683/actuator/led (50 rounds)...
  [CoAP Round 001/050] RTT:  68.42 ms | Code: 2.04 Changed
  [CoAP Round 002/050] RTT:  10.59 ms | Code: 2.04 Changed
  [CoAP Round 003/050] RTT:   8.58 ms | Code: 2.04 Changed
  [CoAP Round 004/050] RTT:   9.85 ms | Code: 2.04 Changed
  .
  .
  . 
  [CoAP Round 049/050] RTT:   8.35 ms | Code: 2.04 Changed
  [CoAP Round 050/050] RTT:   8.80 ms | Code: 2.04 Changed

================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
CoAP (RFC7252) | 50/50      |   0.0% |     8.35 |     12.93 |    68.42 |       8.85
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




## ผลการทดลองที่ได้ HTTP REST
```powershell
================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
HTTP (REST)  | 50/50      |   0.0% |    79.27 |    200.79 |   613.42 |     131.79
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

```

## ผลการทดลองที่ได้ UDP Socket
```powershell
================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
UDP (Socket) | 50/50      |   0.0% |     7.04 |     12.82 |    38.21 |       6.10
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
## ผลการทดลองที่ได้ CoAP PUT
```powershell

================================================================================
   ESP32 Local Protocol Benchmark: HTTP vs UDP vs CoAP
   Industrial IoT Systems Engineering - Real-time Performance Lab
================================================================================


[*] Starting CoAP Benchmark -> coap://172.20.10.2:5683/actuator/led (50 rounds)...
  [CoAP Round 001/050] RTT: 1123.88 ms | Code: 2.04 Changed
  [CoAP Round 002/050] RTT:  15.94 ms | Code: 2.04 Changed
  [CoAP Round 003/050] RTT:  13.78 ms | Code: 2.04 Changed
  [CoAP Round 004/050] RTT:  14.56 ms | Code: 2.04 Changed
  [CoAP Round 005/050] RTT:  17.19 ms | Code: 2.04 Changed
  [CoAP Round 006/050] RTT:  12.00 ms | Code: 2.04 Changed
  [CoAP Round 007/050] RTT:  19.26 ms | Code: 2.04 Changed
  [CoAP Round 008/050] RTT:  16.79 ms | Code: 2.04 Changed
  [CoAP Round 009/050] RTT:  14.90 ms | Code: 2.04 Changed
  [CoAP Round 010/050] RTT:  19.04 ms | Code: 2.04 Changed
  [CoAP Round 011/050] RTT:  13.69 ms | Code: 2.04 Changed
  [CoAP Round 012/050] RTT:  13.82 ms | Code: 2.04 Changed
  [CoAP Round 013/050] RTT:  20.26 ms | Code: 2.04 Changed
  [CoAP Round 014/050] RTT: 289.14 ms | Code: 2.04 Changed
  [CoAP Round 015/050] RTT:  11.97 ms | Code: 2.04 Changed
  [CoAP Round 016/050] RTT:  15.14 ms | Code: 2.04 Changed
  [CoAP Round 017/050] RTT:  13.41 ms | Code: 2.04 Changed
  [CoAP Round 018/050] RTT:  12.99 ms | Code: 2.04 Changed
  [CoAP Round 019/050] RTT:  15.13 ms | Code: 2.04 Changed
  [CoAP Round 020/050] RTT:  14.41 ms | Code: 2.04 Changed
  [CoAP Round 021/050] RTT:  13.76 ms | Code: 2.04 Changed
  [CoAP Round 022/050] RTT:  12.48 ms | Code: 2.04 Changed
  [CoAP Round 023/050] RTT:  12.88 ms | Code: 2.04 Changed
  [CoAP Round 024/050] RTT:  17.97 ms | Code: 2.04 Changed
  [CoAP Round 025/050] RTT:  21.84 ms | Code: 2.04 Changed
  [CoAP Round 026/050] RTT:  12.68 ms | Code: 2.04 Changed
  [CoAP Round 027/050] RTT:  17.92 ms | Code: 2.04 Changed
  [CoAP Round 028/050] RTT:  16.53 ms | Code: 2.04 Changed
  [CoAP Round 029/050] RTT:  18.09 ms | Code: 2.04 Changed
  [CoAP Round 030/050] RTT:  12.44 ms | Code: 2.04 Changed
  [CoAP Round 031/050] RTT:  31.54 ms | Code: 2.04 Changed
  [CoAP Round 032/050] RTT:  13.69 ms | Code: 2.04 Changed
  [CoAP Round 033/050] RTT:  12.55 ms | Code: 2.04 Changed
  [CoAP Round 034/050] RTT:  11.93 ms | Code: 2.04 Changed
  [CoAP Round 035/050] RTT:  15.58 ms | Code: 2.04 Changed
  [CoAP Round 036/050] RTT:  11.92 ms | Code: 2.04 Changed
  [CoAP Round 037/050] RTT:  19.32 ms | Code: 2.04 Changed
  [CoAP Round 038/050] RTT:  19.71 ms | Code: 2.04 Changed
  [CoAP Round 039/050] RTT:  28.64 ms | Code: 2.04 Changed
  [CoAP Round 040/050] RTT:  19.69 ms | Code: 2.04 Changed
  [CoAP Round 041/050] RTT:  15.39 ms | Code: 2.04 Changed
  [CoAP Round 042/050] RTT:  18.84 ms | Code: 2.04 Changed
  [CoAP Round 043/050] RTT:  16.03 ms | Code: 2.04 Changed
  [CoAP Round 044/050] RTT:  19.18 ms | Code: 2.04 Changed
  [CoAP Round 045/050] RTT:  15.28 ms | Code: 2.04 Changed
  [CoAP Round 046/050] RTT:  12.29 ms | Code: 2.04 Changed
  [CoAP Round 047/050] RTT: 321.14 ms | Code: 2.04 Changed
  [CoAP Round 048/050] RTT:  15.69 ms | Code: 2.04 Changed
  [CoAP Round 049/050] RTT:  14.99 ms | Code: 2.04 Changed
  [CoAP Round 050/050] RTT:  15.47 ms | Code: 2.04 Changed

================================================================================
                    BENCHMARK RESULTS & METRICS SUMMARY
================================================================================
Protocol     | Success   | Loss %  | Min (ms)  | Mean (ms)  | Max (ms)  | Jitter (ms)
--------------------------------------------------------------------------------
CoAP (RFC7252) | 50/50      |   0.0% |    11.92 |     49.86 |  1123.88 |     163.62
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
#### 3. ตารางบันทึกผลการทดสอบ RTT จริง (หน่วย: มิลลิวินาที ms)

| โปรโตคอล                       | จำนวนรอบที่สำเร็จ | Packet Loss (%) | Min RTT (ms) | Mean RTT (ms) | Max RTT (ms) | Jitter / StdDev (ms) |
| :----------------------------- | :---------------: | :-------------: | :----------: | :-----------: | :----------: | :------------------: |
| **HTTP REST (`/api/led`)**     |  **50 / 50**      |   **0.0%**      | **79.27 ms** | **200.79 ms** | **613.42 ms** | **131.79 ms** |
| **UDP Socket (`พอร์ต 3333`)**  |  **50 / 50**      |   **0.0%**      | **7.04 ms**  | **12.82 ms**  | **38.21 ms** | **6.10 ms**       |
| **CoAP PUT (`/actuator/led`)** |  **50 / 50**      |   **0.0%**      | **11.92 ms** | **49.86 ms**  | **1123.88 ms** | **163.62 ms** |

---

### กิจกรรมที่ 10-4.3 การตรวจสอบหน่วยความจำบน ESP32 (RAM Footprint)

ตรวจสอบการใช้ทรัพยากรแรมบนบอร์ด ESP32 โดยสังเกตค่า `free_heap` จาก Log Monitor (`idf.py monitor`) หรือจาก HTTP JSON Response (`/api/status`)

| สถานะการทำงานของ ESP32                                 | หน่วยความจำแรมคงเหลือ (Free Heap) | ปริมาณแรมที่ใช้ไปโดยประมาณ |
| :----------------------------------------------------- | :-------------------------------: | :------------------------: |
| **Baseline หลังต่อ Wi-Fi สำเร็จ** (ก่อนรันเซิร์ฟเวอร์) |           ~ 240–260 KB            |       อ้างอิง (0 KB)       |
| **HTTP REST Server (`esp_http_server`)**               |           ~ 210–225 KB            |         ~ 25–40 KB         |
| **UDP Socket Server (LwIP Raw Socket)**                |           ~ 235–250 KB            |       **~ 5–10 KB**        |
| **CoAP Server (`espressif/coap` / `libcoap`)**         |           ~ 190–215 KB            |         ~ 35–50 KB         |

> [!NOTE] **วิเคราะห์เชิงสถาปัตยกรรม:**
> * **UDP Socket** ใช้แรมน้อยที่สุดเนื่องจากเป็นเพียง Socket FD ธรรมดาบน LwIP ไม่ต้องมี State Machine หรือ Context Complex
> * **HTTP Server** ใช้แรมปานกลางสำหรับจัดสรร Connection Slots และ Parsing Buffer
> * **CoAP Server (libcoap)** มีขนาด Task Stack Size (8 KB) และหน่วยความจำสำหรับจัดการ Resource Tree, PDU Session State, และ Observer List

---

## 4. วิเคราะห์ผลและสรุปบทเรียนเชิงวิศวกรรม (Engineering Synthesis)

ให้นักศึกษาตอบคำถามเชิงวิเคราะห์ต่อไปนี้ลงในรายงานการทดลอง

### 1. เปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix)
จงสรุปข้อดีและข้อจำกัดของ **HTTP, UDP, และ CoAP** จากผลการทดลองจริง 
* **HTTP** ใช้งานง่ายและรองรับโดยเว็บเบราว์เซอร์/เครื่องมือทั่วไป เหมาะกับ REST API แต่มี overhead จาก HTTP และ TCP สูงกว่า อีกทั้งผลทดลองนี้มี RTT เฉลี่ย **200.79 ms** และ jitter **131.79 ms** จึงไม่เหมาะกับการควบคุมที่ต้องการความหน่วงต่ำหรือสม่ำเสมอ
* **UDP** มี overhead ต่ำ ไม่ต้องตั้งการเชื่อมต่อ และจากผลทดลองนี้ให้ RTT เฉลี่ยต่ำสุดที่ **12.82 ms** (ต่ำสุด **7.04 ms**, jitter **6.10 ms**) และสำเร็จ **50/50 รอบ** จึงเหมาะกับงานส่งข้อมูล/ควบคุมที่ต้องการตอบสนองเร็ว อย่างไรก็ตาม UDP ไม่รับประกันการส่งถึงหรือลำดับแพ็กเก็ต ผู้พัฒนาต้องจัดการการยืนยัน/ส่งซ้ำและตรวจจับแพ็กเก็ตสูญหายเองเมื่อจำเป็น
* **CoAP** ออกแบบมาสำหรับอุปกรณ์ IoT และใช้ UDP พร้อมรูปแบบ request/response แบบ REST ที่มี overhead ต่ำกว่า HTTP โดยรองรับการส่งแบบยืนยัน (CON) ได้ แต่ผลทดลองนี้มี RTT เฉลี่ย **49.86 ms**, jitter **163.62 ms** และมีค่าสูงสุด **1123.88 ms** แม้สำเร็จ **50/50 รอบ** จึงควรพิจารณาความผันผวนที่พบและทดสอบซ้ำในสภาพเครือข่ายจริงก่อนเลือกใช้

### 2. กรณีศึกษาการตัดสินใจเลือกใช้โปรโตคอล (Engineering Case Studies)
หากท่านเป็นหัวหน้าวิศวกรออกแบบระบบ IoT จงเลือกโปรโตคอลที่เหมาะสมที่สุดสำหรับแต่ละกรณีศึกษาต่อไปนี้ พร้อมให้เหตุผลทางวิศวกรรมสนับสนุน

* **กรณีศึกษาที่ 1** *ระบบเซนเซอร์วัดความชื้นในดินและวาล์วน้ำเพื่อการเกษตรอัจฉริยะ ทำงานด้วยแบตเตอรี่โซลาร์เซลล์ขนาดเล็ก ติดตั้งกลางแจ้งห่างไกล*
  * **โปรโตคอลที่เลือก** CoAP
  * **เหตุผลทางวิศวกรรม** CoAP ใช้ UDP และมีส่วนหัวขนาดเล็ก เหมาะกับอุปกรณ์ IoT ที่มีพลังงานและแบนด์วิดท์จำกัด อีกทั้งมีรูปแบบ resource/request-response สำหรับอ่านค่าความชื้นและสั่งวาล์ว และเลือกใช้ข้อความแบบ CON เพื่อให้มีการยืนยันการรับคำสั่งสำคัญได้ อย่างไรก็ตาม การติดตั้งกลางแจ้งห่างไกลยังต้องออกแบบเครือข่ายและการจัดการพลังงานให้เหมาะสม เพราะโปรโตคอลเพียงอย่างเดียวไม่ได้รับประกันระยะสัญญาณหรืออายุแบตเตอรี่

* **กรณีศึกษาที่ 2** *เว็บแดชบอร์ดสำหรับฝ่ายซ่อมบำรุงในโรงงาน เพื่อเปิดดูสถานะและตั้งค่าพารามิเตอร์ของอุปกรณ์ผ่านเว็บบราวเซอร์บนแท็บเล็ต/สมาร์ตโฟน*
  * **โปรโตคอลที่เลือก** HTTP REST
  * **เหตุผลทางวิศวกรรม** เว็บเบราว์เซอร์รองรับ HTTP โดยตรง และ REST API เชื่อมต่อกับเว็บแดชบอร์ดและระบบหลังบ้านได้สะดวก เหมาะกับการดูสถานะและตั้งค่าที่ไม่ต้องตอบสนองระดับมิลลิวินาที แม้ผลทดลองนี้ HTTP จะมี RTT เฉลี่ยสูงกว่า UDP/CoAP แต่ความเข้ากันได้และความง่ายในการพัฒนาสำคัญกับกรณีนี้ ควรใช้ HTTPS พร้อมการยืนยันตัวตนเมื่อใช้งานจริง

* **กรณีศึกษาที่ 3** *ระบบควบคุมแขนกลอุตสาหกรรมความเร็วสูง ที่ต้องการส่งพิกัดตำแหน่งแกนหมุน (X, Y, Z) ด้วยความถี่ 100 ครั้งต่อวินาที (100 Hz) ภายในเครือข่าย LAN ปิด (intranet)*
  * **โปรโตคอลที่เลือก** UDP Socket (ในตัวเลือกของการทดลองนี้)
  * **เหตุผลทางวิศวกรรม** ผลทดลองใน LAN ให้ UDP มี RTT เฉลี่ยต่ำสุด (**12.82 ms**) และ jitter ต่ำสุด (**6.10 ms**) ในสามโปรโตคอล จึงเหมาะกว่า HTTP/CoAP สำหรับการส่งข้อมูลถี่และต้องการลด overhead อย่างไรก็ตาม ค่า RTT ที่วัดได้ไม่ได้ยืนยันว่าจะรองรับรอบเวลา 10 ms ของ 100 Hz ได้อย่างสม่ำเสมอ และ UDP ไม่รับประกันการส่งถึงหรือลำดับข้อมูล ดังนั้นระบบแขนกลจริงต้องทดสอบ latency/jitter ภายใต้โหลดและใช้กลไกตรวจลำดับ/ข้อมูลเก่า, watchdog และระบบหยุดอย่างปลอดภัย; งานควบคุมที่ต้องการ deterministic real-time ควรพิจารณา fieldbus สำหรับอุตสาหกรรมโดยเฉพาะแทน UDP ทั่วไป



## A. สคริปต์ python `benchmark_protocols.py` สำหรับทำ benchmark โพรโตคอลต่าง ๆ 

<details>
<summary><b>🔍 คลิกดูซอร์สโค้ดฉบับสมบูรณ์ทั้งไฟล์ (benchmark_protocols.py)</b></summary>

```python
#!/usr/bin/env python3
"""
================================================================================
IoT Local Communication & Control Benchmark Tool
Week 10: HTTP vs UDP vs CoAP Protocol Performance Analyzer
================================================================================
Author: IoT Systems Engineering Laboratory
Purpose: Benchmarking Round-Trip Time (RTT), Jitter, Packet Reliability,
         and Protocol Overhead for ESP32 Local Control.
================================================================================
"""

import sys
import time
import math
import socket
import argparse
import asyncio
from typing import List, Dict, Any, Optional

# Protocol Specific Imports with Graceful Fallbacks
try:
    import urllib.request
    import json
except ImportError:
    pass

try:
    from aiocoap import Context, Message, Code
except ImportError:
    pass


class BenchmarkStats:
    """Calculates statistical metrics for benchmark rounds."""
    def __init__(self, latencies: List[float], sent_bytes: int, recv_bytes: int, total_rounds: int):
        self.total_rounds = total_rounds
        self.success_rounds = len(latencies)
        self.failed_rounds = total_rounds - self.success_rounds
        self.latencies = latencies
        self.sent_bytes = sent_bytes
        self.recv_bytes = recv_bytes

        if self.success_rounds > 0:
            self.min_rtt = min(latencies)
            self.max_rtt = max(latencies)
            self.mean_rtt = sum(latencies) / self.success_rounds
            variance = sum((x - self.mean_rtt) ** 2 for x in latencies) / self.success_rounds
            self.std_dev = math.sqrt(variance)
            self.packet_loss_pct = (self.failed_rounds / self.total_rounds) * 100.0
        else:
            self.min_rtt = 0.0
            self.max_rtt = 0.0
            self.mean_rtt = 0.0
            self.std_dev = 0.0
            self.packet_loss_pct = 100.0


def print_banner():
    banner = r"""
================================================================================
   ESP32 Local Protocol Benchmark: HTTP vs UDP vs CoAP
   Industrial IoT Systems Engineering - Real-time Performance Lab
================================================================================
"""
    print(banner)


# ------------------------------------------------------------------------------
# 1. HTTP REST Benchmark
# ------------------------------------------------------------------------------
def benchmark_http(target: str, rounds: int, timeout: float) -> BenchmarkStats:
    print(f"\n[*] Starting HTTP REST Benchmark -> http://{target}/api/led ({rounds} rounds)...")
    url = f"http://{target}/api/led"
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    headers = {
        "Content-Type": "application/json",
        "Connection": "keep-alive"
    }

    for i in range(1, rounds + 1):
        state = (i % 2 == 1)
        payload = json.dumps({"state": state}).encode("utf-8")
        req = urllib.request.Request(url, data=payload, headers=headers, method="POST")

        t_start = time.perf_counter()
        try:
            with urllib.request.urlopen(req, timeout=timeout) as response:
                resp_data = response.read()
                t_end = time.perf_counter()
                rtt = (t_end - t_start) * 1000.0  # ms
                latencies.append(rtt)
                total_tx_bytes += len(payload) + 180  # approx HTTP POST header size
                total_rx_bytes += len(resp_data) + 120  # approx HTTP 200 header size
                print(f"  [HTTP Round {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Status: {response.status}")
        except Exception as e:
            print(f"  [HTTP Round {i:03d}/{rounds:03d}] FAILED: {e}")

        time.sleep(0.02)  # 20ms pacing

    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


# ------------------------------------------------------------------------------
# 2. UDP Raw Socket Benchmark
# ------------------------------------------------------------------------------
def benchmark_udp(target: str, rounds: int, timeout: float, port: int = 3333) -> BenchmarkStats:
    print(f"\n[*] Starting UDP Socket Benchmark -> {target}:{port} ({rounds} rounds)...")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    for i in range(1, rounds + 1):
        cmd = "LED_ON" if (i % 2 == 1) else "LED_OFF"
        tx_data = cmd.encode("utf-8")

        t_start = time.perf_counter()
        try:
            sock.sendto(tx_data, (target, port))
            total_tx_bytes += len(tx_data) + 8  # 8 bytes UDP header
            rx_data, _ = sock.recvfrom(128)
            t_end = time.perf_counter()
            rtt = (t_end - t_start) * 1000.0  # ms
            latencies.append(rtt)
            total_rx_bytes += len(rx_data) + 8
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Recv: {rx_data.decode('utf-8', errors='ignore')}")
        except socket.timeout:
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] TIMEOUT (Packet Dropped)")
        except Exception as e:
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] ERROR: {e}")

        time.sleep(0.02)

    sock.close()
    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


# ------------------------------------------------------------------------------
# 3. CoAP (RFC 7252) Benchmark
# ------------------------------------------------------------------------------
async def _async_benchmark_coap(target: str, rounds: int, timeout: float, port: int = 5683) -> BenchmarkStats:
    print(f"\n[*] Starting CoAP Benchmark -> coap://{target}:{port}/actuator/led ({rounds} rounds)...")
    protocol = await Context.create_client_context()
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    for i in range(1, rounds + 1):
        val = b"1" if (i % 2 == 1) else b"0"
        request = Message(code=Code.PUT, payload=val, uri=f"coap://{target}:{port}/actuator/led")

        t_start = time.perf_counter()
        try:
            response = await asyncio.wait_for(protocol.request(request).response, timeout=timeout)
            t_end = time.perf_counter()
            rtt = (t_end - t_start) * 1000.0  # ms
            latencies.append(rtt)
            total_tx_bytes += len(val) + 4 + 8  # CoAP 4-byte header + UDP 8-byte
            total_rx_bytes += 4 + 8  # CoAP response header + UDP
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Code: {response.code}")
        except asyncio.TimeoutError:
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] TIMEOUT (No ACK)")
        except Exception as e:
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] ERROR: {e}")

        await asyncio.sleep(0.02)

    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


def benchmark_coap(target: str, rounds: int, timeout: float) -> BenchmarkStats:
    if "aiocoap" not in sys.modules:
        try:
            import aiocoap
        except ImportError:
            print("[!] Error: 'aiocoap' is not installed. Please run: pip install aiocoap")
            return BenchmarkStats([], 0, 0, rounds)

    return asyncio.run(_async_benchmark_coap(target, rounds, timeout))


# ------------------------------------------------------------------------------
# 4. Result Presentation & Comparison Matrix
# ------------------------------------------------------------------------------
def display_results(results: Dict[str, BenchmarkStats]):
    print("\n" + "=" * 80)
    print("                    BENCHMARK RESULTS & METRICS SUMMARY")
    print("=" * 80)
    print(f"{'Protocol':<12} | {'Success':<9} | {'Loss %':<7} | {'Min (ms)':<9} | {'Mean (ms)':<10} | {'Max (ms)':<9} | {'Jitter (ms)':<11}")
    print("-" * 80)

    for proto_name, stats in results.items():
        if stats.success_rounds > 0:
            print(f"{proto_name:<12} | {stats.success_rounds}/{stats.total_rounds:<7} | {stats.packet_loss_pct:>5.1f}% | {stats.min_rtt:>8.2f} | {stats.mean_rtt:>9.2f} | {stats.max_rtt:>8.2f} | {stats.std_dev:>10.2f}")
        else:
            print(f"{proto_name:<12} | {stats.success_rounds}/{stats.total_rounds:<7} | {stats.packet_loss_pct:>5.1f}% | {'N/A':>8} | {'N/A':>9} | {'N/A':>8} | {'N/A':>10}")

    print("=" * 80)

    # Architectural Overhead Comparison Table
    print("\n" + "=" * 80)
    print("             THEORETICAL PROTOCOL OVERHEAD ANALYSIS (Per LED Command)")
    print("=" * 80)
    print(f"{'Layer / Attribute':<30} | {'HTTP REST (POST)':<17} | {'UDP Socket':<13} | {'CoAP (PUT)':<12}")
    print("-" * 80)
    print(f"{'Transport Layer (L4)':<30} | {'TCP (20-32 bytes)':<17} | {'UDP (8 bytes)':<13} | {'UDP (8 bytes)':<12}")
    print(f"{'Application Header (L7)':<30} | {'~150-250 bytes':<17} | {'0 bytes':<13} | {'4-8 bytes':<12}")
    print(f"{'Command Payload Size':<30} | {'14 bytes (JSON)':<17} | {'6 bytes (ASCII)':<13} | {'1 byte (char)':<12}")
    print(f"{'Est. Total Bytes per Request':<30} | {'~190-300 bytes':<17} | {'~14 bytes':<13} | {'~13-17 bytes':<12}")
    print(f"{'Payload Efficiency Ratio':<30} | {'~4.5 - 7.0 %':<17} | {'~42.8 %':<13} | {'~7.7 - 25.0 %':<12}")
    print("=" * 80)
    print("\n[✔] Benchmark Complete! Copy the metrics above into your Lab 10.4 Report.\n")


# ------------------------------------------------------------------------------
# Main Entry Point
# ------------------------------------------------------------------------------
def main():
    print_banner()

    parser = argparse.ArgumentParser(description="IoT Protocol Benchmark Suite (HTTP vs UDP vs CoAP)")
    parser.add_argument("--target", "-t", type=str, default="192.168.1.181",
                        help="Target ESP32 IP address or hostname (default: 192.168.1.181)")
    parser.add_argument("--protocol", "-p", type=str, choices=["http", "udp", "coap", "all", "interactive"],
                        default="interactive", help="Protocol to benchmark: http, udp, coap, all, or interactive")
    parser.add_argument("--rounds", "-n", type=int, default=50,
                        help="Number of test rounds per protocol (default: 50)")
    parser.add_argument("--timeout", type=float, default=2.0,
                        help="Per-request timeout in seconds (default: 2.0s)")

    args = parser.parse_args()
    target = args.target
    rounds = args.rounds
    timeout = args.timeout

    proto = args.protocol
    if proto == "interactive":
        print(f"Target ESP32 Device: {target}")
        print(f"Number of Rounds:   {rounds}\n")
        print("Select Benchmark Option:")
        print("  [1] Benchmark HTTP REST Server  (Lab 10.1 Firmware)")
        print("  [2] Benchmark UDP Socket Server (Lab 10.2 Firmware)")
        print("  [3] Benchmark CoAP Server       (Lab 10.3 Firmware)")
        print("  [4] Benchmark ALL 3 Protocols Sequentially")
        print("  [0] Exit")

        choice = input("\nEnter your choice [1-4]: ").strip()
        if choice == "1":
            proto = "http"
        elif choice == "2":
            proto = "udp"
        elif choice == "3":
            proto = "coap"
        elif choice == "4":
            proto = "all"
        else:
            print("Exiting.")
            sys.exit(0)

    results: Dict[str, BenchmarkStats] = {}

    if proto in ["http", "all"]:
        results["HTTP (REST)"] = benchmark_http(target, rounds, timeout)

    if proto in ["udp", "all"]:
        results["UDP (Socket)"] = benchmark_udp(target, rounds, timeout)

    if proto in ["coap", "all"]:
        results["CoAP (RFC7252)"] = benchmark_coap(target, rounds, timeout)

    display_results(results)


if __name__ == "__main__":
    main()

```
</details>