# Python CoAP Client request (aiocoap)
import asyncio
from aiocoap import *

async def main():
    protocol = await Context.create_client_context()
    request = Message(code=GET, uri='coap://192.168.1.105:5683/sensor')
    response = await protocol.request(request).response
    print('CoAP Response Code: %s\nPayload: %s'%(response.code, response.payload.decode()))

asyncio.run(main())
