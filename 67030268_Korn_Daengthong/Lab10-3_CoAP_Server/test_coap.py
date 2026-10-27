#!/usr/bin/env python3
"""
Lab 10.3: CoAP Discovery & Control Client (RFC 7252 / RFC 6690)
Student: นายกร แดงทอง (67030268)
"""
import argparse
import asyncio
import time
from aiocoap import Context, Message, Code

parser = argparse.ArgumentParser(description="CoAP Client Test Script")
parser.add_argument("--target", "-t", type=str, default="172.20.10.6", help="ESP32 IP Address")
args = parser.parse_args()

ESP32_IP = args.target

async def main():
    protocol = await Context.create_client_context()

    # 1. ทดสอบ Resource Discovery: GET /.well-known/core
    print("\n--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---")
    t0 = time.perf_counter()
    request_core = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/.well-known/core")
    response_core = await protocol.request(request_core).response
    rtt1 = (time.perf_counter() - t0) * 1000.0
    print("Resource Directory (CoRE Link Format):")
    print(response_core.payload.decode("utf-8"))
    print(f"[COAP] Type: CON | Message ID: {response_core.mid} | Response: {response_core.code} | RTT: {rtt1:.2f} ms")

    # 2. ทดสอบอ่านค่าเซนเซอร์: GET /sensor/pot
    print("\n--- 2. Testing CoAP GET /sensor/pot ---")
    t0 = time.perf_counter()
    request_pot = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/sensor/pot")
    response_pot = await protocol.request(request_pot).response
    rtt2 = (time.perf_counter() - t0) * 1000.0
    print(f"Potentiometer Value: {response_pot.payload.decode('utf-8')} (Code: {response_pot.code})")
    print(f"[COAP] Type: CON | Message ID: {response_pot.mid} | Response: {response_pot.code} | RTT: {rtt2:.2f} ms")

    # 3. ทดสอบสั่งเปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '1'
    print("\n--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---")
    t0 = time.perf_counter()
    request_on = Message(code=Code.PUT, payload=b"1", uri=f"coap://{ESP32_IP}/actuator/led")
    response_on = await protocol.request(request_on).response
    rtt3 = (time.perf_counter() - t0) * 1000.0
    print(f"LED ON Response Code: {response_on.code}")
    print(f"[COAP] Type: CON | Message ID: {response_on.mid} | Response: {response_on.code} | RTT: {rtt3:.2f} ms")

    await asyncio.sleep(1)

    # 4. ทดสอบสั่งปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '0'
    print("\n--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---")
    t0 = time.perf_counter()
    request_off = Message(code=Code.PUT, payload=b"0", uri=f"coap://{ESP32_IP}/actuator/led")
    response_off = await protocol.request(request_off).response
    rtt4 = (time.perf_counter() - t0) * 1000.0
    print(f"LED OFF Response Code: {response_off.code}")
    print(f"[COAP] Type: CON | Message ID: {response_off.mid} | Response: {response_off.code} | RTT: {rtt4:.2f} ms")

if __name__ == "__main__":
    asyncio.run(main())
