# ใบงานการทดลองที่ 10.3
### การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา

---

## 1. บันทึกผลการทดลอง (กิจกรรมที่ 10-3.5 การทดสอบด้วยสคริปต์ `test_coap.py`)

- **คำสั่ง:**
  ```powershell
  python test_coap.py
  ```
- **ผลลัพธ์จากการรันสคริปต์ใน Terminal:**
  ```text
  PS C:\Users\tulbp\Desktop\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control\67030298\Lab10-3_CoAP_Server> python test_coap.py

  --- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
  Resource Directory (CoRE Link Format):
  </sensor/pot>,</actuator/led>

  --- 2. Testing CoAP GET /sensor/pot ---
  Potentiometer Value: 4095 (Code: 2.05 Content)

  --- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
  LED ON Response Code: 2.04 Changed

  --- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
  LED OFF Response Code: 2.04 Changed
  ```

### ผลการทดสอบ 4 ฟังก์ชันหลัก:
1. **Resource Discovery (`GET /.well-known/core`):**
   - Payload ที่ได้รับ (CoRE Link Format): `</sensor/pot>,</actuator/led>`
   - Response Code: `2.05` Content

2. **ทดสอบอ่านค่าเซนเซอร์ (`GET /sensor/pot`):**
   - ค่า Potentiometer: `4095`
   - Response Code: `2.05` Content

3. **ทดสอบสั่งเปิดไฟ LED (`PUT /actuator/led` ส่ง '1'):**
   - Response Code: `2.04` Changed
   - สถานะหลอดไฟ LED บนบอร์ด: [✅] ติด (ON)  [ ] ดับ (OFF)

4. **ทดสอบสั่งปิดไฟ LED (`PUT /actuator/led` ส่ง '0'):**
   - Response Code: `2.04` Changed
   - สถานะหลอดไฟ LED บนบอร์ด: [ ] ติด (ON)  [✅] ดับ (OFF)

- **ภาพบันทึกหน้าจอผลการทดสอบ:**

![alt text](image.png)

---

## 2. คำถามท้ายการทดลอง

### ข้อ 1: การวิเคราะห์ CoRE Link Format (RFC 6690)
นำผลการ Query `/.well-known/core` มาแสดงในรายงาน พร้อมอธิบายรูปแบบ **CoRE Link Format (RFC 6690)** ว่าแสดงข้อมูลทรัพยากรอย่างไร

- **ผลการ Query `/.well-known/core`:**
  ```text
  </sensor/pot>,</actuator/led>
  ```
- **คำอธิบายรูปแบบ:**
  เป็นรูปแบบข้อความขนาดกะทัดรัดที่ใช้เครื่องหมายวงเล็บแหลมครอบเส้นทางของแต่ละบริการ และคั่นระหว่างบริการด้วยเครื่องหมายจุลภาค ทำให้อุปกรณ์สามารถค้นหารายการคำสั่งทั้งหมดบนบอร์ดได้อัตโนมัติโดยไม่ต้องมีเอกสารกำกับ

---

### ข้อ 2: ความแตกต่างระหว่าง CON (Confirmable) และ NON (Non-confirmable)
อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง **CON (Confirmable)** และ **NON (Non-confirmable)** เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ

- **CON (Confirmable):** เป็นการส่งข้อมูลที่ต้องมีข้อความตอบรับกลับมา หากส่งไม่ผ่านจะมีการส่งซ้ำอัตโนมัติ
- **NON (Non-confirmable):** เป็นการส่งข้อมูลออกไปเพียงครั้งเดียวโดยไม่รอข้อความตอบรับและไม่มีการส่งซ้ำ
- **การเปรียบเทียบในสภาวะสัญญาณรบกวน:**
  เมื่อสัญญาณมีคลื่นรบกวน แบบแรกจะรับประกันว่าข้อมูลไปถึงปลายทางแน่นอนแต่อาจช้าลงเพราะต้องส่งซ้ำ ส่วนแบบหลังจะเร็วกว่าแต่ข้อมูลมีโอกาสสูญหายได้ง่าย

---

### ข้อ 3: ความเหมาะสมของ CoAP กับโปรโตคอลเครือข่าย Constrained IoT
ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?

- **เหตุผลทางวิศวกรรม:**
  เพราะทำงานบนโครงสร้างขนาดเล็กที่มีส่วนหัวข้อมูลเพียง 4 ไบต์ และส่งข้อมูลเป็นแบบไบนารีผ่านช่องทางที่ไม่ต้องเชื่อมต่อค้างไว้ จึงกินพลังงานน้อยมากและใช้แบนด์วิดท์ต่ำ เหมาะกับอุปกรณ์ขนาดเล็กที่ใช้แบตเตอรี่

