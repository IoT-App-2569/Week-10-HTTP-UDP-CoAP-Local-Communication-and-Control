# บทเรียนที่ 4: โปรโตคอล CoAP สำหรับระบบสมองกลฝังตัวทรัพยากรจำกัด (CoAP for Constrained IoT)

## 1. จุดกำเนิดและแนวคิดของ CoAP (Motivation for CoAP)
เมื่อพิจารณาข้อดีและข้อเสียระหว่าง **HTTP** และ **UDP** ในสองบทเรียนก่อนหน้า:
* **HTTP**: มีโมเดลการออกแบบที่ดีเยี่ยม (REST, URIs, Methods: GET/POST/PUT/DELETE) แต่น้ำหนักมากเกินไป (Heavyweight) ทั้งขนาด Header ระดับหลายร้อยไบต์ และการใช้ TCP
* **UDP**: เบาและเร็วมาก (Header เพียง 8 ไบต์) แต่ไม่มีโครงสร้างเชิงแอปพลิเคชัน (No REST/No URI) และไม่มีระบบยืนยันความถูกต้องของข้อมูลในตัว

คณะทำงาน **IETF (Internet Engineering Task Force)** จึงได้พัฒนามาตรฐาน **CoAP (Constrained Application Protocol - RFC 7252)** ขึ้นมา เพื่อนำเอา **"ความง่ายและโครงสร้างของ HTTP REST มารวมกับความเบาและความเร็วของ UDP"**

<p align="center">
<!-- [รูปภาพ: การเปรียบเทียบ Protocol Stack ระหว่าง HTTP/TCP และ CoAP/UDP] -->
<!-- <img src="Images/http_vs_coap_stack.svg" width="600"> -->
</p>

---

## 2. โครงสร้างแพ็กเก็ต CoAP (CoAP Message Format)
หัวใจสำคัญที่ทำให้ CoAP ประหยัดแบนด์วิดท์อย่างยิ่งยวด คือ **Fixed Header ขนาดเพียง 4 ไบต์** เท่านั้น:

