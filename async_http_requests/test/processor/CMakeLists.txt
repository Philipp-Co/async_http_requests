
add_executable(
    test_processor 
    ${CMAKE_CURRENT_SOURCE_DIR}/test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_processor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/CMock/src/cmock.c

    ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/src/stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/src/request_processor.c
    
    ${CMAKE_BINARY_DIR}/mocks/processor/mock_logging.c
    ${CMAKE_BINARY_DIR}/mocks/processor/mock_ahr_curl.c
    ${CMAKE_BINARY_DIR}/mocks/processor/mock_async_http_requests.c
)

set_source_files_properties(
    ${CMAKE_BINARY_DIR}/mocks/processor/mock_logging.c
    ${CMAKE_BINARY_DIR}/mocks/processor/mock_ahr_curl.c
    ${CMAKE_BINARY_DIR}/mocks/processor/mock_async_http_requests.c
    PROPERTIES GENERATED TRUE
)

target_include_directories(
    test_processor
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/Unity/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/CMock/src/

    ${CMAKE_BINARY_DIR}/mocks/processor/
)

target_link_directories(
    test_processor
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build/
)

target_link_libraries(
    test_processor
    PUBLIC
    unity
)

add_custom_target(
    generate_processor_mocks 
    COMMAND 
    ruby ${CMAKE_CURRENT_SOURCE_DIR}/../../external/CMock/lib/cmock.rb 
        -o${CMAKE_CURRENT_SOURCE_DIR}/config.yml 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/inc/async_http_requests/logging.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/src/external/inc/external/async_http_requests/ahr_curl.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/inc/async_http_requests/async_http_requests.h 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
)
