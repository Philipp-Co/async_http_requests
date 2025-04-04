#
# ---------------------------------------------------------------------------------------------------------------------
#

cmake_minimum_required(VERSION 3.25)
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
    async_http_requests/src/private/src/ahr_async_http_requests.c
    async_http_requests/src/ahr_http_request_processor.c
    async_http_requests/src/private/src/ahr_stack.c
    async_http_requests/src/private/src/ahr_logging.c
    async_http_requests/src/external/src/ahr_curl.c
    async_http_requests/src/private/src/ahr_result.c
    async_http_requests/src/external/src/ahr_mutex.c
    async_http_requests/src/external/src/ahr_thread.c
)

#
# Add test.
#
#add_subdirectory(
#    ${CMAKE_CURRENT_SOURCE_DIR}/test/request/
#)
#add_subdirectory(
#    ${CMAKE_CURRENT_SOURCE_DIR}/test/processor/
#)

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
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(
        ahr
        PUBLIC
        "-fsanitize=address"
        "-fno-omit-frame-pointer"
        "-Wno-unused-function"
        "-fPIC"
        "-Wall"
        "-Wextra"
        "-Werror"
        "-Wincompatible-pointer-types"
        "-pedantic"
        "-pedantic-errors"
    )
    target_link_options(
        ahr 
        PUBLIC
        "-fsanitize=address"
        "-fno-omit-frame-pointer"
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(
        ahr
        PUBLIC
        "-O2"
        "-Wno-unused-function"
        "-fPIC"
        "-Wall"
        "-Wextra"
        "-Werror"
        "-Wincompatible-pointer-types"
        "-pedantic"
        "-pedantic-errors"
    )
endif()

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
