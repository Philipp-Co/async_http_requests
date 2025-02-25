from pyahr.async_http_requests import Processor, Request, Response, DefaultEventHandler
from time import sleep
from typing import Optional
from logging import Logger, getLogger

logger: Logger = getLogger('mylogger')
logger.setLevel('INFO')

event_handler: DefaultEventHandler = DefaultEventHandler()
p: Processor = Processor(url='www.google.de', event_handler=event_handler, logger=logger)
p.get(
    Request(ressource='').set_header(
        {'Connection': 'keep-alive'}
    )
)

sleep(2)
r: Optional[Response] = event_handler.next()
if r is not None:
    print(
        r
    )

