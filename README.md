# Asynchronous HTTP Requests

What is this about?
This Package enables your Project to handle HTTP Requests in a trouly asynchronous way.

# async_http_requests

C Code.

# python_async_http_requests

Python Code.

# Preconditions

Some Tools are needed to build the Project.

    cmake
    make
    c-compiler
    python >= 3.10

Some Libraries are needed.
    
    libcurl
    openssl

# How to use it

Build the C Lib libahr.

    cd async_http_requests && mkdir build && cd build
    cmake -G "UnixMakefiles" ../
    make

Build the Pythonpackage pyahr.

    cd python_async_http_requests/

Use libahr and pyahr.
    
    pip install python_async_http_requests/dist/pyahr...tar.gz
    export LIB_AHR_PATH=/path/to/libahr.dylib

    # main.py
    from pyahr.async_http_requests import Processor, Request, Response, DefaultEventHandler
    from typing import Optional    

    h: DefaultEventHandler = DefaultEventHandler()
    p: Processor = Processor(url='www.google.de', event_handler=h)
    p.get(
        Request(ressource='')
    )

    ...

    response: Optional[Response] = h.next();
    print(
        response 
    )
