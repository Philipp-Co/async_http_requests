#!/python

from pyahr.async_http_requests import AHR_HttpRequestProcessor, AHR_Request, AHR_Response, AHR_DefaultEventHandler, AHR_HttpMethod, AHR_HttpProcessorFlowError
from logging import Logger, getLogger, Formatter, StreamHandler
from time import sleep, monotonic_ns
from json import loads
from client_state import CountToN


logger: Logger = getLogger('mylogger')
logger.setLevel('INFO')
# create console handler and set level to debug
ch = StreamHandler()

# create formatter
formatter = Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

# add formatter to ch
ch.setFormatter(formatter)

# add ch to logger
logger.addHandler(ch)

# Construct HttpRequestProcessor.
host: str = '127.0.0.1'
objects_name: str = 'test'
counter: CountToN = CountToN(url=f'http://{host}:8000/', name=objects_name, logger=logger)
counter.start()
while not counter.finished():
    counter.disptach_event()

exit(0)

event_handler: AHR_DefaultEventHandler = AHR_DefaultEventHandler()
p: AHR_HttpRequestProcessor = AHR_HttpRequestProcessor(url=f'http://{host}:8000/', event_handler=event_handler, logger=logger)

# Construct Request-Object, prepare and configure the HttpRequestProcessor

try:
    put_request: AHR_Request = p.create_request().set_http_method(
        AHR_HttpMethod.POST
    ).set_ressource(
        f'std/{objects_name}/'
    ).set_body(
        '{"test": 1}'
    ).set_query_parameter(
        {
            "q0": 1,
            "q1": "arg",
        }
    )
    p.configure_request(put_request)
    p.make_request(
        put_request
    )
    while True:
        r = event_handler.next()
        if r is not None:
            exit(0)
except AHR_HttpProcessorFlowError as e:
    print(f'Status: {e.status()}')
    exit(0)

request: AHR_Request = p.create_request().set_http_method(
    AHR_HttpMethod.PUT
).set_ressource(
    f'std/{objects_name}/'
).set_query_parameter(
    {'test1': 'mr.x'}
).set_header(
    {'header1': 'test'}
)
p.configure_request(request)

# Use the Request-Objekt
while True:
    p.make_request(request)

    r = event_handler.next()
    if r is not None:
        try:
            if r.is_valid():
                #logger.info(
                #    loads(
                #        r.body()
                #    )['datetime']
                #)
                print('Request valid.')
            else:
                print('Request invalid...')
        except Exception as e:
            logger.exception(e)
    sleep(1)
exit(0)
