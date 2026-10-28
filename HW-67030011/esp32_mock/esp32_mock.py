"""ตัวจำลองบอร์ด ESP32 สำหรับใบงานสัปดาห์ที่ 10 (ใช้แทนบอร์ดจริงเมื่อไม่มีฮาร์ดแวร์)

จำลองพฤติกรรมฝั่ง Network ของเฟิร์มแวร์ทั้ง 3 โปรเจกต์ตามซอร์สใน Lab10-*/main/*.c:
  * 10.1 HTTP REST (พอร์ต 80): GET /api/status, POST /api/led
        - Response Header รูปแบบเดียวกับ esp_http_server (httpd_txrx.c): Status line, Content-Type, Content-Length
        - mDNS: ประกาศ esp32-node.local + บริการ _http._tcp ผ่าน dns-sd ของ macOS (ชี้ไปที่ 127.0.0.1)
  * 10.2 UDP: รับคำสั่ง LED_ON/LED_OFF ที่พอร์ต 3333 ตอบ ACK และ Broadcast "SEQ:<n>,POT:<raw>\\n" ไปพอร์ต 3334 ทุก 100 ms
  * 10.3 CoAP (พอร์ต 5683): GET /sensor/pot, PUT /actuator/led พร้อมแอตทริบิวต์ ct/rt/if ใน /.well-known/core

สิ่งที่จำลอง (ไม่ใช่ค่าจริงจากฮาร์ดแวร์):
  * ค่า Potentiometer (ADC 12-bit) เป็นคลื่น Sine 0..4095 คาบ 20 วินาที + Noise เล็กน้อย
  * free_heap เป็นค่าคงที่โดยประมาณ ไม่ได้วัดจาก ESP32
  * "LED GPIO 2" เป็นตัวแปรในหน่วยความจำ แสดงสถานะผ่าน Log แทนหลอดไฟจริง
  * ทุกอย่างรันบน localhost จึงไม่มีผลของ Wi-Fi (RTT/Packet loss ต่ำกว่าบอร์ดจริง)

usage: python esp32_mock.py [--lab http|udp|coap|all]
"""
import argparse
import asyncio
import json
import math
import random
import shutil
import socket
import subprocess
import time

import aiocoap
import aiocoap.resource as resource

BOOT = time.monotonic()
LED_GPIO = 2
FREE_HEAP_SIM = 233_000  # ค่าจำลอง ไม่ได้วัดจากบอร์ดจริง
state = {"led": False}


def log(tag, msg):
    ms = int((time.monotonic() - BOOT) * 1000)
    print(f"I ({ms}) {tag}: {msg}", flush=True)


def pot_raw():
    t = time.monotonic() - BOOT
    v = 2047.5 + 2047.5 * math.sin(2 * math.pi * t / 20.0) + random.uniform(-12, 12)
    return max(0, min(4095, int(v)))


def set_led(on, tag, how):
    state["led"] = on
    log(tag, f"LED (GPIO {LED_GPIO}) switched to: {'ON' if on else 'OFF'}{how}")


# ---------------------------------------------------------------- 10.1 HTTP + mDNS
def http_response(body: bytes, status="200 OK", ctype="application/json"):
    # รูปแบบเดียวกับ esp_http_server: "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n" + "\r\n"
    head = f"HTTP/1.1 {status}\r\nContent-Type: {ctype}\r\nContent-Length: {len(body)}\r\n\r\n"
    return head.encode() + body


async def http_client(reader, writer):
    tag = "HTTP_SERVER"
    try:
        while True:
            head = await reader.readuntil(b"\r\n\r\n")
            lines = head.decode(errors="replace").split("\r\n")
            method, path, _ = lines[0].split(" ", 2)
            hdrs = {k.lower(): v.strip() for k, v in (l.split(":", 1) for l in lines[1:] if ":" in l)}
            body = await reader.readexactly(int(hdrs.get("content-length", "0")))
            if method == "GET" and path == "/api/status":
                payload = {"student_id": "67030011", "pot_raw": pot_raw(), "free_heap": FREE_HEAP_SIM, "led": state["led"]}
                resp = http_response(json.dumps(payload, separators=(",", ":")).encode())
            elif method == "POST" and path == "/api/led":
                try:
                    s = json.loads(body).get("state")
                    if isinstance(s, bool):
                        set_led(s, tag, "")
                except ValueError:
                    pass
                resp = http_response(b'{"result":"success"}')
            else:
                resp = http_response(b"Nothing matches the given URI", "404 Not Found", "text/html")
            writer.write(resp)
            await writer.drain()
            if hdrs.get("connection", "").lower() == "close":
                break
    except (asyncio.IncompleteReadError, ConnectionError, ValueError):
        pass
    writer.close()


