import asyncio
import sys
# pyrefly: ignore [missing-import]
from aiocoap import Context, Message, Code

# กำหนดหมายเลข IP ของบอร์ด ESP32
ESP32_IP = "172.20.10.2"

async def main():
    print(f"Connecting to CoAP Server at {ESP32_IP}:5683...", flush=True)
    protocol = await Context.create_client_context()
    
    try:
        # 1. ทดสอบ Resource Discovery: GET /.well-known/core
        print("\n--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---", flush=True)
        request_core = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/.well-known/core")
        response_core = await asyncio.wait_for(protocol.request(request_core).response, timeout=5.0)
        print("Resource Directory (CoRE Link Format):", flush=True)
        print(response_core.payload.decode("utf-8"), flush=True)

        # 2. ทดสอบอ่านค่าเซนเซอร์: GET /sensor/pot
        print("\n--- 2. Testing CoAP GET /sensor/pot ---", flush=True)
        request_pot = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/sensor/pot")
        response_pot = await asyncio.wait_for(protocol.request(request_pot).response, timeout=5.0)
        print(f"Potentiometer Value: {response_pot.payload.decode('utf-8')} (Code: {response_pot.code})", flush=True)

        # 3. ทดสอบสั่งเปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '1'
        print("\n--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---", flush=True)
        request_on = Message(code=Code.PUT, payload=b"1", uri=f"coap://{ESP32_IP}/actuator/led")
        response_on = await asyncio.wait_for(protocol.request(request_on).response, timeout=5.0)
        print(f"LED ON Response Code: {response_on.code}", flush=True)

        print("Waiting 2 seconds...", flush=True)
        await asyncio.sleep(2)

        # 4. ทดสอบสั่งปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '0'
        print("\n--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---", flush=True)
        request_off = Message(code=Code.PUT, payload=b"0", uri=f"coap://{ESP32_IP}/actuator/led")
        response_off = await asyncio.wait_for(protocol.request(request_off).response, timeout=5.0)
        print(f"LED OFF Response Code: {response_off.code}", flush=True)

        print("\n--- CoAP Test Completed Successfully! ---", flush=True)

    except asyncio.TimeoutError:
        print(f"\n[Error] Connection timed out: Could not reach ESP32 at {ESP32_IP}:5683", flush=True)
        print("Please check that:", flush=True)
        print("1. ESP32 is connected to Wi-Fi (IP: 172.20.10.2)", flush=True)
        print("2. Your laptop is connected to the same Wi-Fi hotspot", flush=True)
    except Exception as e:
        print(f"\n[Error] CoAP Request failed: {e}", flush=True)

if __name__ == "__main__":
    asyncio.run(main())
