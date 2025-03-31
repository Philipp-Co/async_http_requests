from pyahr.async_http_requests import Processor, Request, Response, DefaultEventHandler
from time import sleep
from typing import Optional
from logging import Logger, getLogger

logger: Logger = getLogger('mylogger')
logger.setLevel('INFO')

event_handler: DefaultEventHandler = DefaultEventHandler()
url: str = 'http://localhost:8000/'
url1: str = 'https://www.google.de'
urltg = url
p: Processor = Processor(url=urltg, event_handler=event_handler, logger=logger)

from time import monotonic_ns

k = 1
request_objects = []
for i in range(0, k):
    request: Request = p.request()
    request.set_ressource('std/').set_query_parameter({'test1': 'mr.x'}).set_header({'header1': 'test'})
    print(f'Request from processor {request.ressource()}')
    p.get(
        request
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
from pprint import pprint
pprint(
    [result.to_repr() for result in results]
)

from requests import get, Session
s: Session = Session()
start = monotonic_ns()
for i in range(0, k):
    s.get(urltg)
end = monotonic_ns()
s.close()
print(f'DT: {(end - start) / (1000 * 1000 * 1000)}')
