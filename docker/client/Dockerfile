from python:3.10

RUN apt-get update
RUN apt-get -y install git cmake

COPY async_http_requests /async_http_requests
RUN cd /async_http_requests && rm -r build && mkdir build && cd build && cmake -G "Unix Makefiles" ../ && make ahr

COPY python_async_requests/ /python_async_requests
RUN cd /python_async_requests && pip install -r requirements.txt && ./build.sh && cp dist/asynchronous_http_requests-0.0.0.tar.gz /asynchronous_http_requests-0.0.0.tar.gz
RUN pip install /asynchronous_http_requests-0.0.0.tar.gz
COPY system_test/client/main.py /main.py

ENV LIB_AHR_PATH=/async_http_requests/build/libahr.so
CMD python /main.py
#CMD curl 'http://0.0.0.0:8000/std/'
