
#include <string.h>
#include <test_request.h>
#include <async_http_requests/async_http_requests.h>
#include "../../build/mocks/mock_ahr_curl.h"
#include "unity.h"
#include "unity_internals.h"

void test_AHR_CreateRequest(void)
{
    AHR_CurlEasyInit_IgnoreAndReturn((void*)42);
    AHR_CurlEasyCleanUp_Expect(
        (void*)42
    );

    AHR_HttpRequest_t request = AHR_CreateRequest();
    TEST_ASSERT_NOT_NULL(request);

    AHR_Curl_t handle = AHR_RequestHandle(request);
    TEST_ASSERT_EQUAL_PTR(((void*)42), handle);

    AHR_DestroyRequest(&request);
    TEST_ASSERT_NULL(request);
}

void test_AHR_RequestSetHeader(void)
{
    // Set up Mocks.
    AHR_CurlEasyInit_IgnoreAndReturn((void*)42);
    AHR_CurlEasyCleanUp_Ignore();

    // Start Test...
    AHR_HttpRequest_t request = AHR_CreateRequest();

    AHR_Header_t header;
    memset(&header, '\0', sizeof(header));
    header.nheaders = 2;
    strcat(header.header[0].name, "First");
    strcat(header.header[0].value, "v0");
    strcat(header.header[1].name, "Second");
    strcat(header.header[1].value, "v1");
    strcat(header.header[2].name, "Third");
    strcat(header.header[2].value, "v2");

    AHR_RequestSetHeader(request, &header);

    // Assert Result.
    AHR_Header_t return_value;
    AHR_RequestGetHeader(request, &return_value);
    
    TEST_ASSERT_EQUAL_INT(return_value.nheaders, 2);
    TEST_ASSERT_EQUAL_STRING(
        "First", header.header[0].name
    );
    TEST_ASSERT_EQUAL_STRING(
        "v0", header.header[0].value
    );
    TEST_ASSERT_EQUAL_STRING(
        "Second", header.header[1].name
    );
    TEST_ASSERT_EQUAL_STRING(
        "v1", header.header[1].value
    );
    
    // Clean Up.
    AHR_DestroyRequest(&request);
}
