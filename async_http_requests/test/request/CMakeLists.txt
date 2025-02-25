
add_executable(
    test_request 
    ${CMAKE_CURRENT_SOURCE_DIR}/test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_request.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_response.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_makerequest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/src/stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/src/async_http_requests.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/src/request_processor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/CMock/src/cmock.c

    ${CMAKE_BINARY_DIR}/mocks/mock_logging.c
    ${CMAKE_BINARY_DIR}/mocks/mock_ahr_curl.c
)

set_source_files_properties(
    ${CMAKE_BINARY_DIR}/mocks/mock_logging.c
    PROPERTIES GENERATED TRUE
)

target_include_directories(
    test_request
    PUBLIC
    #${CURL_INCLUDE_DIR}/curl/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/inc/
    #${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/inc/async_http_requests/
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/Unity/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/CMock/src/

    ${CMAKE_BINARY_DIR}/mocks/
)

target_link_directories(
    test_request
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build/
)

target_link_libraries(
    test_request
    PUBLIC
    unity
)

#add_custom_command(
#    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/mocks/logging.h ${CMAKE_CURRENT_SOURCE_DIR}/mocks/mock_logging.c ${CMAKE_CURRENT_SOURCE_DIR}/mock_logging.h
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/inc/async_http_requests/logging.h ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/src/logging.h
#    COMMAND ruby ${CMAKE_CURRENT_SOURCE_DIR}/../../external/CMock/lib/cmock.rb -o${CMAKE_CURRENT_SOURCE_DIR}/config.yml ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/inc/async_http_requests/logging.h
#)

add_custom_target(
    generate 
    COMMAND 
    ruby ${CMAKE_CURRENT_SOURCE_DIR}/../../external/CMock/lib/cmock.rb 
        -o${CMAKE_CURRENT_SOURCE_DIR}/config.yml 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/inc/async_http_requests/logging.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../async_http_requests/src/external/inc/external/async_http_requests/ahr_curl.h 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
)
