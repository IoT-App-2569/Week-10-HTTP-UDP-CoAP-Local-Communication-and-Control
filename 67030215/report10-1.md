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
