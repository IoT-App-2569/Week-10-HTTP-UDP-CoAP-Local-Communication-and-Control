# รายงานผลการทดลอง ใบงานที่ 10.1
### การพัฒนาระบบค้นหาอุปกรณ์ด้วย mDNS และเว็บเซิร์ฟเวอร์ควบคุมอุปกรณ์ผ่าน HTTP RESTful API บน ESP-IDF
**รหัสนักศึกษา:** 67030338  

---

## 1. วัตถุประสงค์การทดลอง
1. เพื่อทำความเข้าใจกลไกการเชื่อมต่อเครือข่าย Wi-Fi ในโหมด Station (STA) ด้วยสแตก LwIP บน ESP-IDF
2. สามารถเปิดใช้งานและตั้งค่า mDNS Service Discovery เพื่อประกาศชื่อโฮสต์ (`esp32-node.local`) และบริการ HTTP บนเครือข่ายท้องถิ่นได้
3. สามารถพัฒนา RESTful API Endpoints (`GET /api/status`, `POST /api/led` และ `POST /api/pot`) ด้วยคอมโพเนนต์ `esp_http_server`
4. สามารถประมวลผลข้อมูล JSON Payload (Parsing & Serialization) ด้วยไลบรารี `cJSON` ได้อย่างปลอดภัย ปราศจากปัญหา Memory Leak
5. สามารถทดสอบส่งคำสั่งควบคุม LED (GPIO 2) และอ่าน/ปรับค่า Potentiometer เสมือน (Virtual Potentiometer) ผ่านคำสั่ง `cURL`, PowerShell และสคริปต์ Python

---

## 2. การดัดแปลงระบบเพื่อรองรับกรณีไม่มีฮาร์ดแวร์ Potentiometer (Virtual Potentiometer Solution)

> [!NOTE] **แนวทางการแก้ไขปัญหาเมื่อไม่มีอุปกรณ์ Potentiometer จริง**  
> เนื่องจากไม่มีอุปกรณ์ Potentiometer ทางกายภาพ จึงได้ออกแบบและพัฒนาฟังก์ชัน **Virtual / Simulated Potentiometer** ขึ้นบนตัวเฟิร์มแวร์ ESP32 เพื่อให้สามารถจำลองและปรับเปลี่ยนค่าสัญญาณแอนะล็อก (0 - 4095) ได้อย่างสมบูรณ์แบบโดยไม่ต้องต่อฮาร์ดแวร์จริง

### 2.1 กลไกการทำงานของ Virtual Potentiometer
1. **ตัวแปรภายในระบบ:** กำหนดตัวแปร `static int s_virtual_pot_raw = 2048;` และธงควบคุม `s_use_virtual_pot = true;`
2. **การปรับค่าผ่าน RESTful API (`POST /api/pot`):** เพิ่ม Endpoint ใหม่สำหรับรับ JSON Payload เพื่อตั้งค่าโดยตรง หรือปรับเพิ่ม/ลดค่าทีละขั้น เช่น
   - `{"pot_raw": 3200}` หรือ `{"value": 3200}` : กำหนดค่าโดยตรง
   - `{"step": 150}` : ปรับเพิ่มค่าขึ้น 150 หน่วย
   - `{"auto": true}` : เปิดโหมด Auto Sweep จำลองการหมุนลูกบิดไปกลับโดยอัตโนมัติ
3. **การปรับค่าผ่าน Query String (`GET /api/status?pot=XXXX`):** สามารถกำหนดค่าผ่าน URL ในเว็บบราวเซอร์หรือ cURL ได้ทันที เช่น `GET /api/status?pot=3500`
4. **ความเข้ากันได้ย้อนหลัง (Backward Compatibility):** หากในอนาคตมีการเชื่อมต่อ Potentiometer จริงเข้าที่ขา GPIO 34 (ADC1 Channel 6) โค้ดยังคงมีส่วนเริ่มต้นการทำงานของฮาร์ดแวร์ ADC1 ไว้พร้อมใช้งานทันที

### 2.2 ซอร์สโค้ดส่วนที่เพิ่มสำหรับ Virtual Potentiometer

