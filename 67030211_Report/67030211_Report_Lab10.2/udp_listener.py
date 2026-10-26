import socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('0.0.0.0', 3333))
print("Listening on UDP port 3333...")
data, addr = s.recvfrom(1024)
print(f"Received from {addr}: {data.decode()}")
