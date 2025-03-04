from pyahr.async_http_requests import Processor, Request, Response, DefaultEventHandler
from time import sleep
from typing import Optional
from logging import Logger, getLogger

logger: Logger = getLogger('mylogger')
logger.setLevel('INFO')

event_handler: DefaultEventHandler = DefaultEventHandler()
url: str = 'http://localhost:8000/'
url1: str = 'http://192.168.0.1'
urltg = url
p: Processor = Processor(url=urltg, event_handler=event_handler, logger=logger)

from time import monotonic_ns

k = 10
request_objects = []
for i in range(0, k):
    request: Request = p.request()
    request.set_ressource('').set_query_parameter({'test1': 'mr.x'}).set_header({'header1': 'test'})
    print(f'Request from processor {request.ressource()}')
    p.prepare_request(
        request
    )
    print(
        p.get(request).name
    )
    request_objects.append(request)

print('--------------------------')
start = monotonic_ns()
for i in range(0, k):
    p.make_request(request_objects[i])

results = []
while len(results) != k:
    r = event_handler.next()
    if r is not None:
        results.append(
            r
        )
end = monotonic_ns()
print('--------------------------')
print(f'DT: {(end - start) / (1000 * 1000 * 1000)}')
print(
    [result.status_code() for result in results]
)

from requests import get
start = monotonic_ns()
for i in range(0, k):
    get(urltg)
end = monotonic_ns()
print(f'DT: {(end - start) / (1000 * 1000 * 1000)}')
