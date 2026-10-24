# ใบงานการทดลองที่ 10.1
### การพัฒนาระบบค้นหาอุปกรณ์ด้วย mDNS และเว็บเซิร์ฟเวอร์ควบคุมอุปกรณ์ผ่าน HTTP RESTful API บน ESP-IDF

---

## 1. บันทึกผลการทดลอง (กิจกรรมที่ 10-1.5 การทดสอบและตรวจพิสูจน์)

### 1.1 ตรวจสอบ mDNS ด้วยคำสั่ง ping
- **คำสั่ง:**
  ```powershell
  ping esp32-node.local
  ```
- **ผลลัพธ์จาก Terminal:**
  ```text
  Pinging esp32-node.local [192.168.1.8] with 32 bytes of data:
  Reply from 192.168.1.8: bytes=32 time=1412ms TTL=64
  Reply from 192.168.1.8: bytes=32 time=607ms TTL=64
  Reply from 192.168.1.8: bytes=32 time=42ms TTL=64
  Reply from 192.168.1.8: bytes=32 time=1657ms TTL=64

  Ping statistics for 192.168.1.8:
    Packets: Sent = 4, Received = 4, Lost = 0 (0% loss),
  Approximate round trip times in milli-seconds:
    Minimum = 42ms, Maximum = 1657ms, Average = 929ms
  ```
- **หมายเลข IP Address ที่ Resolve ได้:** `192.168.1.8`
- **ภาพบันทึกหน้าจอผลการ Ping:**

![alt text](image.png)

---

### 1.2 ทดสอบอ่านค่าเซนเซอร์ผ่าน cURL (`GET /api/status`)
- **คำสั่ง:**
  ```powershell 
  curl.exe -X GET http://esp32-node.local/api/status
  ```
- **ผลลัพธ์ JSON Payload:**
  ```json
  {"pot_raw":4095,"free_heap":213176,"led":false}
  ```

---

### 1.3 ทดสอบสั่งเปิด-ปิด LED ผ่าน cURL หรือ PowerShell (`POST /api/led`)

- **คำสั่งสั่งเปิดไฟ LED (`state: true`):**
  ```powershell
  curl.exe -s -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d '{\"state\": true}'
  ```
  - **ข้อความตอบกลับ:** `{"result":"success"}`
  - **สถานะหลอดไฟ LED บนบอร์ดจริง:** [✅] ติด (ON)  [ ] ดับ (OFF)

![alt text](<image (1).png>)

- **คำสั่งสั่งปิดไฟ LED (`state: false`):**
  ```powershell
  curl.exe -s -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d '{\"state\": false}'
  ```
  - **ข้อความตอบกลับ:** `{"result":"success"}`
  - **สถานะหลอดไฟ LED บนบอร์ดจริง:** [ ] ติด (ON)  [✅] ดับ (OFF)

![alt text](<image (2).png>)

---

## 2. คำถามท้ายการทดลอง

### ข้อ 1: การวิเคราะห์ HTTP Response Header และขนาดข้อมูลด้วย `curl -i`
นำผลการรัน `curl.exe -i http://esp32-node.local/api/status` มาแปะ พร้อมวิเคราะห์ว่า HTTP Header มีขนาดกี่ไบต์ และข้อมูล JSON มีขนาดกี่ไบต์

- **ผลการรัน `curl -i`:**
  ```http
  HTTP/1.1 200 OK
  Content-Type: application/json
  Content-Length: 47

  {"pot_raw":4095,"free_heap":213176,"led":false}
  ```
- **การวิเคราะห์ขนาดข้อมูล:**
  - **ขนาด HTTP Header:** 68 ไบต์
  - **ขนาดข้อมูล JSON:** 47 ไบต์
  - **บทวิเคราะห์:** ตัวส่วนหัวข้อมูลมีขนาด 68 ไบต์ ซึ่งใหญ่กว่าตัวข้อมูลจริงที่ส่งเพียง 47 ไบต์ ทำให้กินแบนด์วิดท์โดยไม่จำเป็น จึงไม่ค่อยเหมาะกับการส่งข้อมูลบ่อยๆ บนอุปกรณ์ขนาดเล็ก

