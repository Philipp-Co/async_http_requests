# async_http_requests - lilbahr

This Directory contains the C-Source Code for libahr.

libahr is a shared Library which handles HTTP Requests asynchronously for you.
It uses libcurl, https://curl.se/libcurl/, for HTTP Processing.
What libahr offers is a easy Interface to interact with the Curl Multi Interface.

libahr implements a HttpProcessor which is the main Object you will interact with.
The HttpProcessor is will start its own Thread in the Background which will do all the neccessary work.
The public Interface of the HttpProcessor itselve is Threadsafe.
    
    
    #include <async_http_requests/request_processor.h>

    HttpProcessor_t processor = AHR_CreateHttpProcessor();
    AHR_ProcessorStart(processor);
    ...
    //
    // Do your Thing.
    //
    ...
    //
    // Select a Request Object for the Request.
    //
    size_t request_object = 0;
    //
    // Prepare Request Specific Data.
    //
    AHR_RequestData_t request_data;
    AHR_UserData_t user_data;
    //
    // Configure a POST Request.
    //
    AHR_Post(
        processor,
        request_object,
        &request_data,
        &user_data
    );
    AHR_ProcessorMakeRequest(processor, request_object);
    //
    // Wait for the Response. libahr will call on of 
    // either the Successcallback or the Errorcallback.
    //
    ...
    AHR_DestroyHttpProcessor(&processor);

# Build Library

How to build libahr.

## Static Code Analysis

flawfinder:

    python -m .venv && source ./venv/bin/activate
    pip install -r requirements.txt
    ./flawfinder.sh

cppcheck:
Before using the cppcheck Scripts, install cppcheck, https://cppcheck.sourceforge.io/.

    ./cppcheck.sh

## Requirements

    Tools needed:
        - C-Compiler (f.e. GCC or clang)
        - CMake
        - Make

    Libraries which have to be present on the Build-System:
        - lubcurl - https://cmake.org/cmake/help/latest/module/FindCURL.html

## Build

This Library is developed with 
    
    Apple clang version 17.0.0 (clang-1700.0.13.3)
    Target: arm64-apple-darwin24.4.0
    Thread model: posix

Create a Build-Directory enter it, run CMake and build.

    mkdir build/ && cd build/
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ../  # -DCMAKE_BUILD_TYPE=Release for a Release-Build
    make ahr

This should generate the shared Library libahr.so in your Build-Directory.
The Fileextension may differ depending on the OS you build it on, on MacOS it may be libahr.dylib.

# Setup Library

    # in your CMakeLists.txt
    library_directories(path/to/build/)
    target_link_directories(ahr)

    # in your Source
    #include <async_http_requests/http_processor.h>
    ...

# Test Library
    
## Unittest

    mkdir build && cd build
    cmake -G "Unix Makefiles" ../
    make generate && make generate_processor
    make 

