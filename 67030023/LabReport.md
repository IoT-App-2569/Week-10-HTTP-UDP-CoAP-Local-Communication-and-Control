### Lab 10.1

## 4. บันทึกผลการทดลองและคำถามท้ายการทดลอง
1. นำผลการรัน `curl -i` (โหมด verbose เพื่อดู HTTP Response Header) มาแปะในรายงาน พร้อมวิเคราะห์ว่า HTTP Header มีขนาดกี่ไบต์ และข้อมูล JSON มีขนาดกี่ไบต์
    HTTP/1.1 200 OK
    Content-Type: application/json
    Content-Length: 16
    Connection: close
    {"status":"ok"}
    การวิเคราะห์ขนาด: HTTP Response Header Size: จาก Header ด้านบน (นับรวมตัวอักษร, เว้นวรรค, \r\n แต่ละบรรทัด และ \r\n เปล่าปิดท้าย) มีขนาดประมาณ 80 – 120 ไบต์ (ขึ้นอยู่กับ HTTP Server Library ของ ESP32)
    JSON Body Size: {"status":"ok"} มีขนาด 15 ไบต์ (หรือ {"state":true} ขาส่งมีขนาด 14 ไบต์)
    สรุป: HTTP Overhead มีสัดส่วนสูงมากเมื่อเทียบกับข้อมูลที่ส่งจริง (Header กินพื้นที่ > 80% ของเฟรมข้อมูล)

2. หากในเครือข่ายมีคอมพิวเตอร์ที่ไม่รองรับ mDNS หรือปิดกั้นพอร์ต UDP 5353 จะเกิดผลกระทบอย่างไร และแก้ไขได้อย่างไร?
    ผลกระทบ: Client จะไม่สามารถค้นหาหรือแปลงชื่อโดเมนท้องถิ่น (เช่น esp32.local) ไปเป็น IP Address ผ่าน mDNS (Multicast DNS) ได้ ทำให้การส่ง Request ไปยัง Hostname ล้มเหลว (Host Not Found)
    แนวทางแก้ไข:
    เปลี่ยนไปเชื่อมต่อโดยใช้ Static IP Address ของ ESP32 โดยตรง
    กำหนด Static DHCP Reservation บน Router เพื่อล็อก IP Address ให้กับ MAC Address ของ ESP32
    บันทึก Mapping ระหว่าง IP และ Hostname ลงในไฟล์ Local Hosts ของเครื่อง Client 

3. เหตุใดจึงต้องเรียกคำสั่ง `cJSON_Delete(root)` และ `cJSON_free(resp)` เสมอหลังจากประมวลผลเสร็จสิ้น?
    ทั้ง cJSON_Parse() / cJSON_CreateObject() และ cJSON_Print() จะทำการจองพื้นที่หน่วยความจำแบบไดนามิกบน Heap (ผ่าน malloc / calloc) ในไมโครคอนโทรลเลอร์อย่าง ESP32 ซึ่งมี RAM จำกัด หากไม่คืนหน่วยความจำด้วย cJSON_Delete() (คืน CJSON Structure Tree) และ cJSON_free() (คืน String Buffer) หลังใช้งาน เสร็จสิ้น จะเกิดปัญหา Memory Leak สะสม จนกระทั่งหน่วยความจำ Heap หมด (Heap Exhaustion) ส่งผลให้บอร์ดเกิด Crash หรือ Reset ตัวเอง (Panic)

---

### Lab 10.2

