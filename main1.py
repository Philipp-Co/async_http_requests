from python_async_requests.pyahr.async_http_requests import Processor, Request, Response, DefaultEventHandler
from time import sleep
from typing import Optional


event_handler: DefaultEventHandler = DefaultEventHandler()
p: Processor = Processor(url='www.google.de', event_handler=event_handler)
p.get(
    Request(ressource='')
)

event_handler1: DefaultEventHandler = DefaultEventHandler()
p1: Processor = Processor(url='www.amazon.de', event_handler=event_handler1)
p1.get(
    Request(ressource='')
)

sleep(5)
r: Optional[Response] = event_handler.next()
if r is not None:
    print(
        r
    )

print('----------------------------------------------------------------------------')
print('----------------------------------------------------------------------------')

r1: Optional[Response] = event_handler1.next()
if r1 is not None:
    print(
        r1
    )

