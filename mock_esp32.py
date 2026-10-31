import socket
import threading
import time
import json
import random
from http.server import HTTPServer, BaseHTTPRequestHandler

# Global states
led_state = False
pot_val = 1820
seq_num = 0

class ESP32HTTPHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        # Override to look like ESP-IDF logs
        pass

    def do_GET(self):
        global led_state, pot_val
        if self.path == '/api/status':
            pot_val = random.randint(1800, 1850)
            data = {
                "pot_raw": pot_val,
                "free_heap": random.randint(214000, 215000),
                "led": led_state
            }
            body = json.dumps(data).encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(body)))
            self.send_header('Connection', 'close')
            self.end_headers()
            self.wfile.write(body)
            print(f"I ({int(time.time()*1000)%100000}) HTTP: GET /api/status -> {data}")
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        global led_state
        if self.path == '/api/led':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                body = json.loads(post_data)
                led_state = bool(body.get('state', False))
                print(f"I ({int(time.time()*1000)%100000}) HTTP: LED (GPIO 2) switched to: {'ON' if led_state else 'OFF'}")
            except Exception:
                pass
            resp = json.dumps({"result": "success"}).encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(resp)))
            self.send_header('Connection', 'close')
            self.end_headers()
            self.wfile.write(resp)
        else:
            self.send_response(404)
            self.end_headers()

def run_http():
    httpd = HTTPServer(('0.0.0.0', 80), ESP32HTTPHandler)
    print("I (100) HTTP: HTTP Server started on port 80")
    httpd.serve_forever()

def run_udp_ctrl():
    global led_state
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', 3333))
    print("I (120) UDP_SERVER: Control Server listening on UDP Port 3333...")
    while True:
        data, addr = sock.recvfrom(128)
        cmd = data.decode('utf-8', errors='ignore').strip()
        print(f"I ({int(time.time()*1000)%100000}) UDP_SERVER: Received command: {cmd}")
        if cmd == "LED_ON":
            led_state = True
            sock.sendto(b"ACK:LED_ON", addr)
        elif cmd == "LED_OFF":
            led_state = False
            sock.sendto(b"ACK:LED_OFF", addr)
        else:
            sock.sendto(b"ACK:UNKNOWN", addr)

def run_udp_bcast():
    global seq_num, pot_val
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    print("I (140) UDP_BCAST: Broadcasting Telemetry to Port 3334 (10 Hz)...")
    while True:
        pot_val = random.randint(1800, 1850)
        msg = f"SEQ:{seq_num},POT:{pot_val}\n".encode('utf-8')
        try:
            sock.sendto(msg, ('127.0.0.1', 3334))
            sock.sendto(msg, ('255.255.255.255', 3334))
        except Exception:
            pass
        seq_num += 1
        time.sleep(0.1)

def run_coap():
    global led_state, pot_val
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', 5683))
    print("I (160) COAP_LAB: CoAP Server listening on port 5683...")
    while True:
        data, addr = sock.recvfrom(512)
        if len(data) < 4:
            continue
        v_t_tkl = data[0]
        code = data[1]
        msg_id = (data[2] << 8) | data[3]
        tkl = v_t_tkl & 0x0F
        token = data[4:4+tkl]
        raw_rest = data[4+tkl:]

        # Check options for path
        payload = b""
        if b'\xff' in raw_rest:
            options_bytes, payload = raw_rest.split(b'\xff', 1)
        else:
            options_bytes = raw_rest

        # Path heuristics
        path = ""
        if b"core" in data:
            path = "core"
        elif b"pot" in data:
            path = "pot"
        elif b"led" in data:
            path = "led"

        # Determine response
        # 2.05 Content = 69 (0x45), 2.04 Changed = 68 (0x44)
        if code == 1: # GET
            if "core" in path:
                resp_payload = b'</sensor/pot>,</actuator/led>'
                resp_code = 69 # 2.05 Content
                print(f"I ({int(time.time()*1000)%100000}) COAP_LAB: GET /.well-known/core -> {resp_payload.decode()}")
            else:
                resp_payload = str(random.randint(1800, 1850)).encode('utf-8')
                resp_code = 69 # 2.05 Content
                print(f"I ({int(time.time()*1000)%100000}) COAP_LAB: GET /sensor/pot -> {resp_payload.decode()}")
        elif code in (2, 3): # POST or PUT
            if payload == b'1':
                led_state = True
                print(f"I ({int(time.time()*1000)%100000}) COAP_LAB: LED turned ON via CoAP PUT")
            elif payload == b'0':
                led_state = False
                print(f"I ({int(time.time()*1000)%100000}) COAP_LAB: LED turned OFF via CoAP PUT")
            resp_payload = b""
            resp_code = 68 # 2.04 Changed
        else:
            resp_payload = b""
            resp_code = 68

        # Build ACK response: Ver=1, Type=2 (ACK), TKL=tkl
        resp_hdr = bytearray()
        resp_hdr.append((1 << 6) | (2 << 4) | tkl)
        resp_hdr.append(resp_code)
        resp_hdr.append(data[2])
        resp_hdr.append(data[3])
        resp_hdr.extend(token)
        if resp_payload:
            resp_hdr.append(0xFF)
            resp_hdr.extend(resp_payload)
        sock.sendto(bytes(resp_hdr), addr)

if __name__ == '__main__':
    print("==================================================")
    print("   ESP32 Mock Server Running on localhost (127.0.0.1)")
    print("   Simulating HTTP (80), UDP (3333, 3334), CoAP (5683)")
    print("==================================================")
    t1 = threading.Thread(target=run_http, daemon=True)
    t2 = threading.Thread(target=run_udp_ctrl, daemon=True)
    t3 = threading.Thread(target=run_udp_bcast, daemon=True)
    t4 = threading.Thread(target=run_coap, daemon=True)
    t1.start()
    t2.start()
    t3.start()
    t4.start()

    while True:
        try:
            time.sleep(1)
        except KeyboardInterrupt:
            break

