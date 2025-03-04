#
# ---------------------------------------------------------------------------------------------------------------------
#

cmake_minimum_required(VERSION 3.31)
project(async_http_requests VERSION 0.0.0 LANGUAGES C)

#
# ---------------------------------------------------------------------------------------------------------------------
#

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
# Find required Packages.
#
find_package(CURL REQUIRED)

#
# ---------------------------------------------------------------------------------------------------------------------
#

#
# add unity
#
add_library(
    unity STATIC
    external/Unity/src/unity.c
)

#
# Add libahr.
#
add_library(
    ahr SHARED
    async_http_requests/src/async_http_requests.c
    async_http_requests/src/request_processor.c
    async_http_requests/src/stack.c
    async_http_requests/src/logging.c
    async_http_requests/src/external/src/ahr_curl.c
    async_http_requests/src/private/src/result.c
)

#
# Add test.
#
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/test/request/
)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/test/processor/
)

#
# ---------------------------------------------------------------------------------------------------------------------
#

#
# configure unity.
#
target_include_directories(
    unity
    PUBLIC
    external/Unity/src/
)

#
# Configure libahr.
#
target_compile_options(
    ahr
    PUBLIC
    "-O2"
    "-fPIC"
)

target_include_directories(
    ahr
    PUBLIC
    async_http_requests/inc/
    async_http_requests/src/external/inc/
    async_http_requests/src/private/inc/
    external/curl/include/
)

target_link_libraries(
    ahr
    PUBLIC
    CURL::libcurl
)

#
# ---------------------------------------------------------------------------------------------------------------------
#
