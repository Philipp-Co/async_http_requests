#!/bin/bash
cppcheck --addon=misra -I async_http_requests/inc/ -I async_http_request/src/external/inc/ -I async_http_request/src/private/inc/ --inline-suppr --inconclusive --enable=style async_http_requests/src/
