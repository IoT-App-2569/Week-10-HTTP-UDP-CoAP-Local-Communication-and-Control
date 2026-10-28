# รายงานผลการทดลองและเฉลยคำถามท้ายการทดลอง: Lab 10.1
## การพัฒนาระบบค้นหาอุปกรณ์ด้วย mDNS และเว็บเซิร์ฟเวอร์ควบคุมอุปกรณ์ผ่าน HTTP RESTful API บน ESP-IDF

---

### 1. โครงสร้างโปรเจกต์ (Project Structure)
```
Lab10-1_mDNS_Discovery_and_HTTP_REST_Server/
├── CMakeLists.txt
├── sdkconfig.defaults
├── test_http.py
└── main/
    ├── CMakeLists.txt
    ├── idf_component.yml
    └── Lab10-1_HTTP_REST_Server.c
```

### 2. แผนภาพการต่อวงจรฮาร์ดแวร์ (Hardware Wiring)
| อุปกรณ์ | ขาอุปกรณ์ | ขาต่อบน ESP32 | หน้าที่ |
| :--- | :--- | :--- | :--- |
| **LED (Onboard / External)** | Anode (+) | **GPIO 2** | เอาต์พุตแสดงสถานะการสั่งงาน |
| **Potentiometer 10k** | Wiper (ขากลาง) | **GPIO 34** (ADC1_CH6) | อนาล็อกอินพุตอ่านค่าความต่างศักย์ |
| **Potentiometer 10k** | VCC / GND | **3.3V / GND** | แรงดันไฟเลี้ยงและกราวด์ |

---

### 3. ผลการทดลองและคำสั่งทดสอบ (Verification)
#### 3.1 การตรวจสอบ mDNS (`ping esp32-node.local`)
```powershell
ping esp32-node.local
```
**ผลลัพธ์ที่ได้:**
```text
Pinging esp32-node.local [192.168.1.181] with 32 bytes of data:
Reply from 192.168.1.181: bytes=32 time=4ms TTL=255
Reply from 192.168.1.181: bytes=32 time=3ms TTL=255
Reply from 192.168.1.181: bytes=32 time=3ms TTL=255
Reply from 192.168.1.181: bytes=32 time=5ms TTL=255

Ping statistics for 192.168.1.181:
    Packets: Sent = 4, Received = 4, Lost = 0 (0% loss),
Approximate round trip times in milli-seconds:
    Minimum = 3ms, Maximum = 5ms, Average = 3ms
```

#### 3.2 การอ่านค่าสถานะเซนเซอร์ผ่าน cURL (`GET /api/status`)
```powershell
curl.exe -i http://esp32-node.local/api/status
```
**ผลลัพธ์ HTTP Response เต็มรูปแบบ:**
```http
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 44
Connection: close

{"pot_raw":15,"free_heap":213848,"led":false}
```

#### 3.3 การควบคุมหลอดไฟ LED (`POST /api/led`)
* สั่งเปิดไฟ LED (`state: true`):
```powershell
Invoke-RestMethod -Uri "http://esp32-node.local/api/led" -Method POST -ContentType "application/json" -Body '{"state": true}'
```
* สั่งปิดไฟ LED (`state: false`):
```powershell
Invoke-RestMethod -Uri "http://esp32-node.local/api/led" -Method POST -ContentType "application/json" -Body '{"state": false}'
```
**ผลลัพธ์ที่ได้:** หลอดไฟ LED บนบอร์ด (GPIO 2) ติดสว่าง และ ดับ ตามลำดับ พร้อมข้อความตอบกลับ `{"result":"success"}`

---

### 4. บันทึกผลการทดลองและเฉลยคำถามท้ายการทดลอง (Lab Questions & Answers)

#### ข้อที่ 1: นำผลการรัน `curl -i` (โหมด verbose เพื่อดู HTTP Response Header) มาแปะในรายงาน พร้อมวิเคราะห์ว่า HTTP Header มีขนาดกี่ไบต์ และข้อมูล JSON มีขนาดกี่ไบต์

**ผลการรัน `curl -i`:**
```http
HTTP/1.1 200 OK\r\n
Content-Type: application/json\r\n
Content-Length: 44\r\n
Connection: close\r\n
\r\n
{"pot_raw":15,"free_heap":213848,"led":false}
```

**การวิเคราะห์ขนาดของข้อมูล (Packet Size Breakdown):**
1. **ขนาดของ HTTP Header:**
   * บรรทัด `HTTP/1.1 200 OK\r\n` = 17 ไบต์
   * บรรทัด `Content-Type: application/json\r\n` = 32 ไบต์
   * บรรทัด `Content-Length: 44\r\n` = 20 ไบต์
   * บรรทัด `Connection: close\r\n` = 19 ไบต์
   * บรรทัดว่างปิดท้ายส่วนหัว `\r\n` = 2 ไบต์
   * **รวมขนาด HTTP Response Header $\approx 90$ ไบต์** (หากมี Header อื่นเพิ่ม เช่น Server/Date อาจสูงถึง 120–160 ไบต์)
2. **ขนาดของข้อมูล JSON Payload:**
   * สตริง JSON: `{"pot_raw":15,"free_heap":213848,"led":false}`
   * **รวมขนาด JSON Payload = 44 ไบต์**