---

### ข้อ 2: การแก้ไขปัญหาเมื่อไม่สามารถใช้งาน mDNS ได้
หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?

- **ผลกระทบ:** เครื่องคอมพิวเตอร์จะไม่สามารถค้นหาหรือแปลงชื่อเป็นหมายเลขไอพีได้ ทำให้เปิดใช้งานผ่านชื่อไม่ได้
- **แนวทางแก้ไข:** 
  1. ใช้งานผ่านหมายเลขไอพีของบอร์ดโดยตรง
  2. ตั้งค่าจับคู่ชื่อกับไอพีในระบบเครือข่ายหรือในเครื่องคอมพิวเตอร์
  3. เปิดอนุญาตพอร์ตในระบบความปลอดภัยหรือเปลี่ยนเครือข่ายให้เป็นแบบส่วนตัว

---

### ข้อ 3: การจัดการหน่วยความจำของ cJSON
เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?

- **คำตอบ:** เพราะตัวระบบมีการจองพื้นที่ในแรมตอนสร้างข้อมูลและแปลงเป็นข้อความ ถ้าใช้งานเสร็จแล้วไม่สั่งคืนพื้นที่ แรมจะถูกกินไปเรื่อยๆ จนเต็มและทำให้บอร์ดค้างหรือรีสตาร์ทเองได้