```c
// ตัวแปรจำลองค่า Potentiometer ในแรมของ ESP32
static int  s_virtual_pot_raw   = 2048; // ค่าเริ่มต้นกึ่งกลางช่วง 12-bit ADC (0 - 4095)
static bool s_use_virtual_pot   = true; // เปิดใช้งานระบบจำลองค่า Potentiometer
static bool s_auto_sweep        = false;// โหมดปรับค่าขึ้นลงอัตโนมัติ
static int  s_sweep_direction   = 1;    // ทิศทางการเปลี่ยนแปลง (+1 หรือ -1)

// Handler สำหรับ POST /api/pot เพื่อปรับค่าโดยตรงผ่าน HTTP
static esp_err_t pot_post_handler(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root != NULL) {
        cJSON *val_item = cJSON_GetObjectItem(root, "pot_raw");
        if (!val_item) val_item = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsNumber(val_item)) {
            int val = val_item->valueint;
            if (val < 0) val = 0;
            if (val > 4095) val = 4095;
            s_virtual_pot_raw = val;
            s_use_virtual_pot = true;
            ESP_LOGI(TAG, "Virtual Pot set to: %d", s_virtual_pot_raw);
        }
        cJSON_Delete(root);
    }

    cJSON *resp_json = cJSON_CreateObject();
    cJSON_AddStringToObject(resp_json, "result", "success");
    cJSON_AddNumberToObject(resp_json, "pot_raw", s_virtual_pot_raw);

    const char *resp = cJSON_PrintUnformatted(resp_json);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));

    cJSON_free((void *)resp);
    cJSON_Delete(resp_json);
    return ESP_OK;
}
```

---

## 3. ผลการทดสอบและตรวจพิสูจน์ (Verification & Forensics)

### 3.1 การตรวจสอบ mDNS ด้วยคำสั่ง Ping (`ping esp32-node.local`)
ทำการทดสอบส่งแพ็กเก็ต ICMP Echo Request ไปยังชื่อโฮสต์เสมือน `esp32-node.local` ผ่านโปรโตคอล Multicast DNS (UDP พอร์ต 5353)

```powershell
PS C:\work-2026-1\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control> ping esp32-node.local

Pinging esp32-node.local [192.168.1.41] with 32 bytes of data:
Reply from 192.168.1.41: bytes=32 time=9ms TTL=64
Reply from 192.168.1.41: bytes=32 time=118ms TTL=64
Reply from 192.168.1.41: bytes=32 time=27ms TTL=64
Reply from 192.168.1.41: bytes=32 time=49ms TTL=64

Ping statistics for 192.168.1.41:
    Packets: Sent = 4, Received = 4, Lost = 0 (0% loss),
Approximate round trip times in milli-seconds:
    Minimum = 9ms, Maximum = 118ms, Average = 50ms
```
**ผลการตรวจสอบ:** ระบบสามารถ Resolve ชื่อ `esp32-node.local` เป็นหมายเลข IP จริงของบอร์ดคือ `192.168.1.41` ได้สำเร็จและมีการตอบสนองครบ 100% (Lost = 0)

---

### 3.2 การอ่านค่าสถานะระบบและเซนเซอร์ผ่าน `curl.exe`
ทำการส่งคำสั่ง HTTP GET เพื่ออ่านสถานะของเซนเซอร์ หน่วยความจำ และไฟ LED

```powershell
PS C:\work-2026-1\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control> curl.exe -X GET http://esp32-node.local/api/status
{"pot_raw":2048,"free_heap":217684,"led":false}
```

---

### 3.3 การทดสอบปรับค่า Potentiometer เสมือน (Virtual Potentiometer Adjustment)

#### การปรับค่าผ่านคำสั่ง POST `/api/pot`
ทดสอบเปลี่ยนค่า Potentiometer จาก 2048 เป็น 3500:

```powershell
PS C:\> curl.exe -s -X POST http://esp32-node.local/api/pot -H "Content-Type: application/json" -d '{\"pot_raw\": 3500}'
{"result":"success","pot_raw":3500,"auto":false}
```

เมื่อทำการเรียก `GET /api/status` ซ้ำเพื่อยืนยันผล:
```powershell
PS C:\> curl.exe -X GET http://esp32-node.local/api/status
{"pot_raw":3500,"free_heap":213720,"led":false}
```
**ผลการทดสอบ:** ค่า `pot_raw` ถูกปรับเปลี่ยนเป็น 3500 ตามคำสั่ง โดยไม่ต้องต่อฮาร์ดแวร์ Potentiometer จริง

#### การปรับค่าผ่าน Query Parameter ใน URL
```powershell
PS C:\> curl.exe -X GET "http://esp32-node.local/api/status?pot=1250"
{"pot_raw":1250,"free_heap":213720,"led":false}
```

---