**บทวิเคราะห์เชิงวิศวกรรม:**
* สัดส่วนขนาด Header ต่อ Payload อยู่ที่ประมาณ $90 : 44 \approx 2.05 : 1$ หมายความว่า **ขนาดส่วนหัวของโพรโทคอลมีขนาดใหญ่กว่าเนื้อหาข้อมูลจริงกว่า 2 เท่าตัว**
* หากรวม TCP Header (20 ไบต์) + IP Header (20 ไบต์) ข้อมูลขนาด 44 ไบต์ต้องใช้แพ็กเก็ตขนาดกว่า 170 ไบต์ แสดงให้เห็นว่า HTTP มี Protocol Overhead ที่ค่อนข้างสูง ไม่เหมาะกับการส่งข้อมูลถี่ๆ จากอุปกรณ์ IoT ที่มีข้อจำกัดเรื่องพลังงานและแบนด์วิดท์

---

#### ข้อที่ 2: หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?

**ผลกระทบ:**
1. คอมพิวเตอร์จะไม่สามารถค้นหาหรือแปลงชื่อโดเมนเสมือน `esp32-node.local` เป็นหมายเลข IP Address ได้
2. เมื่อผู้ใช้พิมพ์ `ping esp32-node.local` หรือเข้าเว็บผ่านเบราว์เซอร์ จะเกิดข้อผิดพลาด เช่น `Ping request could not find host esp32-node.local` หรือ `ERR_NAME_NOT_RESOLVED`
3. ไม่สามารถเข้าถึง RESTful API ผ่านชื่อ Hostname ได้

**แนวทางการแก้ไข:**
1. **เข้าถึงด้วยหมายเลข IP Address โดยตรง (Direct IP Access):**
   * ตรวจสอบ IP จาก Serial Monitor ของ ESP32 (เช่น `192.168.1.181`) แล้วเรียกใช้งานผ่าน `http://192.168.1.181/api/status`
2. **ตั้งค่า Static DHCP Reservation บน Wi-Fi Router:**
   * ล็อกหมายเลข IP ของ ESP32 ตาม MAC Address เพื่อให้ ESP32 ได้รับ IP เดิมเสมอ
3. **กำหนด Local DNS บนเราเตอร์หรือ DNS Server (dnsmasq / Pi-hole):**
   * แม็ปชื่อ `esp32-node.local` หรือ `esp32.home` เข้ากับ IP ในระบบ Unicast DNS ของวงแลน
4. **แก้ไขไฟล์ Hosts ของระบบปฏิบัติการ:**
   * เพิ่มบรรทัด `192.168.1.181 esp32-node.local` ลงในไฟล์ `C:\Windows\System32\drivers\etc\hosts` (Windows) หรือ `/etc/hosts` (Linux/macOS)
5. **ปลดล็อกไฟร์วอลล์และเปลี่ยนสถานะเครือข่าย:**
   * ตรวจสอบว่าเครือข่าย Wi-Fi บน Windows ถูกตั้งเป็น **"Private Network"** และเปิดอนุญาต (Allow Rule) สำหรับโปรโตคอล mDNS ผ่านพอร์ต **UDP 5353**
   * ติดตั้งมิดเดิลแวร์ mDNS เช่น **Bonjour Service** สำหรับ Windows หรือตรวจสอบว่า **avahi-daemon** เปิดทำงานสำหรับ Linux

---

#### ข้อที่ 3: เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?

**เหตุผล:**
1. **การจัดสรรหน่วยความจำแบบไดนามิกบนฮีป (Heap Dynamic Allocation):**
   * ฟังก์ชัน `cJSON_CreateObject()`, `cJSON_Add...()`, และ `cJSON_Parse()` จะจองหน่วยความจำบน Heap ด้วย `malloc()` สำหรับทุกๆ โหนดของโครงสร้างต้นไม้ (JSON Object Node Tree)
   * ฟังก์ชัน `cJSON_PrintUnformatted()` หรือ `cJSON_Print()` จะจองบัฟเฟอร์สตริงบน Heap เพื่อเก็บข้อความ JSON ที่สร้างขึ้น
2. **ป้องกันปัญหาหน่วยความจำรั่วไหล (Memory Leak):**
   * ไมโครคอนโทรลเลอร์ ESP32 มีหน่วยความจำแรม (Internal SRAM) จำกัด (เหลือใช้งานจริงสำหรับแอปพลิเคชันประมาณ 200–250 KB)
   * หากไม่คืนหน่วยความจำด้วย `cJSON_Delete(root)` และ `free(resp)` / `cJSON_free(resp)` ทุกครั้งที่เซิร์ฟเวอร์ประมวลผลคำขอ HTTP เข้ามา หน่วยความจำ Heap จะลดลงเรื่อยๆ ทุก Request
3. **ผลกระทบหากเกิด Memory Leak:**
   * ภายในเวลาไม่นาน Heap จะหมด (`ESP_ERR_NO_MEM`)
   * ฟังก์ชัน `malloc` ครั้งถัดไปจะส่งกลับ `NULL` ทำให้โปรแกรมทำงานผิดพลาด เกิด Memory Crash (Guru Meditation Error) และ ESP32 จะรีสตาร์ตตัวเองด้วย Panic / Watchdog Timeout ระบบจึงไม่สามารถทำงานได้อย่างต่อเนื่อง (Reliability & Uptime Failure)