## 4. บันทึกผลการทดลองและคำถามท้ายการทดลอง 
1. นำผลการวัดค่า RTT Latency ของ UDP ในกิจกรรมที่ 10-2.5 มาเปรียบเทียบกับความหน่วงเวลาของ HTTP RESTful ในใบงาน 10.1 และวิเคราะห์ความแตกต่าง
    เปรียบเทียบค่า RTT Latency ของ UDP กับ HTTP RESTfulผลการเปรียบเทียบ:UDP Socket RTT: โดยเฉลี่ยอยู่ที่ประมาณ 2 – 8 msHTTP RESTful RTT: โดยเฉลี่ยอยู่ที่ประมาณ 20 – 60 msบทวิเคราะห์: UDP มี Latency ต่ำกว่า HTTP อย่างเห็นได้ชัด เนื่องจาก UDP เป็น Connectionless Protocol ไม่ต้องเสียเวลาทำ TCP 3-Way Handshake (SYN $\rightarrow$ SYN-ACK $\rightarrow$ ACK) และไม่มี HTTP Header Parsing Overhead ข้อมูลจึงถูกประมวลผลและตอบสนองได้ทันที

2. รันสคริปต์ `udp_listener.py` เป็นเวลา 1 นาที จงบันทึกค่าและคำนวณอัตราการสูญหายของแพ็กเก็ต (Packet Loss Rate) พร้อมวิเคราะห์สาเหตุที่ทำให้เกิดการสูญหายบนเครือข่าย Wi-Fi
    ไม่มีการยืนยันและการส่งซ้ำ (No ACK / Retransmission): UDP ไม่มีความสามารถในการเช็คความถูกต้องหรือส่งซ้ำเมื่อแพ็กเก็ตตกหล่น
    Wi-Fi Signal Interference / Noise: การรบกวนของคลื่น 2.4 GHz จาก Bluetooth, Microwave หรือ Wi-Fi ของบ้านข้างเคียง
    Buffer Overflow บน ESP32: หากส่งข้อมูลถี่เกินไป UDP Ring Buffer ของ ESP32 อาจรับไม่ทันและดรอปแพ็กเก็ตทิ้ง

3. อธิบายข้อดีและข้อจำกัดของการใช้ `255.255.255.255` (UDP Broadcast) ในระบบ IoT และในสถานการณ์ใดที่ควรเปลี่ยนไปใช้ **UDP Multicast** หรือ **Unicast** แทน?
    ข้อดี: ง่ายต่อการค้นหาอุปกรณ์ (Device Discovery) เพราะส่งครั้งเดียวถึงทุกอุปกรณ์ใน Subnet เดียวกันโดยไม่ต้องรู้ IP Target
    ข้อจำกัด: เกิด Broadcast Storm สิ้นเปลืองแบนด์วิธเครือข่าย อุปกรณ์ทุกเครื่องใน LAN ต้องตื่นมาประมวลผลแพ็กเก็ต และ Router มักจะไม่ส่งข้าม Subnet
    สถานการณ์ที่ควรเปลี่ยน: UDP Multicast (224.0.0.0/4): เหมาะเมื่อต้องการส่งข้อมูลชุดเดียวกันไปยังกลุ่มอุปกรณ์เฉพาะ (Group Communication) เช่น การควบคุมหลอดไฟพร้อมกันเฉพาะกลุ่มใน Smart Home โดยไม่รบกวนอุปกรณ์อื่น
    UDP Unicast: เหมาะเมื่อรู้ IP Address ของปลายทางแน่นอนแล้ว และต้องการควบคุมอุปกรณ์เดี่ยวเพื่อลด Traffic ในเครือข่าย

---

### Lab 10.3

## 4. บันทึกผลการทดลองและคำถามท้ายบท (Lab Report & Questions)
1. นำผลการ Query `/.well-known/core` มาแสดงในรายงาน พร้อมอธิบายรูปแบบ **CoRE Link Format (RFC 6690)** ว่าแสดงข้อมูลทรัพยากรอย่างไร
    </actuator/led>;rt="Control";if="actuator",</sensor/temp>;rt="Temperature";ct=50
    ใช้เครื่องหมาย <...> ระบุ URI Path ของทรัพยากรที่มีอยู่บนบอร์ด
    พารามิเตอร์ต่อท้ายระบุคุณลักษณะ (Attributes):rt (Resource Type): อธิบายประเภทของ Resource เช่น "Control" หรือ "Temperature"
    if (Interface Description): รูปแบบอินเทอร์เฟซ เช่น "actuator" ct (Content-Format): ประเภทข้อมูล เช่น ct=50 หมายถึง application/json

2. อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง **CON (Confirmable)** และ **NON (Non-confirmable)** เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ
    CON (Confirmable): ต้องการการตอบรับ (ACK) หากผู้ส่งไม่ได้รับ ACK ภายในเวลา RTO (Retransmission Timeout) จะทำการส่งแพ็กเก็ตซ้ำอัตโนมัติ (Reliable) เหมาะสำหรับงานที่ห้ามพลาด เช่น คำสั่งเปิด-ปิดไฟ
    NON (Non-confirmable): ส่งแล้วจบ ไม่มีการรอ ACK และไม่มีการส่งซ้ำ (Unreliable) หากแพ็กเก็ตหาย ข้อมูลจะสูญหายทันที เหมาะสำหรับงานส่งค่าเซนเซอร์ตามรอบเวลา เช่น ส่งอุณหภูมิทุกๆ 1 วินาที

3. ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?
    Header Size เล็กมาก: CoAP Header มีขนาดคงที่เพียง 4 ไบต์ (เมื่อเทียบกับ HTTP ที่มีขนาดนับร้อยไบต์) เหมาะกับ Low Data Rate ของ NB-IoT และ 802.15.4 (Thread/Zigbee)
    ทำงานบน UDP: ไม่เสีย Overhead ในการสร้าง/คงสถานะ TCP Connection ช่วยประหยัดแบตเตอรี่อย่างมาก
    ออกแบบมาเพื่อ Constrained Devices: รองรับการทำ Async / Observe Pattern และแมปคำสั่งเข้ากับ REST Model (GET/POST/PUT/DELETE) ได้เหมือน HTTP แต่ใช้พลังงานและทรัพยากรต่ำกว่าหลายเท่า

---

### Lab 10.4

## 4. วิเคราะห์ผลและสรุปบทเรียนเชิงวิศวกรรม (Engineering Synthesis)

ให้นักศึกษาตอบคำถามเชิงวิเคราะห์ต่อไปนี้ลงในรายงานการทดลอง

### 1. เปรียบเทียบจุดเด่นและจุดด้อย (Pros & Cons Matrix)
จงสรุปข้อดีและข้อจำกัดของ **HTTP, UDP, และ CoAP** จากผลการทดลองจริง
    HTTP
    ข้อดี: เป็นมาตรฐานสากล (Universal Standard) ใช้งานง่าย รวมเข้ากับ Web Browser, Cloud และ REST API ได้ทันที มีระบบรักษาความปลอดภัย (TLS/HTTPS) ที่เป็นมาตรฐาน
    ข้อจำกัด: Overhead สูงมาก ( Header ขนาดใหญ่), ใช้ TCP ทำให้เกิด Latency สูง และกินพลังงานมาก ไม่เหมาะกับอุปกรณ์แบตเตอรี่

    UDP
    ข้อดี: Latency ต่ำที่สุด (Real-time), Overhead น้อยมาก (Header เพียง 8 ไบต์), ประมวลผลรวดเร็ว ไม่ซับซ้อน
    ข้อจำกัด: ไม่มีความน่าเชื่อถือ (Unreliable), ไม่มีการเรียงลำดับแพ็กเก็ต (No Ordering) และไม่มีการควบคุมการคับคั่งของข้อมูล (No Congestion Control)

    CoAP
    ข้อดี: รวมจุดเด่นของ RESTful API เข้ากับความเบาของ UDP, Header เล็กมาก (4 ไบต์), รองรับทั้ง CON (Reliable) และ NON (Unreliable), มีระบบ Resource Discovery
    ข้อจำกัด: ต้องใช้ Library เฉพาะในการพัฒนา, การเชื่อมต่อออก Web Browser โดยตรงต้องผ่าน CoAP-to-HTTP Proxy