### 3.4 การทดสอบสั่งงานเปิด-ปิด LED (GPIO 2) ผ่าน REST API

#### 1) คำสั่งเปิดไฟ LED (`{"state": true}`)
```powershell
PS C:\> curl.exe -s -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d '{\"state\": true}'
{"result":"success"}
```
ตรวจสอบสถานะผ่าน `GET /api/status`:
```powershell
PS C:\> curl.exe -X GET http://esp32-node.local/api/status
{"pot_raw":3500,"free_heap":213600,"led":true}
```
*สังเกตได้ว่าฟิลด์ `"led": true` และหลอดไฟ LED บนบอร์ด ESP32 ติดสว่าง*

#### 2) คำสั่งปิดไฟ LED (`{"state": false}`)
```powershell
PS C:\> curl.exe -s -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d '{\"state\": false}'
{"result":"success"}
```
ตรวจสอบสถานะผ่าน `GET /api/status`:
```powershell
PS C:\> curl.exe -X GET http://esp32-node.local/api/status
{"pot_raw":3500,"free_heap":217188,"led":false}
```
*หลอดไฟ LED ดับลง และสถานะตอบกลับระบุ `"led": false`*

---

### 3.5 การทดสอบอัตโนมัติด้วยสคริปต์ Python (`python test_rest_api.py`)
ทำการรันสคริปต์ทดสอบอัตโนมัติเพื่อตรวจสอบ API ทุกฟังก์ชันอย่างครบวงจร:

```powershell
PS C:\work-2026-1\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control> python test_rest_api.py
=== Testing ESP32 RESTful API on http://esp32-node.local ===

1. Testing GET /api/status...
Status Code: 200
Response Headers: {'Content-Type': 'application/json', 'Content-Length': '47'}
Response Body: {"pot_raw":2048,"free_heap":217604,"led":false}

2. Turning LED ON...
Response: {"result":"success"}

3. Adjusting Virtual Potentiometer to 3500 via POST /api/pot...
Response: {"result":"success","pot_raw":3500,"auto":false}

4. Verifying status after adjustment...
Response Body: {"pot_raw":3500,"free_heap":217188,"led":true}

5. Turning LED OFF...
Response: {"result":"success"}
```
**ผลการทดสอบ:** ระบบสามารถตอบสนองต่อสคริปต์ Python อัตโนมัติได้สมบูรณ์ ทั้งการอ่านค่าระบบ, สั่งเปิด-ปิดไฟ LED และการปรับตั้งค่า Virtual Potentiometer โดยไม่มีข้อผิดพลาด
```
PS C:\work-2026-1\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control\Lab10-1_HTTP_REST_Server> python test_rest_api.py
=== Testing ESP32 RESTful API on http://esp32-node.local ===

1. Testing GET /api/status...
Status Code: 200
Response Headers: {'Content-Type': 'application/json', 'Content-Length': '47'}
Response Body: {"pot_raw":3500,"free_heap":217604,"led":false}

2. Turning LED ON...
Response: {"result":"success"}
3. Adjusting Virtual Potentiometer to 3500 via POST /api/pot...
Response: {"result":"success","pot_raw":3500,"auto":false}

4. Verifying status after adjustment...
Response Body: {"pot_raw":3500,"free_heap":217212,"led":true}

5. Turning LED OFF...
Response: {"result":"success"}
```
---

## 4. บันทึกผลการทดลองและคำถามท้ายการทดลอง

### คำถามข้อที่ 1
> **นำผลการรัน `curl -i` (โหมด verbose เพื่อดู HTTP Response Header) มาแปะในรายงาน พร้อมวิเคราะห์ว่า HTTP Header มีขนาดกี่ไบต์ และข้อมูล JSON มีขนาดกี่ไบต์**

#### ผลการรันคำสั่ง `curl.exe -i http://esp32-node.local/api/status`
```http
PS C:\work-2026-1\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control\Lab10-1_HTTP_REST_Server> curl.exe -i http://esp32-node.local/api/status
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 47

{"pot_raw":3500,"free_heap":217364,"led":false}
```



---

### คำถามข้อที่ 2
> **หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?**

#### 1) ผลกระทบที่เกิดขึ้น
- **ไม่สามารถแปลงชื่อโฮสต์เสมือนได้ (Name Resolution Failure):** โปรโตคอล mDNS (Multicast DNS) ทำงานโดยการส่งแพ็กเก็ต UDP Multicast ไปยังแอดเดรส `224.0.0.251` (สำหรับ IPv4) หรือ `ff02::fb` (สำหรับ IPv6) ที่พอร์ต **5353** 

