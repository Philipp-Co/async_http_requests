cmake_minimum_required(VERSION 3.11)
project(async_http_requests VERSION 0.0.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(
    ahr SHARED
    async_http_requests/src/async_http_requests.c
    async_http_requests/src/request_processor.c
    async_http_requests/src/stack.c
)

target_compile_options(
    ahr
    PUBLIC
    "-O2"
)

add_executable(
    test
    test/src/main.c
)

target_include_directories(
    ahr
    PUBLIC
    async_http_requests/inc/
    external/curl/include/
)

target_link_directories(
    ahr
    PUBLIC
    build/
)

target_link_libraries(
    ahr
    PUBLIC
    curl    
)

target_include_directories(
    test
    PUBLIC
    async_http_requests/inc/
)

target_link_directories(
    test
    PUBLIC
    build/
)

target_link_libraries(
    test
    ahr
)
