# ใบงานการทดลองที่ 10.1
## 4. บันทึกผลการทดลองและคำถามท้ายการทดลอง
1. นำผลการรัน `curl -i` (โหมด verbose เพื่อดู HTTP Response Header) มาแปะในรายงาน พร้อมวิเคราะห์ว่า HTTP Header มีขนาดกี่ไบต์ และข้อมูล JSON มีขนาดกี่ไบต์

```powershell
curl -i -X GET http://esp32-node.local/api/status
```
ผลลัพธ์ 
```
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 44
 
{"pot_raw":15,"free_heap":213848,"led":false}
```
**วิเคราะห์**
- Header ยาวกว่าข้อมูลจริงประมาณ 1.6 เท่า ข้อมูลที่ใช้งานได้จึงเป็นเพียง 44/115 ≈ 38% ของข้อความ HTTP ทั้งหมด ซึ่งยังไม่รวม TCP/IP header
- `esp_http_server` ใส่ Header เฉพาะที่จำเป็น ไม่มี `Date` หรือ `Server` จึงประหยัดกว่าเว็บเซิร์ฟเวอร์ทั่วไป
- ขนาด JSON เปลี่ยนไปตามจำนวนหลักของค่าที่อ่านได้ เช่น ถ้า pot_raw = 4095 Body จะยาว 46 ไบต์
- ถ้าต้องส่งข้อมูลถี่ ๆ ควรพิจารณาโปรโตคอลที่มี overhead ต่ำกว่า เช่น WebSocket, MQTT หรือ CoAP

2. หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?
```
**ผลกระทบ**
- เครื่องนั้นแปลงชื่อ `esp32-node.local` เป็น IP ไม่ได้ ทั้ง ping, curl และเบราว์เซอร์จะแจ้งว่า `could not find host`
- ค้นหาบริการ `_http._tcp` ผ่าน DNS-SD ไม่เจอ เพราะ mDNS ทำงานบน multicast `224.0.0.251:5353`
- ตัว ESP32 และ HTTP Server ยังทำงานปกติ ส่วนที่ใช้ไม่ได้มีแค่การเรียกด้วยชื่อ ถ้ารู้ IP ก็ยังเข้าถึงได้
**วิธีแก้ไข**
1. เข้าถึงด้วย IP โดยตรง เช่น `http://192.168.1.181/api/status` โดยดู IP ได้จาก Serial Monitor
2. ตั้ง DHCP Reservation บน Router หรือกำหนด Static IP ให้ ESP32 เพื่อให้ IP ไม่เปลี่ยน
3. ตั้งค่า Windows ให้เป็น Private Network หรือเปิด Firewall ให้รับ UDP 5353 และติดตั้ง Bonjour หรือ Avahi ในเครื่องที่ยังไม่รองรับ mDNS
4. เพิ่มชื่อลงในไฟล์ `hosts` หรือใน DNS ของ Router เช่น `192.168.1.181 esp32-node.local`
5. ปิด AP Isolation บน Router และถ้าอุปกรณ์อยู่คนละ VLAN ให้ใช้ mDNS Reflector
```
3. เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?
```
- `cJSON_CreateObject()`, `cJSON_Add…()` และ `cJSON_Parse()` จองหน่วยความจำให้ทุก node บน heap ด้วย `malloc`
หน่วยความจำส่วนนี้จะไม่ถูกคืนเองเมื่อฟังก์ชันทำงานจบ
  - `cJSON_Delete(root)` จะคืนหน่วยความจำของ root และ node ลูกทั้งหมดแบบ recursive
- `cJSON_PrintUnformatted()` จองบัฟเฟอร์สตริงขึ้นมาใหม่อีกก้อน แยกจากต้นไม้ JSON จึงต้องคืนแยกด้วย `cJSON_free(resp)`
  - ควรใช้ `cJSON_free()` แทน `free()` เพราะ cJSON อาจถูกตั้งให้ใช้ allocator ของตัวเองผ่าน `cJSON_InitHooks()`
- handler ถูกเรียกทุกครั้งที่มี request และ ESP32 ไม่มี Garbage Collector ถ้าไม่คืนหน่วยความจำจะเกิด **Memory Leak** สะสมทุกครั้งที่เรียก
  - heap ราว 200 KB จะค่อย ๆ หมด จนระบบ Wi-Fi/HTTP ทำงานผิดพลาด แล้วบอร์ด crash หรือรีบูต
  - อาการนี้สังเกตได้จากค่า `free_heap` ใน `/api/status` ที่ลดลงเรื่อย ๆ
- ต้องคืนหน่วยความจำในทุกเส้นทางของโค้ด รวมถึงกรณีที่เกิด error และต้องคืนหลังส่งข้อมูลเสร็จแล้วเท่านั้น ห้ามใช้ `resp` อีกหลังจาก free ไปแล้ว
```