```
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

### คำอธิบายฟิลด์ใน Header:
1. **Ver (Version - 2 bits)**: ค่าเวอร์ชันของโปรโตคอล (ปัจจุบันกำหนดเป็น `1`)
2. **T (Type - 2 bits)**: ระบุรูปแบบความน่าเชื่อถือของแพ็กเก็ต:
   * `0`: **CON (Confirmable)** - ต้องการการตอบรับ (ACK) ยืนยันว่าถึงปลายทาง
   * `1`: **NON (Non-confirmable)** - ส่งแบบไม่รอการตอบรับ (คล้าย UDP ธรรมดา)
   * `2`: **ACK (Acknowledgement)** - แพ็กเก็ตตอบรับการได้รับข้อความ CON
   * `3`: **RST (Reset)** - ปฏิเสธข้อความหรือไม่สามารถประมวลผลได้
3. **TKL (Token Length - 4 bits)**: ความยาวของ Token (0-8 ไบต์) สำหรับจับคู่ Request กับ Response
4. **Code (8 bits)**: แบ่งเป็น Class (3 bits) และ Detail (5 bits) เช่น:
   * `0.01` = **GET**
   * `0.02` = **POST**
   * `0.03` = **PUT**
   * `0.04` = **DELETE**
   * `2.05` = **Content** (เทียบเท่า HTTP 200 OK)
   * `4.04` = **Not Found** (เทียบเท่า HTTP 404)
5. **Message ID (16 bits)**: หมายเลขกำกับแพ็กเก็ต ใช้ตรวจจับแพ็กเก็ตที่ส่งซ้ำ (Duplicate Detection) และจับคู่กับ ACK
6. **Payload Marker (0xFF)**: ไบต์คั่นที่มีค่า `1111 1111` เพื่อบอกจุดเริ่มต้นของเนื้อหา Payload

---

## 3. กลไกสำคัญของ CoAP (Key Features)

### 3.1 การรับประกันการส่งข้อมูลบน UDP (CON vs NON)
* เมื่อส่งแพ็กเก็ตแบบ **CON**, ผู้รับจะต้องส่ง **ACK** ที่มี Message ID เดียวกันกลับมา หากผู้ส่งไม่ได้รับ ACK ภายในเวลาที่กำหนด (Timeout) จะทำการส่งซ้ำแบบ Exponential Backoff อัตโนมัติ
* เมื่อส่งแพ็กเก็ตแบบ **NON**, ผู้ส่งจะยิงข้อมูลออกไปครั้งเดียว ไม่มีการส่งซ้ำ เหมาะสำหรับข้อมูลเซนเซอร์ที่มีการส่งอัปเดตต่อเนื่อง

<p align="center">
<!-- [รูปภาพ: แผนภาพลำดับการทำงานของ CON/ACK และ NON Messages] -->
<!-- <img src="Images/coap_con_non_sequence.svg" width="600"> -->
</p>

### 3.2 กลไกการติดตามสถานะ CoAP Observe (RFC 7641)
ใน HTTP หาก Client ต้องการอัปเดตสถานะเซนเซอร์จะต้องส่ง HTTP GET ซ้ำๆ (Polling) ซึ่งเปลืองพลังงานและเครือข่ายอย่างมาก

ใน CoAP มีส่วนขยายชื่อ **Observe**:
1. Client ส่งคำสั่ง `GET /sensor/pot` พร้อมตั้งค่า Option `Observe = 0` (Registration)
2. CoAP Server (ESP32) จะจดจำ Client ไว้ในตาราง Observer
3. เมื่อใดก็ตามที่ค่าเซนเซอร์เปลี่ยนแปลง ESP32 จะส่ง Notification แพ็กเก็ตไปหา Client ทันทีโดย Client ไม่ต้องถามซ้ำ (ทำงานเสมือน Publish/Subscribe บน UDP โดยไม่ต้องพึ่งพา MQTT Broker!)

### 3.3 การค้นหา Resource ในตัวอุปกรณ์ (Resource Discovery)
CoAP มีมาตรฐาน **CoRE Link Format (RFC 6690)** ในตัว โดย Client สามารถส่งคำสั่ง:
```bash
GET /.well-known/core
```
ESP32 จะตอบกลับรายชื่อ Endpoints ทั้งหมดที่มีบนอุปกรณ์ เช่น:
```
</sensors/pot>;title="Analog Sensor";rt="Sensor",</actuators/led>;title="LED Light";rt="Actuator"
```

---

## 4. ตารางเปรียบเทียบเชิงลึก: HTTP vs UDP vs CoAP

| มิติการเปรียบเทียบ | HTTP / REST | UDP Raw Socket | CoAP (RFC 7252) |
| :--- | :--- | :--- | :--- |
| **ชั้น Transport** | TCP | UDP | UDP |
| **พอร์ตมาตรฐาน** | 80 (HTTP), 443 (HTTPS) | กำหนดเอง (เช่น 3333) | 5683 (CoAP), 5684 (CoAPS) |
| **ขนาด Header ต่ำสุด** | ~100-500 ไบต์ (ASCII) | 8 ไบต์ (ไบนารี) | **4 ไบต์** (ไบนารี) |
| **สถาปัตยกรรม** | Client / Server | Peer-to-Peer / Datagram | Client / Server (RESTful) |
| **REST Methods** | GET, POST, PUT, DELETE | ไม่มี | **GET, POST, PUT, DELETE** |
| **การส่งข้อมูลต่อเนื่อง** | Long-polling / WebSocket | สตรีมมิ่งต่อเนื่อง | **Observe (Pub/Sub)** |
| **การใช้พลังงาน (Power)** | สูง | ต่ำมาก | **ต่ำมาก** |
| **การใช้งานหลัก** | Web Browser, Dashboard | ส่งภาพ/เสียง, ค้นหาอุปกรณ์ | **เซนเซอร์ IoT, แบตเตอรี่** |

---

## 5. สรุปท้ายบทเรียน (Chapter Summary)
* CoAP ได้รับการขนานนามว่าเป็น **"HTTP สำหรับอุปกรณ์ขนาดเล็ก"**
* ให้โครงสร้าง RESTful API และความน่าเชื่อถือเทียบเท่า HTTP แต่ใช้ทรัพยากร พลังงาน และแบนด์วิดท์เทียบเท่ากับ UDP
* ในใบงานที่ 10.3 และ 10.4 เราจะได้ลงมือทดสอบ CoAP Server บน ESP32 พร้อมทั้งใช้โปรแกรม **Wireshark** ดักจับแพ็กเก็ตเพื่อพิสูจน์ขนาด 4 ไบต์ของ CoAP Header ด้วยตนเอง
