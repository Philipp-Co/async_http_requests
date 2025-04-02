# async_http_requests

This Directory contains the C-Source Code for libahr.

# Build Library

How to build libahr.

## Requirements

    Tools needed:
        - C-Compiler
        - CMake
        - Make

    Libraries which have to be present on the Build-System:
        - lubcurl - https://cmake.org/cmake/help/latest/module/FindCURL.html

## Build

Create a build/-Directory enter it, run CMake and build.

    mkdir build/ && cd build/
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ../  # -DCMAKE_BUILD_TYPE=Release for a Release-Build
    make ahr

# Test
    
    mkdir build && cd build
    cmake -G "Unix Makefiles" ../
    make generate && make generate_processor
    make 

# How to use libahr

    # in your CMakeLists.txt
    library_directories(path/to/build/)
    target_link_directories(ahr)

    # in your Source
    #include <async_http_requests/http_processor.h>
    ...