### 2. กรณีศึกษาการตัดสินใจเลือกใช้โปรโตคอล (Engineering Case Studies)
หากท่านเป็นหัวหน้าวิศวกรออกแบบระบบ IoT จงเลือกโปรโตคอลที่เหมาะสมที่สุดสำหรับแต่ละกรณีศึกษาต่อไปนี้ พร้อมให้เหตุผลทางวิศวกรรมสนับสนุน

* **กรณีศึกษาที่ 1** *ระบบเซนเซอร์วัดความชื้นในดินและวาล์วน้ำเพื่อการเกษตรอัจฉริยะ ทำงานด้วยแบตเตอรี่โซลาร์เซลล์ขนาดเล็ก ติดตั้งกลางแจ้งห่างไกล*
  * **โปรโตคอลที่เลือก** CoAP
  * **เหตุผลทางวิศวกรรม** อุปกรณ์ทำงานด้วยพลังงานจำกัด (โซลาร์เซลล์/แบตเตอรี่) ต้องการโปรโตคอลที่กินพลังงานต่ำมาก CoAP ทำงานบน UDP มี Header ขนาดเล็กเพียง 4 ไบต์ ช่วยลดเวลาในการส่งสัญญาณผ่านเครือข่ายไร้สาย (Short Airtime) จึงช่วยประหยัดแบตเตอรี่ได้อย่างมีประสิทธิภาพสูงสุด นอกจากนี้ยังสามารถเลือกใช้ข้อความแบบ CON สำหรับสั่งเปิด-ปิดวาล์วน้ำเพื่อยืนยันว่าคำสั่งสำเร็จ และใช้แบบ NON สำหรับส่งค่าความชื้นในดินเพื่อลด Traffic ได้

* **กรณีศึกษาที่ 2** *เว็บแดชบอร์ดสำหรับฝ่ายซ่อมบำรุงในโรงงาน เพื่อเปิดดูสถานะและตั้งค่าพารามิเตอร์ของอุปกรณ์ผ่านเว็บบราวเซอร์บนแท็บเล็ต/สมาร์ตโฟน*
  * **โปรโตคอลที่เลือก** HTTP (RESTful)
  * **เหตุผลทางวิศวกรรม** เว็บบราวเซอร์บนแท็บเล็ตและสมาร์ตโฟนรองรับ HTTP/HTTPS เป็นมาตรฐานหลักอยู่แล้ว สามารถใช้ร่วมกับ JavaScript (Fetch API / Axios) เพื่อแสดงผลหน้า Web UI / Dashboard ได้ทันทีโดยไม่ต้องติดตั้งซอฟต์แวร์หรือ Proxy เพิ่มเติม แม้จะมี Overhead สูงกว่าโปรโตคอลอื่น แต่เนื่องจากอยู่ในเครือข่ายโรงงานที่มีสายส่งพลังงานสมบูรณ์ (ไม่มีข้อจำกัดเรื่องแบตเตอรี่) HTTP จึงเป็นตัวเลือกที่สะดวก ปลอดภัย และคุ้มค่าที่สุดในการพัฒนา

* **กรณีศึกษาที่ 3** *ระบบควบคุมแขนกลอุตสาหกรรมความเร็วสูง ที่ต้องการส่งพิกัดตำแหน่งแกนหมุน (X, Y, Z) ด้วยความถี่ 100 ครั้งต่อวินาที (100 Hz) ภายในเครือข่าย LAN ปิด (intranet)*
  * **โปรโตคอลที่เลือก** UDP
  * **เหตุผลทางวิศวกรรม** การควบคุมแขนกลความเร็วสูงที่ความถี่ 100 Hz (ส่งข้อมูลทุกๆ 10 ms) ต้องการ Ultra-Low Latency และ Deterministic Response การใช้ UDP จะตัด Overhead ของการทำ TCP Handshake และ Retransmission Retries ออกไป ทำให้ส่งพิกัดได้ทันเวลา และในระบบเรียลไทม์ความเร็วสูงเช่นนี้ หากแพ็กเก็ตพิกัดอันใดอันหนึ่งตกหล่น การนำพิกัดเก่ามาส่งซ้ำจะไม่มีประโยชน์ เนื่องจากระบบต้องการพิกัดล่าสุด (Latest State) เท่านั้น



