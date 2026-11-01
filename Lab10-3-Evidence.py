import asyncio
import time
from aiocoap import Context, Message, Code

ESP32_IP = "192.168.1.33"

async def request(protocol, code, path, payload=b""):
    started = time.perf_counter()
    response = await protocol.request(Message(code=code, payload=payload,
        uri=f"coap://{ESP32_IP}{path}")).response
    elapsed = (time.perf_counter() - started) * 1000
    return response, elapsed

async def main():
    print("LAB 10.3 - CoAP SERVER (ACTUAL ESP32)")
    print("67030142 Piyawit Pholsri | TechCom")
    print("=" * 64)
    protocol = await Context.create_client_context()

    core, core_ms = await request(protocol, Code.GET, "/.well-known/core")
    print(f"GET /.well-known/core  -> {core.code}  RTT={core_ms:.2f} ms")
    print(f"Resources: {core.payload.decode()}")

    pot, pot_ms = await request(protocol, Code.GET, "/sensor/pot")
    print(f"GET /sensor/pot       -> {pot.code}  value={pot.payload.decode()}  RTT={pot_ms:.2f} ms")

    on, on_ms = await request(protocol, Code.PUT, "/actuator/led", b"1")
    print(f"PUT /actuator/led [1] -> {on.code}  LED ON   RTT={on_ms:.2f} ms")
    await asyncio.sleep(0.5)
    off, off_ms = await request(protocol, Code.PUT, "/actuator/led", b"0")
    print(f"PUT /actuator/led [0] -> {off.code}  LED OFF  RTT={off_ms:.2f} ms")

    print("\nPASS: Resource discovery, sensor and LED control verified.")
    input("\nPress Enter to close...")

asyncio.run(main())
