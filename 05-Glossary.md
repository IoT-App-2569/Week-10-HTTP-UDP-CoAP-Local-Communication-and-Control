# อภิธานศัพท์และคำย่อทางเทคนิค (Glossary of Technical Terms)
### ประจำสัปดาห์ที่ 10: Local Communication & Control (HTTP, UDP, CoAP)

---

| คำศัพท์ / คำย่อ | คำเต็ม (Full Term) | คำอธิบายความหมายเชิงลึก |
| :--- | :--- | :--- |
| **Local Control** | การควบคุมภายในเครือข่ายท้องถิ่น | สถาปัตยกรรมการสั่งงานอุปกรณ์ IoT โดยตรงผ่านเครือข่ายแลน (LAN) โดยไม่จำเป็นต้องส่งข้อมูลผ่านอินเทอร์เน็ตหรือคลาวด์ภายนอก ช่วยลดความหน่วงเวลาและเพิ่มความเสถียร |
| **mDNS** | Multicast Domain Name System (RFC 6762) | โปรโตคอลการแปลงชื่อโดเมน (เช่น `esp32.local`) เป็นหมายเลข IP ภายในเครือข่ายแลนโดยไม่ต้องมีเครื่องแม่ข่าย DNS ส่วนกลาง ทำงานผ่านพอร์ต UDP 5353 |
| **DNS-SD** | DNS-based Service Discovery (RFC 6763) | มาตรฐานการประกาศบริการ (Service Discovery) ร่วมกับ mDNS เพื่อให้อุปกรณ์อื่นสามารถค้นหาประเภทบริการ (เช่น `_http._tcp`, `_coap._udp`) และหมายเลขพอร์ตได้อัตโนมัติ |
| **LwIP** | Lightweight IP Stack | ซอฟต์แวร์สแตก TCP/IP แบบโอเพนซอร์สขนาดกะทัดรัดที่ออกแบบมาเฉพาะสำหรับระบบสมองกลฝังตัวและไมโครคอนโทรลเลอร์ ซึ่งเป็นแกนหลักเครือข่ายของ ESP-IDF |
| **BSD Socket API** | Berkeley Software Distribution Socket API | มาตรฐานการเขียนโปรแกรมอินเทอร์เฟซเพื่อติดต่อสื่อสารระดับเน็ตเวิร์ก (POSIX-compliant) เช่น ฟังก์ชัน `socket()`, `bind()`, `sendto()`, `recvfrom()` |
| **TCP** | Transmission Control Protocol (RFC 793) | โปรโตคอลระดับ Transport แบบ Connection-Oriented มีการสร้างการเชื่อมต่อ (3-Way Handshake) และรับประกันความถูกต้องของการส่งข้อมูลตามลำดับ |
| **UDP** | User Datagram Protocol (RFC 768) | โปรโตคอลระดับ Transport แบบ Connectionless ขนาด Header เล็กเพียง 8 ไบต์ ส่งข้อมูลแบบ Best-effort ไม่มีการรับประกันการส่งซ้ำ เหมาะสำหรับข้อมูล Real-time |
| **HTTP / REST** | HyperText Transfer Protocol / Representational State Transfer | โปรโตคอลระดับแอปพลิเคชันบน TCP นิยมใช้งานร่วมกับสถาปัตยกรรม RESTful API โดยใช้ HTTP Methods (GET, POST, PUT, DELETE) ในการจัดการทรัพยากร |
| **cJSON** | C JSON Library | ไลบรารีภาษา C น้ำหนักเบาใน ESP-IDF สำหรับการเข้ารหัส (Serialization) และถอดรหัส (Parsing) ข้อมูลในรูปแบบ JSON |
| **CoAP** | Constrained Application Protocol (RFC 7252) | โปรโตคอลระดับแอปพลิเคชันแบบ REST-like ที่ออกแบบมาเพื่ออุปกรณ์ที่มีข้อจำกัดด้านหน่วยความจำและพลังงาน ทำงานบน UDP ด้วยขนาด Fixed Header เพียง 4 ไบต์ |
| **CON** | Confirmable Message | รูปแบบแพ็กเก็ตของ CoAP ที่ต้องการการตอบรับ (ACK) ยืนยันว่าผู้รับได้รับข้อมูล หากหมดเวลาจะส่งซ้ำโดยอัตโนมัติ |
| **NON** | Non-confirmable Message | รูปแบบแพ็กเก็ตของ CoAP ที่ส่งข้อมูลแบบไม่ต้องการการตอบรับ (Fire-and-forget) คล้ายคลึงกับ UDP ธรรมดา |
| **CoAP Observe** | Observing Resources in CoAP (RFC 7641) | กลไกส่วนขยายของ CoAP ที่เปิดให้ Client สมัครรับข้อมูลการเปลี่ยนแปลงของ Resource จาก Server แบบอัตโนมัติ (คล้าย Publish/Subscribe) โดยไม่ต้องทำ Polling ซ้ำๆ |
| **CoRE Link Format** | Constrained RESTful Environments Link Format (RFC 6690) | มาตรฐานรูปแบบข้อความที่ CoAP Server ใช้ส่งรายชื่อทรัพยากร (Resource Directory) กลับมาเมื่อถูกเรียกผ่าน URI `/.well-known/core` |
| **DTLS** | Datagram Transport Layer Security (RFC 6347) | โปรโตคอลความปลอดภัยสำหรับการเข้ารหัสข้อมูลและการตรวจสอบสิทธิ์ ทำงานครอบบน UDP เพื่อให้ความปลอดภัยระดับเดียวกับ TLS บน TCP |
| **RTT** | Round-Trip Time | เวลาที่แพ็กเก็ตข้อมูลใช้เดินทางจากผู้ส่งไปยังผู้รับ บวกกับเวลาที่แพ็กเก็ตตอบรับ (Response/ACK) เดินทางกลับมาถึงผู้ส่ง |
| **Packet Overhead** | ภาระส่วนหัวของแพ็กเก็ต | สัดส่วนของไบต์ข้อมูล Header (เช่น IP + UDP/TCP + Application Header) เทียบกับขนาดข้อมูลจริง (Payload) |
| **Wireshark** | โปรแกรมวิเคราะห์แพ็กเก็ตเครือข่าย | เครื่องมือ Open-Source สำหรับตรวจจับ (Packet Sniffing) และวิเคราะห์โครงสร้างข้อมูลของโปรโตคอลเครือข่ายทุกระดับชั้น |
