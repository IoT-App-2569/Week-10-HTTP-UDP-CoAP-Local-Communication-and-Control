import socket
import time

ESP32_IP = "127.0.0.1"
PORT = 5683

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(2.0)

print("\n--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---")
# CoAP CON GET message for core: Ver=1, Type=0, TKL=0, Code=1 (GET), ID=0x0001
sock.sendto(b'\x40\x01\x00\x01core', (ESP32_IP, PORT))
resp, _ = sock.recvfrom(256)
payload = resp.split(b'\xff')[1] if b'\xff' in resp else resp
print("Resource Directory (CoRE Link Format):")
print(payload.decode('utf-8', errors='ignore'))

print("\n--- 2. Testing CoAP GET /sensor/pot ---")
sock.sendto(b'\x40\x01\x00\x02pot', (ESP32_IP, PORT))
resp, _ = sock.recvfrom(256)
payload = resp.split(b'\xff')[1] if b'\xff' in resp else resp
print(f"Potentiometer Value: {payload.decode('utf-8', errors='ignore')} (Code: 2.05 Content)")

print("\n--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---")
sock.sendto(b'\x40\x03\x00\x03led\xff1', (ESP32_IP, PORT))
resp, _ = sock.recvfrom(256)
print("LED ON Response Code: 2.04 Changed")

time.sleep(1)

print("\n--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---")
sock.sendto(b'\x40\x03\x00\x04led\xff0', (ESP32_IP, PORT))
resp, _ = sock.recvfrom(256)
print("LED OFF Response Code: 2.04 Changed")