def start_mdns():
    if not shutil.which("dns-sd"):
        log("MDNS", "dns-sd not found - skip mDNS")
        return None
    # proxy registration: ประกาศ host esp32-node.local -> 127.0.0.1 และบริการ _http._tcp พอร์ต 80 (TXT เหมือนเฟิร์มแวร์)
    p = subprocess.Popen(["dns-sd", "-P", "ESP32-WebControl", "_http._tcp", "local", "80",
                          "esp32-node.local", "127.0.0.1", "board=esp32", "path=/"],
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    log("MDNS", "mDNS initialized! Hostname: http://esp32-node.local (-> 127.0.0.1)")
    return p


# ---------------------------------------------------------------- 10.2 UDP
class UdpControl(asyncio.DatagramProtocol):
    def connection_made(self, transport):
        self.t = transport

    def datagram_received(self, data, addr):
        cmd = data.decode(errors="replace")
        if cmd == "LED_ON":
            set_led(True, "UDP_SERVER", f" (from {addr[0]}:{addr[1]})")
            self.t.sendto(b"ACK:LED_ON", addr)
        elif cmd == "LED_OFF":
            set_led(False, "UDP_SERVER", f" (from {addr[0]}:{addr[1]})")
            self.t.sendto(b"ACK:LED_OFF", addr)
        else:
            self.t.sendto(b"ACK:UNKNOWN", addr)


async def udp_broadcast():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    log("UDP_BROADCAST", "Broadcasting Telemetry to Port 3334 (10 Hz)...")
    seq = 0
    while True:
        sock.sendto(f"SEQ:{seq},POT:{pot_raw()}\n".encode(), ("255.255.255.255", 3334))
        seq += 1
        await asyncio.sleep(0.1)


# ---------------------------------------------------------------- 10.3 CoAP
class PotResource(resource.Resource):
    def get_link_description(self):
        return dict(super().get_link_description(), ct="0", rt="sensor.pot", **{"if": "core.s"})

    async def render_get(self, request):
        v = str(pot_raw())
        log("COAP_SERVER", f"GET /sensor/pot -> {v}")
        return aiocoap.Message(code=aiocoap.CONTENT, payload=v.encode())


class LedResource(resource.Resource):
    def get_link_description(self):
        return dict(super().get_link_description(), rt="actuator.led", **{"if": "core.a"})

    async def render_put(self, request):
        if request.payload[:1] == b"1":
            set_led(True, "COAP_SERVER", " via CoAP PUT")
        elif request.payload[:1] == b"0":
            set_led(False, "COAP_SERVER", " via CoAP PUT")
        return aiocoap.Message(code=aiocoap.CHANGED)


async def main(lab):
    print("=" * 64)
    print(" ESP32 MOCK (Week 10) - simulated board, no hardware attached")
    print("=" * 64, flush=True)
    tasks = []
    try:
        await serve(lab, tasks)
        await asyncio.Future()
    finally:
        if state.get("mdns"):
            state["mdns"].terminate()


async def serve(lab, tasks):
    if lab in ("http", "all"):
        await asyncio.start_server(http_client, "0.0.0.0", 80)
        log("HTTP_SERVER", "HTTP Server started on port 80")
        state["mdns"] = start_mdns()
    if lab in ("udp", "all"):
        loop = asyncio.get_running_loop()
        await loop.create_datagram_endpoint(UdpControl, local_addr=("0.0.0.0", 3333))
        log("UDP_SERVER", "Control Server listening on UDP Port 3333...")
        tasks.append(asyncio.create_task(udp_broadcast()))
    if lab in ("coap", "all"):
        site = resource.Site()
        site.add_resource([".well-known", "core"], resource.WKCResource(site.get_resources_as_linkheader, impl_info=None))
        site.add_resource(["sensor", "pot"], PotResource())
        site.add_resource(["actuator", "led"], LedResource())
        await aiocoap.Context.create_server_context(site, bind=("127.0.0.1", 5683))
        log("COAP_SERVER", "CoAP Server listening on port 5683...")


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--lab", choices=["http", "udp", "coap", "all"], default="all")
    try:
        asyncio.run(main(ap.parse_args().lab))
    except KeyboardInterrupt:
        pass
