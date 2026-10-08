import socket, time, sys

IP = "172.20.10.2"   # IP ของ ESP32 (จาก serial log)
PORT = 5683
TYPES = {0: "CON", 1: "NON", 2: "ACK", 3: "RST"}

def build(mtype, mid, payload):
    hdr = bytes([0x40 | (mtype << 4), 0x03]) + mid.to_bytes(2, "big")  # PUT = 0.03
    opts = bytes([0xB8]) + b"actuator" + bytes([0x03]) + b"led"
    return hdr + opts + b"\xff" + payload

def parse(b):
    t = (b[0] >> 4) & 3
    code = f"{b[1] >> 5}.{b[1] & 0x1f:02d}"
    return TYPES[t], code, int.from_bytes(b[2:4], "big")

def send(mtype, mid, payload):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    pkt = build(mtype, mid, payload)
    timeout, t0 = 2.0, time.time()
    tries = 5 if mtype == 0 else 1          # CON ส่งซ้ำได้ 4 ครั้ง
    for n in range(tries):
        s.settimeout(timeout)
        s.sendto(pkt, (IP, PORT))
        tag = "ส่ง" if n == 0 else f"ส่งซ้ำครั้งที่ {n}"
        print(f"[{time.time()-t0:5.2f}s] {tag}: {TYPES[mtype]} MID={mid} payload={payload}")
        try:
            r, _ = s.recvfrom(256)
            ty, code, rmid = parse(r)
            print(f"[{time.time()-t0:5.2f}s] ได้รับ: {ty} code={code} MID={rmid}")
            return
        except socket.timeout:
            print(f"[{time.time()-t0:5.2f}s] ไม่ได้รับ ACK/response ภายใน {timeout:.0f}s")
            timeout *= 2
    print("ล้มเหลว: ไม่ได้รับการตอบกลับ")

mid = 0x1000
for i in range(10):
    send(0, mid + i, b"1" if i % 2 == 0 else b"0")        # CON
for i in range(10):
    send(1, mid + 100 + i, b"1" if i % 2 == 0 else b"0")  # NON