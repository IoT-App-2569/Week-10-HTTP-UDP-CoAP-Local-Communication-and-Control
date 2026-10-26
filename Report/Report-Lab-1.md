## 1. ผลการเตรียมและทดสอบระบบ

### 1.1 ผลการ Reconfigure ระบบบิลด์

ใช้คำสั่งต่อไปนี้เพื่อให้ ESP-IDF ตรวจสอบ Dependency และสร้างไฟล์สำหรับระบบบิลด์:

```powershell
idf.py reconfigure
```

ผลการทำงานพบข้อความสำคัญดังนี้:

```text
-- Configuring done
-- Generating done
-- Build files have been written to: ...\build
```

ผลดังกล่าวแสดงว่าโครงสร้างโปรเจกต์และ Dependency สามารถถูกประมวลผลเพื่อเตรียมการ Build ได้

### 1.2 ผลการเชื่อมต่อ Wi-Fi และการเริ่มต้นบริการ

จาก Serial Log บอร์ดเชื่อมต่อ Access Point สำเร็จ และได้รับข้อมูลเครือข่ายดังนี้:

| รายการ | ผลการทดลอง |
| :--- | :--- |
| สถานะ Wi-Fi | Connected |
| IP Address | `10.14.195.186` |
| Gateway | `10.14.195.187` |
| RSSI | `-48 dBm` |
| ชื่อ Hostname mDNS | `esp32-node.local` |
| HTTP Server | Started on port `80` |

ข้อความสำคัญจาก Log:

```text
Wi-Fi Connected! IP Address: 10.14.195.186
mDNS initialized! Hostname: http://esp32-node.local
HTTP Server started on port 80
Ready! Test with: curl http://esp32-node.local/api/status
```

### 1.3 ตรวจสอบ mDNS ด้วยคำสั่ง `ping`

```powershell
ping esp32-node.local
```

ผลการทดสอบ:

```text
Pinging esp32-node.local [10.14.195.186] with 32 bytes of data:
Reply from 10.14.195.186: bytes=32 time=145ms TTL=64
Reply from 10.14.195.186: bytes=32 time=235ms TTL=64
Reply from 10.14.195.186: bytes=32 time=161ms TTL=64
Reply from 10.14.195.186: bytes=32 time=162ms TTL=64

Packets: Sent = 4, Received = 4, Lost = 0 (0% loss)
Minimum = 145ms, Maximum = 235ms, Average = 175ms
```

จึงสรุปได้ว่าเครื่องทดสอบสามารถ Resolve ชื่อ `esp32-node.local` เป็น IP Address `10.14.195.186` ได้สำเร็จ

### 1.4 อ่านค่าสถานะผ่าน `GET /api/status`

```powershell
curl.exe -X GET http://esp32-node.local/api/status
```

ผลการตอบกลับ:

```json
{"pot_raw":2385,"free_heap":217808,"led":false}
```

| รายการ | ค่า |
| :--- | ---: |
| ค่า Potentiometer (`pot_raw`) | 2385 |
| Free Heap | 217808 bytes |
| สถานะ LED | `false` (ดับ) |

### 1.5 ควบคุม LED ผ่าน `POST /api/led`

| คำสั่ง | ผลตอบกลับ | สถานะ LED |
| :--- | :--- | :--- |
| `{"state": true}` | `{"result":"success"}` | ติด |
| `{"state": false}` | `{"result":"success"}` | ดับ |

Serial Log ยืนยันการรับคำสั่งเปิดไฟ:

```text
LED (GPIO 2) switched to: ON
```

---

## 2. คำตอบคำถามท้ายการทดลอง

### ข้อ 1: วิเคราะห์ขนาด HTTP Header และ JSON Body

ตัวอย่างข้อมูล JSON ที่ได้รับคือ:

```json
{"pot_raw":2385,"free_heap":217808,"led":false}
```

เมื่อนับข้อมูลตามผลลัพธ์นี้ จะได้:

- **ขนาด JSON Body:** 47 ไบต์
- **ค่า `Content-Length`:** 47 ไบต์
- **ขนาด HTTP Response Header โดยประมาณ:** 71 ไบต์
- **ขนาดรวม Header และ Body:** 118 ไบต์

