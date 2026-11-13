import asyncio
import sys
from aiocoap import Context, Message, Code

# กำหนด IP ของบอร์ด ESP32 (ส่งผ่าน argument ได้ เช่น python test_coap.py 192.168.1.50)
ESP32_IP = sys.argv[1] if len(sys.argv) > 1 else "192.168.1.181"

async def main():
    protocol = await Context.create_client_context()
    
    # 1. ทดสอบค้นหารายการทรัพยากร: GET /.well-known/core
    print("\n--- 1. ทดสอบ CoAP Resource Discovery (/.well-known/core) ---")
    request_core = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/.well-known/core")
    response_core = await protocol.request(request_core).response
    print("รายการ Resource ทั้งหมดที่พบบนบอร์ด (CoRE Link Format):")
    print(response_core.payload.decode("utf-8"))

    # 2. ทดสอบอ่านค่าเซนเซอร์: GET /sensor/pot
    print("\n--- 2. ทดสอบ CoAP GET /sensor/pot ---")
    request_pot = Message(code=Code.GET, uri=f"coap://{ESP32_IP}/sensor/pot")
    response_pot = await protocol.request(request_pot).response
    print(f"ค่าเซนเซอร์ Potentiometer ที่อ่านได้: {response_pot.payload.decode('utf-8')} (รหัสสถานะ: {response_pot.code})")

    # 3. ทดสอบสั่งเปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '1'
    print("\n--- 3. ทดสอบ CoAP PUT /actuator/led (สั่งเปิดไฟ) ---")
    request_on = Message(code=Code.PUT, payload=b"1", uri=f"coap://{ESP32_IP}/actuator/led")
    response_on = await protocol.request(request_on).response
    print(f"สถานะตอบกลับหลังสั่งเปิดไฟ: {response_on.code}")

    await asyncio.sleep(2)

    # 4. ทดสอบสั่งปิดไฟ LED: PUT /actuator/led ด้วยข้อมูล '0'
    print("\n--- 4. ทดสอบ CoAP PUT /actuator/led (สั่งปิดไฟ) ---")
    request_off = Message(code=Code.PUT, payload=b"0", uri=f"coap://{ESP32_IP}/actuator/led")
    response_off = await protocol.request(request_off).response
    print(f"สถานะตอบกลับหลังสั่งปิดไฟ: {response_off.code}")

if __name__ == "__main__":
    asyncio.run(main())
