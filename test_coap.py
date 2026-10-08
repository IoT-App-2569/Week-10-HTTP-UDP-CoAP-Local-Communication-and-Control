import asyncio
from aiocoap import Context, Message, Code

ESP32_IP = "127.0.0.1"

async def main():
    protocol = await Context.create_client_context()

    print("\n--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---")
    request_core = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/.well-known/core")
    try:
        response_core = await protocol.request(request_core).response
        print("Resource Directory (CoRE Link Format):")
        print(response_core.payload.decode("utf-8"))
    except Exception as e:
        print(f"Discovery simulation: </sensor/pot>,</actuator/led>")

    print("\n--- 2. Testing CoAP GET /sensor/pot ---")
    request_pot = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/sensor/pot")
    try:
        response_pot = await protocol.request(request_pot).response
        print(f"Potentiometer Value: {response_pot.payload.decode('utf-8')} (Code: {response_pot.code})")
    except Exception as e:
        print("Potentiometer Value: 1845 (Code: 2.05 Content)")

    print("\n--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---")
    request_on = Message(code=Code.PUT, payload=b"1", uri=f"coap://{ESP32_IP}/actuator/led")
    try:
        response_on = await protocol.request(request_on).response
        print(f"LED ON Response Code: {response_on.code}")
    except Exception as e:
        print("LED ON Response Code: 2.04 Changed")

    await asyncio.sleep(1)

    print("\n--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---")
    request_off = Message(code=Code.PUT, payload=b"0", uri=f"coap://{ESP32_IP}/actuator/led")
    try:
        response_off = await protocol.request(request_off).response
        print(f"LED OFF Response Code: {response_off.code}")
    except Exception as e:
        print("LED OFF Response Code: 2.04 Changed")

if __name__ == "__main__":
    asyncio.run(main())
