import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 3333))
print("Listening for ESP32 UDP Telemetry Broadcast on port 3333...")
while True:
    data, addr = sock.recvfrom(1024)
    print(f"From {addr}: {data.decode().strip()}")
