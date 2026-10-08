import asyncio
from aiocoap import Context, Message, Code

ESP32_IP = "172.20.10.2"  # แก้ไขให้ตรงกับหมายเลข IP ของ ESP32

async def main():
    protocol = await Context.create_client_context()
    
    # 1. ทดสอบ Resource Discovery: GET /.well-known/core
    print("\n--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---")
    request_core = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/.well-known/core")
    response_core = await protocol.request(request_core).response
    print("Resource Directory (CoRE Link Format):")
    print(response_core.payload.decode("utf-8"))

    # 2. ทดสอบอ่านค่าเซนเซอร์: GET /sensor/pot
    print("\n--- 2. Testing CoAP GET /sensor/pot ---")
    request_pot = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/sensor/pot")
    response_pot = await protocol.request(request_pot).response
    print(f"Potentiometer Value: {response_pot.payload.decode('utf-8')} (Code: {response_pot.code})")

    # 3. ทดสอบสั่งเปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '1'
    print("\n--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---")
    request_on = Message(code=Code.PUT, payload=b"1", uri=f"coap://{ESP32_IP}/actuator/led")
    response_on = await protocol.request(request_on).response
    print(f"LED ON Response Code: {response_on.code}")

    await asyncio.sleep(2)

    # 4. ทดสอบสั่งปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '0'
    print("\n--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---")
    request_off = Message(code=Code.PUT, payload=b"0", uri=f"coap://{ESP32_IP}/actuator/led")
    response_off = await protocol.request(request_off).response
    print(f"LED OFF Response Code: {response_off.code}")

if __name__ == "__main__":
    asyncio.run(main())