---
### Logs
```text
I (27) boot: ESP-IDF v6.1 2nd stage bootloader
I (27) boot: compile time Oct 24 2026 19:48:43
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
I (79) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=1f314h (127764) map
I (132) esp_image: segment 1: paddr=0002f33c vaddr=3ffb0000 size=00cdch (  3292) load
I (133) esp_image: segment 2: paddr=00030020 vaddr=400d0020 size=b11bch (725436) map
I (394) esp_image: segment 3: paddr=000e11e4 vaddr=3ffb0cdc size=03bfch ( 15356) load
I (400) esp_image: segment 4: paddr=000e4de8 vaddr=40080000 size=154b0h ( 87216) load
I (436) esp_image: segment 5: paddr=000fa2a0 vaddr=50000000 size=00028h (    40) load
I (447) boot: Loaded app from partition at offset 0x10000
I (447) boot: Disabling RNG early entropy source...
I (458) cpu_start: Multicore app
I (466) cpu_start: GPIO 3 and 1 are used as console UART I/O pins
I (466) cpu_start: Pro cpu start user code
I (466) cpu_start: cpu freq: 160000000 Hz
I (468) app_init: Application information:
I (472) app_init: Project name:     Lab10-3_CoAP_Server
I (477) app_init: App version:      3f35f96
I (481) app_init: Compile time:     Oct 24 2026 19:48:04
I (486) app_init: ELF file SHA256:  812779d2f...
I (490) app_init: ESP-IDF:          v6.1
I (494) efuse_init: Min chip rev:     v0.0
I (498) efuse_init: Max chip rev:     v3.99
I (502) efuse_init: Chip rev:         v3.1
I (506) heap_init: Initializing. RAM available for dynamic allocation:
I (512) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (517) heap_init: At 3FFBC4B0 len 00023B50 (142 KiB): DRAM
I (522) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (528) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (533) heap_init: At 400954B0 len 0000AB50 (42 KiB): IRAM
I (539) spi_flash: detected chip: generic
I (542) spi_flash: flash io: dio
W (545) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (559) main_task: Started on CPU0
I (559) main_task: Calling app_main()
I (579) COAP_LAB: ADC Initialized on GPIO 34
I (599) wifi:wifi driver task: 3ffc4028, prio:23, stack:6656, core=0
I (609) wifi:wifi firmware version: e12a754
I (609) wifi:wifi certification version: v7.0
I (609) wifi:config NVS flash: enabled
I (609) wifi:config nano formatting: disabled
I (619) wifi:Init data frame dynamic rx buffer num: 32
I (619) wifi:Init static rx mgmt buffer num: 5
I (619) wifi:Init management short buffer num: 32
I (629) wifi:Init dynamic tx buffer num: 32
I (629) wifi:Init static rx buffer size: 1600
I (639) wifi:Init static rx buffer num: 10
I (639) wifi:Init dynamic rx buffer num: 32
I (649) wifi_init: rx ba win: 6
I (649) wifi_init: accept mbox: 6
I (649) wifi_init: tcpip mbox: 32
I (649) wifi_init: udp mbox: 6
I (659) wifi_init: tcp mbox: 6
I (659) wifi_init: tcp tx win: 5760
I (659) wifi_init: tcp rx win: 5760
I (659) wifi_init: tcp mss: 1440
I (669) wifi_init: WiFi IRAM OP enabled
I (669) wifi_init: WiFi RX IRAM OP enabled
I (699) phy_init: phy_version 4863,a3a4459,Oct 28 2025,14:30:06
I (799) phy_init: Saving new calibration data due to checksum failure or outdated calibration data, mode(0)
I (819) wifi:mode : sta (84:1f:e8:39:bd:64)
I (819) wifi:enable tsf
I (819) COAP_LAB: Connecting to AP: FBT4402_2.4...
I (1019) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1, snd_ch_cfg:0x0
I (1019) wifi:state: init -> auth (0xb0)
I (1699) wifi:state: auth -> assoc (0x0)
I (1709) wifi:state: assoc -> run (0x10)
I (1829) wifi:connected with FBT4402_2.4, aid = 2, channel 2, BW20, bssid = 34:4a:c3:26:26:d6
I (1829) wifi:security: WPA3-SAE HUNT_AND_PECK, phy: bgn, rssi: -39, cipher(pairwise:0x3, group:0x3), pmf:1
I (1849) wifi:pm start, type: 1

I (1849) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (1859) wifi:<ba-add>idx:0 (ifx:0, 34:4a:c3:26:26:d6), tid:5, ssn:3, winSize:64
I (1929) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (3349) esp_netif_handlers: sta ip: 192.168.1.8, mask: 255.255.255.0, gw: 192.168.1.1
I (3349) COAP_LAB: Connected! IP Address: 192.168.1.8
I (3349) COAP_LAB: CoAP Server listening on port 5683...
I (3349) COAP_LAB: Ready! Test CoAP with: python test_coap.py
I (3359) main_task: Returned from app_main()
I (53349) wifi:<ba-add>idx:1 (ifx:0, 34:4a:c3:26:26:d6), tid:0, ssn:2, winSize:64
I (53369) COAP_LAB: GET /sensor/pot -> 4095
I (53389) COAP_LAB: LED turned ON via CoAP PUT
I (56719) COAP_LAB: LED turned OFF via CoAP PUT
```