---
### Logs
```text
I (27) boot: ESP-IDF v6.1 2nd stage bootloader
I (27) boot: compile time Oct 24 2026 18:46:58
I (28) boot: Multicore bootloader
I (29) boot: chip revision: v3.1
I (31) boot.esp32: SPI Speed      : 40MHz
I (35) boot.esp32: SPI Mode       : DIO
I (39) boot.esp32: SPI Flash Size : 2MB
I (42) boot: Enabling RNG early entropy source...
I (47) boot: Partition Table:
I (49) boot: ## Label            Usage          Type ST Offset   Length
I (56) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (62) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (69) boot:  2 factory          factory app      00 00 00010000 00100000
I (75) boot: End of partition table
I (79) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=1e534h (124212) map
I (130) esp_image: segment 1: paddr=0002e55c vaddr=3ffb0000 size=01abch (  6844) load
I (133) esp_image: segment 2: paddr=00030020 vaddr=400d0020 size=9fa6ch (653932) map
I (367) esp_image: segment 3: paddr=000cfa94 vaddr=3ffb1abc size=02b9ch ( 11164) load
I (372) esp_image: segment 4: paddr=000d2638 vaddr=40080000 size=154c4h ( 87236) load
I (408) esp_image: segment 5: paddr=000e7b04 vaddr=50000000 size=00028h (    40) load
I (419) boot: Loaded app from partition at offset 0x10000
I (419) boot: Disabling RNG early entropy source...
I (429) cpu_start: Multicore app
I (438) cpu_start: GPIO 3 and 1 are used as console UART I/O pins
I (438) cpu_start: Pro cpu start user code
I (438) cpu_start: cpu freq: 160000000 Hz
I (440) app_init: Application information:
I (443) app_init: Project name:     Lab10-1_HTTP_REST_Server
I (449) app_init: App version:      b2d0a95
I (453) app_init: Compile time:     Oct 24 2026 18:46:34
I (458) app_init: ELF file SHA256:  e2bd6e89e...
I (462) app_init: ESP-IDF:          v6.1
I (466) efuse_init: Min chip rev:     v0.0
I (470) efuse_init: Max chip rev:     v3.99
I (474) efuse_init: Chip rev:         v3.1
I (478) heap_init: Initializing. RAM available for dynamic allocation:
I (484) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (489) heap_init: At 3FFB9A48 len 000265B8 (153 KiB): DRAM
I (494) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (499) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (505) heap_init: At 400954C4 len 0000AB3C (42 KiB): IRAM
I (511) spi_flash: detected chip: generic
I (514) spi_flash: flash io: dio
W (517) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (531) main_task: Started on CPU0
I (531) main_task: Calling app_main()
I (561) HTTP_REST_LAB: mDNS initialized! Hostname: http://esp32-node.local
I (561) HTTP_REST_LAB: ADC Initialized on GPIO 34 (Channel 6)
I (571) wifi:wifi driver task: 3ffc28d8, prio:23, stack:6656, core=0
I (581) wifi:wifi firmware version: e12a754
I (581) wifi:wifi certification version: v7.0
I (581) wifi:config NVS flash: enabled
I (581) wifi:config nano formatting: disabled
I (591) wifi:Init data frame dynamic rx buffer num: 32
I (591) wifi:Init static rx mgmt buffer num: 5
I (591) wifi:Init management short buffer num: 32
I (601) wifi:Init dynamic tx buffer num: 32
I (601) wifi:Init static rx buffer size: 1600
I (611) wifi:Init static rx buffer num: 10
I (611) wifi:Init dynamic rx buffer num: 32
I (621) wifi_init: rx ba win: 6
I (621) wifi_init: accept mbox: 6
I (621) wifi_init: tcpip mbox: 32
I (621) wifi_init: udp mbox: 6
I (631) wifi_init: tcp mbox: 6
I (631) wifi_init: tcp tx win: 5760
I (631) wifi_init: tcp rx win: 5760
I (631) wifi_init: tcp mss: 1440
I (641) wifi_init: WiFi IRAM OP enabled
I (641) wifi_init: WiFi RX IRAM OP enabled
I (671) phy_init: phy_version 4863,a3a4459,Oct 28 2025,14:30:06
I (781) phy_init: Saving new calibration data due to checksum failure or outdated calibration data, mode(0)
I (801) wifi:mode : sta (84:1f:e8:39:bd:64)
I (801) wifi:enable tsf
I (811) HTTP_REST_LAB: Connecting to SSID: FBT4402_2.4 ...
I (3231) HTTP_REST_LAB: Retrying Wi-Fi connection (1/5)...
I (3311) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1, snd_ch_cfg:0x0
I (3311) wifi:state: init -> auth (0xb0)
I (7611) wifi:state: auth -> init (0x200)
I (7631) HTTP_REST_LAB: Retrying Wi-Fi connection (2/5)...
I (10051) HTTP_REST_LAB: Retrying Wi-Fi connection (3/5)...
I (10071) wifi:state: init -> auth (0xb0)
I (10761) wifi:state: auth -> assoc (0x0)
I (10781) wifi:state: assoc -> run (0x10)
I (11321) wifi:connected with FBT4402_2.4, aid = 4, channel 2, BW20, bssid = 34:4a:c3:26:26:d6
I (11331) wifi:security: WPA3-SAE HUNT_AND_PECK, phy: bgn, rssi: -50, cipher(pairwise:0x3, group:0x3), pmf:1
I (11341) wifi:pm start, type: 1

I (11341) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (11911) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (13661) wifi:<ba-add>idx:0 (ifx:0, 34:4a:c3:26:26:d6), tid:5, ssn:5, winSize:64
I (22741) esp_netif_handlers: sta ip: 192.168.1.8, mask: 255.255.255.0, gw: 192.168.1.1
I (22741) HTTP_REST_LAB: Wi-Fi Connected! IP Address: 192.168.1.8
I (22741) HTTP_REST_LAB: Connected to AP successfully!
I (22751) HTTP_REST_LAB: HTTP Server started on port 80
I (22751) HTTP_REST_LAB: Ready! Test with: curl.exe http://esp32-node.local/api/status
I (22761) main_task: Returned from app_main()

(fx:0, 34:4a:c3:26:26:d6), tid:0, ssn:13, winSize:64
HTTP_REST_LAB: LED (GPIO 2) switched to: ON
I (891691) HTTP_REST_LAB: LED (GPIO 2) switched to: OFF
```