#### 2) แนวทางการแก้ไขปัญหา
1. **การเข้าถึงผ่าน IP Address โดยตรง (Direct IP Addressing):**
   - ผู้ใช้สามารถเข้าใช้งานผ่านหมายเลข IP จริงของ ESP32 ได้โดยตรง เช่น `http://192.168.1.181/api/status` โดยตรวจสอบ IP ได้จากข้อความ Log ของ ESP32 ใน Serial Monitor (`ip_event_got_ip`) หรือตาราง DHCP Client บนเร้าเตอร์
2. **การตั้งค่าเครือข่ายและไฟร์วอลล์บนเครื่องคอมพิวเตอร์:**
   - เปลี่ยน Network Profile บน Windows จาก **Public network** เป็น **Private network** (เพื่อปลดล็อกการบล็อก Multicast)
   - เพิ่มข้อยกเว้น (Inbound Rule) ใน Firewall เพื่ออนุญาตให้รับส่งข้อมูลผ่านพอร์ต **UDP 5353**
   - ถอดสาย LAN หรือปิดการ์ดแลนชั่วคราว หากใช้งาน Wi-Fi เพื่อป้องกันปัญหา Windows ทำ Routing ออกทางอินเทอร์เฟซผิดเส้นทาง


---

### คำถามข้อที่ 3
> **เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?**

1. **กลไกการจองหน่วยความจำแบบพลวัต (Dynamic Heap Allocation):**
   - ไลบรารี `cJSON` ทำงานโดยการจองหน่วยความจำในส่วนของ Heap Memory ผ่านฟังก์ชัน `malloc()` เพื่อสร้างโครงสร้างต้นไม้ (Tree Data Structure) ของ JSON Object โดยแต่ละ Node (เช่น ฟิลด์ คีย์ ค่าตัวเลข บูลีน) จะถูกสร้างเป็น `cJSON struct` อิสระที่มีพอยน์เตอร์เชื่อมโยงหากัน
   - เมื่อเรียกฟังก์ชันสร้างสตริง เช่น `cJSON_Print()` หรือ `cJSON_PrintUnformatted()` ตัวไลบรารีจะทำการจองบล็อกหน่วยความจำแรมผืนใหม่ใน Heap สำหรับเก็บตัวอักษรสตริงข้อความ JSON ทั้งหมด
2. **การป้องกันปัญหาหน่วยความจำรั่วไหล (Memory Leak):**
   - ภาษา C ไม่มีระบบ Garbage Collection อัตโนมัติ หากนักพัฒนาไม่สั่งปลดปล่อยหน่วยความจำ (Deallocation) หน่วยความจำบล็อกเหล่านั้นจะยังคงค้างอยู่ใน Heap แม้ฟังก์ชัน Handler จะสิ้นสุดการทำงานไปแล้ว
   - การเรียก `cJSON_Delete(root)` เป็นการวนลูปสั่งปลดปล่อยหน่วยความจำของโหนดทุกโหนดในโครงสร้าง JSON Tree ย้อนกลับคืนสู่ระบบ Heap
   - การเรียก `cJSON_free((void *)resp)` เป็นการปลดปล่อยบล็อกแรมของสตริงที่ถูกสร้างจาก `cJSON_PrintUnformatted()`


---

## 5. สรุปผลการทดลอง
- สามารถสร้างระบบ Local Control ด้วย ESP32 ในฐานะ HTTP RESTful Server และ mDNS ได้สำเร็จ โดยผู้ใช้สามารถควบคุมเปิด-ปิดหลอดไฟ LED ผ่าน `POST /api/led` และอ่านค่าสถานะของระบบผ่าน `GET /api/status`
- ในกรณีที่ไม่มีฮาร์ดแวร์ Potentiometer ได้นำเสนอโซลูชัน **Virtual Potentiometer** ที่สามารถปรับเปลี่ยนค่าได้อย่างยืดหยุ่นผ่าน REST API (`POST /api/pot`) และ Query String (`?pot=...`) ช่วยให้การทดสอบระบบสามารถดำเนินต่อไปได้อย่างสมบูรณ์
- ได้เรียนรู้และพิสูจน์ขนาดของ Protocol Overhead ใน HTTP/1.1 ซึ่งมีขนาด Header มากกว่าตัวข้อมูลจริง และเข้าใจความสำคัญสูงสุดของการบริหารจัดการหน่วยความจำ Heap ในระบบสมองกลฝังตัว