การคำนวณขนาด Header ใช้ `\r\n` ขนาด 2 ไบต์เป็นตัวจบแต่ละบรรทัด:

| ส่วนประกอบ | การคำนวณ | ขนาด |
| :--- | :--- | ---: |
| Status Line | `HTTP/1.1 200 OK\r\n` | 17 ไบต์ |
| `Content-Type` | `Content-Type: application/json\r\n` | 32 ไบต์ |
| `Content-Length` | `Content-Length: 47\r\n` | 20 ไบต์ |
| บรรทัดว่างคั่น Header กับ Body | `\r\n` | 2 ไบต์ |
| **รวม Header** |  | **71 ไบต์** |

ดังนั้นข้อมูลที่ส่งกลับทั้งหมดมีขนาดประมาณ `71 + 47 = 118` ไบต์ ทั้งนี้ค่าอาจเปลี่ยนได้หากจำนวนหลักของค่า `pot_raw` หรือ `free_heap` เปลี่ยนแปลง

### ข้อ 2: ผลกระทบเมื่อไม่รองรับ mDNS หรือปิดกั้น UDP 5353

หากเครื่องไคลเอนต์ไม่รองรับ mDNS หรือ Firewall ปิดกั้น UDP พอร์ต 5353 เครื่องจะไม่สามารถแปลงชื่อ `esp32-node.local` เป็น IP Address ได้ แม้ ESP32 จะเชื่อมต่อ Wi-Fi และเปิด HTTP Server สำเร็จแล้วก็ตาม ผลคือคำสั่ง `ping` และ HTTP Request ที่เรียกผ่านชื่อ Hostname จะไม่สามารถทำงานได้

แนวทางแก้ไข:

1. ใช้ IP Address ของ ESP32 โดยตรง เช่น `http://10.14.195.186/api/status`
2. ตั้ง DHCP Reservation ให้บอร์ดได้รับ IP เดิมเสมอ
3. เพิ่มรายการ IP และ Hostname ลงในไฟล์ Hosts ของเครื่องไคลเอนต์
4. อนุญาต UDP พอร์ต 5353 และ Multicast mDNS ใน Windows Firewall
5. ตรวจสอบให้เครื่องไคลเอนต์กับ ESP32 อยู่ในเครือข่ายเดียวกัน และใช้โปรไฟล์ Private Network

### ข้อ 3: เหตุผลที่ต้องเรียก `cJSON_Delete(root)` และ `cJSON_free(resp)`

ฟังก์ชัน cJSON จัดสรรหน่วยความจำแบบ Dynamic Allocation บน Heap โดย `root` เป็นโครงสร้าง JSON Tree และ `resp` เป็นบัฟเฟอร์สตริงที่ได้จาก `cJSON_PrintUnformatted`

- `cJSON_Delete(root)` ใช้คืนหน่วยความจำของโหนด JSON ทั้งหมด
- `cJSON_free(resp)` ใช้คืนหน่วยความจำของสตริง JSON ที่สร้างขึ้น

หากไม่คืนหน่วยความจำหลังประมวลผลแต่ละ Request จะเกิด Memory Leak ทำให้ค่า Free Heap ลดลงเรื่อย ๆ เมื่อมีคำขอเข้ามาจำนวนมาก และอาจทำให้ ESP32 เกิด Out of Memory, ระบบค้าง หรือรีสตาร์ตได้

---

## 3. สรุปผลการทดลอง

ระบบสามารถเชื่อมต่อ Wi-Fi, ประกาศชื่อผ่าน mDNS และเปิด HTTP REST Server บนพอร์ต 80 ได้สำเร็จ การเรียก `ping esp32-node.local` สามารถ Resolve ไปยัง `10.14.195.186` โดยไม่มี Packet Loss จากการทดสอบ 4 ครั้ง นอกจากนี้ API สามารถอ่านค่า Potentiometer และ Free Heap ผ่าน `GET /api/status` รวมถึงควบคุม LED GPIO 2 ผ่านคำสั่ง `POST /api/led` ได้ตามที่กำหนด
