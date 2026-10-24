### กิจกรรมที่ 10-1.5 การทดสอบและตรวจพิสูจน์ (Verification & Forensics)

#### 1. ตรวจสอบ mDNS ด้วยคำสั่ง ping
เปิด Terminal บนเครื่องคอมพิวเตอร์ที่อยู่ในวง Wi-Fi เดียวกัน
```powershell
ping esp32-node.local
```
<img width="602" height="287" alt="image" src="https://github.com/user-attachments/assets/ee6945a0-3e89-4b11-b5d9-e7a274155f9b" />

#### 2. ทดสอบอ่านค่าเซนเซอร์ผ่าน cURL
เปิด PowerShell แล้วรันคำสั่ง (แนะนำให้พิมพ์ `curl.exe` เพื่อเรียกใช้โปรแกรม cURL แท้ของระบบแทน PowerShell Alias)
```powershell
curl.exe -X GET http://esp32-node.local/api/status
```
ผลลัพธ์ที่ได้จะเป็น JSON Payload เช่น
```json
{"pot_raw":15,"free_heap":213848,"led":true}
```
<img width="853" height="73" alt="image" src="https://github.com/user-attachments/assets/58eebbb8-b0b3-475b-b30d-2fb63ed7be6e" />

#### 3. ทดสอบสั่งเปิด-ปิด LED ผ่าน cURL หรือ PowerShell

**วิธีที่ 1 ใช้ `curl.exe` (ครอบสตริง JSON ด้วย Single Quote เพื่อป้องกัน PowerShell ตัดเครื่องหมายคำพูด)**
```powershell
# สั่งเปิดไฟ LED (GPIO 2)
curl.exe -s -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d '{\"state\": true}'

# สั่งปิดไฟ LED (GPIO 2)
curl.exe -s -X POST http://esp32-node.local/api/led -H "Content-Type: application/json" -d '{\"state\": false}'
```
การเปิดและปิดไฟ จะได้ผลลัพธ์เป็น `{"result":"success"}` ทั้งคู่

<img width="1032" height="130" alt="image" src="https://github.com/user-attachments/assets/77374768-7a5c-4801-9c70-90a301f7a5a4" />

## 4. บันทึกผลการทดลองและคำถามท้ายการทดลอง
1. นำผลการรัน `curl -i` (โหมด verbose เพื่อดู HTTP Response Header) มาแปะในรายงาน พร้อมวิเคราะห์ว่า HTTP Header มีขนาดกี่ไบต์ และข้อมูล JSON มีขนาดกี่ไบต์

<img width="486" height="140" alt="Screenshot 2026-10-22 213031" src="https://github.com/user-attachments/assets/9042b138-6ae2-4a48-bc1c-2e1451d6c011" />

```
วิเคราะห์
- ข้อมูลรวมในชั้น HTTP = 71 + 47 = 118 ไบต์
  สัดส่วนข้อมูลจริง = 47 / 118 x 100 = 39.8 %
  แปลว่าข้อมูลที่ส่งเกินครึ่งหนึ่ง (60.2 %) เป็นส่วนหัว ไม่ใช่ข้อมูลที่ต้องการ
- ถ้ารวม TCP Header (20 ไบต์) และ IP Header (20 ไบต์) จะเป็น 158 ไบต์
  สัดส่วนข้อมูลจริงลดลงเหลือ 47 / 158 x 100 = 29.7 %
- header นี้ถือว่าเล็กแล้ว เพราะ esp_http_server ส่งเฉพาะ header ที่จำเป็น
  ถ้าเป็นเว็บเซิร์ฟเวอร์ทั่วไปจะมี Date, Server, Connection ฯลฯ ทำให้ใหญ่ขึ้นเป็นหลายร้อยไบต์
- ยังไม่รวมฝั่ง Request ที่ curl ส่งไป (เช่น Host, User-Agent, Accept)
  และแพ็กเก็ต TCP Handshake ซึ่งเพิ่มภาระอีกหลายแพ็กเก็ต
```
1. หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?
```
ผลกระทบ คอมพิวเตอร์เครื่องนั้นจะแปลงชื่อโฮสต์ (เช่น esp32.local) เป็น IP ไม่ได้ ทำให้เปิดหน้าเว็บ ping หรือค้นหาอุปกรณ์ด้วยชื่อไม่เจอ ทั้งที่ ESP32 ทำงานปกติและมี IP อยู่แล้ว
วิธีแก้ ใช้ IP Address เข้าถึงอุปกรณ์โดยตรง (ดูได้จาก Serial Log หรือหน้า DHCP ของ Router) และตั้ง DHCP Reservation หรือ Static IP ให้ IP ไม่เปลี่ยน
หรือติดตั้งตัวรองรับ mDNS เช่น Bonjour บน Windows และ Avahi บน Linux แล้วเปิดพอร์ต UDP 5353 ใน Firewall ให้ใช้งานได้ หรือเพิ่มชื่อกับ IP ลงในไฟล์ hosts ของเครื่องนั้นแทน
```

2. เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?
```
cJSON สร้างออบเจกต์ JSON และสตริงที่แปลงออกมาบน Heap ทั้งหมด ไม่ได้คืนให้อัตโนมัติ
cJSON_Delete(root) คืนหน่วยความจำของโครงสร้าง JSON ทั้งต้นไม้ (root และโหนดลูกทุกตัว)
cJSON_free(resp) คืนหน่วยความจำของสตริงผลลัพธ์ที่ได้จาก cJSON_Print ซึ่งเป็นอีกก้อนหนึ่ง cJSON_Delete ไม่ได้ล้างให้
ถ้าไม่เรียก ทุกครั้งที่มี Request เข้ามา Heap จะหายไปเรื่อยๆ (Memory Leak) ESP32 มี RAM น้อย สุดท้ายจะจองหน่วยความจำไม่ได้ ระบบค้างหรือรีบูตเอง
```
