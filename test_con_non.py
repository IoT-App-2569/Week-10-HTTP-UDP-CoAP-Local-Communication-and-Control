import asyncio, time, sys
from aiocoap import Context, Message, Code, CON, NON

ESP32_IP = "172.20.10.2"   # แก้ให้ตรงกับ ESP32
N = 30                       # จำนวนครั้งต่อโหมด
URI = f"coap://{ESP32_IP}/actuator/led"

async def run(protocol, mtype, name):
    ok = fail = 0
    times = []
    for i in range(N):
        payload = b"1" if i % 2 == 0 else b"0"
        req = Message(code=Code.PUT, mtype=mtype, payload=payload, uri=URI)
        t0 = time.perf_counter()
        try:
            # NON ใช้ timeout สั้น เพราะไม่มี ACK (แต่ server ยังตอบ NON กลับ)
            await asyncio.wait_for(protocol.request(req).response,
                                   timeout=30 if mtype == CON else 5)
            times.append((time.perf_counter() - t0) * 1000)
            ok += 1
        except Exception:
            fail += 1
        await asyncio.sleep(0.2)
    avg = sum(times) / len(times) if times else 0
    mx = max(times) if times else 0
    print(f"| {name} | {N} | {ok} | {fail} | {avg:.1f} | {mx:.1f} |")

async def main():
    protocol = await Context.create_client_context()
    print(f"\n### ผลทดลอง ({sys.argv[1] if len(sys.argv) > 1 else 'unnamed'})")
    print("| โหมด | ส่ง | สำเร็จ | ล้มเหลว | เฉลี่ย (ms) | สูงสุด (ms) |")
    print("|---|---|---|---|---|---|")
    await run(protocol, CON, "CON")
    await run(protocol, NON, "NON")

asyncio.run(main())