import asyncio
from aiocoap import *
async def main():
    ctx = await Context.create_client_context()
    req = Message(code=GET, uri='coap://192.168.1.105:5683/sensor')
    res = await ctx.request(req).response
    print("CoAP Response:", res.payload.decode())
asyncio.run(main())