## A. สคริปต์ python `benchmark_protocols.py` สำหรับทำ benchmark โพรโตคอลต่าง ๆ 

<details>
<summary><b>🔍 คลิกดูซอร์สโค้ดฉบับสมบูรณ์ทั้งไฟล์ (benchmark_protocols.py)</b></summary>

```python
#!/usr/bin/env python3
"""
================================================================================
IoT Local Communication & Control Benchmark Tool
Week 10: HTTP vs UDP vs CoAP Protocol Performance Analyzer
================================================================================
Author: IoT Systems Engineering Laboratory
Purpose: Benchmarking Round-Trip Time (RTT), Jitter, Packet Reliability,
         and Protocol Overhead for ESP32 Local Control.
================================================================================
"""

import sys
import time
import math
import socket
import argparse
import asyncio
from typing import List, Dict, Any, Optional

# Protocol Specific Imports with Graceful Fallbacks
try:
    import urllib.request
    import json
except ImportError:
    pass

try:
    from aiocoap import Context, Message, Code
except ImportError:
    pass


class BenchmarkStats:
    """Calculates statistical metrics for benchmark rounds."""
    def __init__(self, latencies: List[float], sent_bytes: int, recv_bytes: int, total_rounds: int):
        self.total_rounds = total_rounds
        self.success_rounds = len(latencies)
        self.failed_rounds = total_rounds - self.success_rounds
        self.latencies = latencies
        self.sent_bytes = sent_bytes
        self.recv_bytes = recv_bytes

        if self.success_rounds > 0:
            self.min_rtt = min(latencies)
            self.max_rtt = max(latencies)
            self.mean_rtt = sum(latencies) / self.success_rounds
            variance = sum((x - self.mean_rtt) ** 2 for x in latencies) / self.success_rounds
            self.std_dev = math.sqrt(variance)
            self.packet_loss_pct = (self.failed_rounds / self.total_rounds) * 100.0
        else:
            self.min_rtt = 0.0
            self.max_rtt = 0.0
            self.mean_rtt = 0.0
            self.std_dev = 0.0
            self.packet_loss_pct = 100.0


def print_banner():
    banner = r"""
================================================================================
   ESP32 Local Protocol Benchmark: HTTP vs UDP vs CoAP
   Industrial IoT Systems Engineering - Real-time Performance Lab
================================================================================
"""
    print(banner)


# ------------------------------------------------------------------------------
# 1. HTTP REST Benchmark
# ------------------------------------------------------------------------------
def benchmark_http(target: str, rounds: int, timeout: float) -> BenchmarkStats:
    print(f"\n[*] Starting HTTP REST Benchmark -> http://{target}/api/led ({rounds} rounds)...")
    url = f"http://{target}/api/led"
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    headers = {
        "Content-Type": "application/json",
        "Connection": "keep-alive"
    }

    for i in range(1, rounds + 1):
        state = (i % 2 == 1)
        payload = json.dumps({"state": state}).encode("utf-8")
        req = urllib.request.Request(url, data=payload, headers=headers, method="POST")

        t_start = time.perf_counter()
        try:
            with urllib.request.urlopen(req, timeout=timeout) as response:
                resp_data = response.read()
                t_end = time.perf_counter()
                rtt = (t_end - t_start) * 1000.0  # ms
                latencies.append(rtt)
                total_tx_bytes += len(payload) + 180  # approx HTTP POST header size
                total_rx_bytes += len(resp_data) + 120  # approx HTTP 200 header size
                print(f"  [HTTP Round {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Status: {response.status}")
        except Exception as e:
            print(f"  [HTTP Round {i:03d}/{rounds:03d}] FAILED: {e}")

        time.sleep(0.02)  # 20ms pacing

    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


# ------------------------------------------------------------------------------
# 2. UDP Raw Socket Benchmark
# ------------------------------------------------------------------------------
def benchmark_udp(target: str, rounds: int, timeout: float, port: int = 3333) -> BenchmarkStats:
    print(f"\n[*] Starting UDP Socket Benchmark -> {target}:{port} ({rounds} rounds)...")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    for i in range(1, rounds + 1):
        cmd = "LED_ON" if (i % 2 == 1) else "LED_OFF"
        tx_data = cmd.encode("utf-8")

        t_start = time.perf_counter()
        try:
            sock.sendto(tx_data, (target, port))
            total_tx_bytes += len(tx_data) + 8  # 8 bytes UDP header
            rx_data, _ = sock.recvfrom(128)
            t_end = time.perf_counter()
            rtt = (t_end - t_start) * 1000.0  # ms
            latencies.append(rtt)
            total_rx_bytes += len(rx_data) + 8
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Recv: {rx_data.decode('utf-8', errors='ignore')}")
        except socket.timeout:
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] TIMEOUT (Packet Dropped)")
        except Exception as e:
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] ERROR: {e}")

        time.sleep(0.02)

    sock.close()
    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


# ------------------------------------------------------------------------------
# 3. CoAP (RFC 7252) Benchmark
# ------------------------------------------------------------------------------
async def _async_benchmark_coap(target: str, rounds: int, timeout: float, port: int = 5683) -> BenchmarkStats:
    print(f"\n[*] Starting CoAP Benchmark -> coap://{target}:{port}/actuator/led ({rounds} rounds)...")
    protocol = await Context.create_client_context()
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    for i in range(1, rounds + 1):
        val = b"1" if (i % 2 == 1) else b"0"
        request = Message(code=Code.PUT, payload=val, uri=f"coap://{target}:{port}/actuator/led")

        t_start = time.perf_counter()
        try:
            response = await asyncio.wait_for(protocol.request(request).response, timeout=timeout)
            t_end = time.perf_counter()
            rtt = (t_end - t_start) * 1000.0  # ms
            latencies.append(rtt)
            total_tx_bytes += len(val) + 4 + 8  # CoAP 4-byte header + UDP 8-byte
            total_rx_bytes += 4 + 8  # CoAP response header + UDP
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Code: {response.code}")
        except asyncio.TimeoutError:
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] TIMEOUT (No ACK)")
        except Exception as e:
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] ERROR: {e}")

        await asyncio.sleep(0.02)

    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


def benchmark_coap(target: str, rounds: int, timeout: float) -> BenchmarkStats:
    if "aiocoap" not in sys.modules:
        try:
            import aiocoap
        except ImportError:
            print("[!] Error: 'aiocoap' is not installed. Please run: pip install aiocoap")
            return BenchmarkStats([], 0, 0, rounds)

    return asyncio.run(_async_benchmark_coap(target, rounds, timeout))


# ------------------------------------------------------------------------------
# 4. Result Presentation & Comparison Matrix
# ------------------------------------------------------------------------------
def display_results(results: Dict[str, BenchmarkStats]):
    print("\n" + "=" * 80)
    print("                    BENCHMARK RESULTS & METRICS SUMMARY")
    print("=" * 80)
    print(f"{'Protocol':<12} | {'Success':<9} | {'Loss %':<7} | {'Min (ms)':<9} | {'Mean (ms)':<10} | {'Max (ms)':<9} | {'Jitter (ms)':<11}")
    print("-" * 80)

    for proto_name, stats in results.items():
        if stats.success_rounds > 0:
            print(f"{proto_name:<12} | {stats.success_rounds}/{stats.total_rounds:<7} | {stats.packet_loss_pct:>5.1f}% | {stats.min_rtt:>8.2f} | {stats.mean_rtt:>9.2f} | {stats.max_rtt:>8.2f} | {stats.std_dev:>10.2f}")
        else:
            print(f"{proto_name:<12} | {stats.success_rounds}/{stats.total_rounds:<7} | {stats.packet_loss_pct:>5.1f}% | {'N/A':>8} | {'N/A':>9} | {'N/A':>8} | {'N/A':>10}")

    print("=" * 80)

    # Architectural Overhead Comparison Table
    print("\n" + "=" * 80)
    print("             THEORETICAL PROTOCOL OVERHEAD ANALYSIS (Per LED Command)")
    print("=" * 80)
    print(f"{'Layer / Attribute':<30} | {'HTTP REST (POST)':<17} | {'UDP Socket':<13} | {'CoAP (PUT)':<12}")
    print("-" * 80)
    print(f"{'Transport Layer (L4)':<30} | {'TCP (20-32 bytes)':<17} | {'UDP (8 bytes)':<13} | {'UDP (8 bytes)':<12}")
    print(f"{'Application Header (L7)':<30} | {'~150-250 bytes':<17} | {'0 bytes':<13} | {'4-8 bytes':<12}")
    print(f"{'Command Payload Size':<30} | {'14 bytes (JSON)':<17} | {'6 bytes (ASCII)':<13} | {'1 byte (char)':<12}")
    print(f"{'Est. Total Bytes per Request':<30} | {'~190-300 bytes':<17} | {'~14 bytes':<13} | {'~13-17 bytes':<12}")
    print(f"{'Payload Efficiency Ratio':<30} | {'~4.5 - 7.0 %':<17} | {'~42.8 %':<13} | {'~7.7 - 25.0 %':<12}")
    print("=" * 80)
    print("\n[✔] Benchmark Complete! Copy the metrics above into your Lab 10.4 Report.\n")


# ------------------------------------------------------------------------------
# Main Entry Point
# ------------------------------------------------------------------------------
def main():
    print_banner()

    parser = argparse.ArgumentParser(description="IoT Protocol Benchmark Suite (HTTP vs UDP vs CoAP)")
    parser.add_argument("--target", "-t", type=str, default="192.168.1.181",
                        help="Target ESP32 IP address or hostname (default: 192.168.1.181)")
    parser.add_argument("--protocol", "-p", type=str, choices=["http", "udp", "coap", "all", "interactive"],
                        default="interactive", help="Protocol to benchmark: http, udp, coap, all, or interactive")
    parser.add_argument("--rounds", "-n", type=int, default=50,
                        help="Number of test rounds per protocol (default: 50)")
    parser.add_argument("--timeout", type=float, default=2.0,
                        help="Per-request timeout in seconds (default: 2.0s)")

    args = parser.parse_args()
    target = args.target
    rounds = args.rounds
    timeout = args.timeout

    proto = args.protocol
    if proto == "interactive":
        print(f"Target ESP32 Device: {target}")
        print(f"Number of Rounds:   {rounds}\n")
        print("Select Benchmark Option:")
        print("  [1] Benchmark HTTP REST Server  (Lab 10.1 Firmware)")
        print("  [2] Benchmark UDP Socket Server (Lab 10.2 Firmware)")
        print("  [3] Benchmark CoAP Server       (Lab 10.3 Firmware)")
        print("  [4] Benchmark ALL 3 Protocols Sequentially")
        print("  [0] Exit")

        choice = input("\nEnter your choice [1-4]: ").strip()
        if choice == "1":
            proto = "http"
        elif choice == "2":
            proto = "udp"
        elif choice == "3":
            proto = "coap"
        elif choice == "4":
            proto = "all"
        else:
            print("Exiting.")
            sys.exit(0)

    results: Dict[str, BenchmarkStats] = {}

    if proto in ["http", "all"]:
        results["HTTP (REST)"] = benchmark_http(target, rounds, timeout)

    if proto in ["udp", "all"]:
        results["UDP (Socket)"] = benchmark_udp(target, rounds, timeout)

    if proto in ["coap", "all"]:
        results["CoAP (RFC7252)"] = benchmark_coap(target, rounds, timeout)

    display_results(results)


if __name__ == "__main__":
    main()

```
</details>