#include <string.h>

#include <test_response.h>
#include <async_http_requests/async_http_requests.h>

#include "../../build/mocks/mock_ahr_curl.h"

#include "unity.h"
#include "unity_internals.h"


void test_AHR_CreateResponse(void)
{
    AHR_HttpResponse_t response = AHR_CreateResponse();
    TEST_ASSERT_NOT_NULL(response);
    
    long status_code = AHR_ResponseStatusCode(response);
    TEST_ASSERT_LESS_THAN_INT32(0, status_code);

    AHR_Header_t header = {.nheaders = 5};
    AHR_ResponseHeader(response, &header);
    TEST_ASSERT_EQUAL_INT32(0, (int)header.nheaders);

    const char *body = AHR_ResponseBody(response);
    TEST_ASSERT_NOT_NULL(body);
    TEST_ASSERT_EQUAL_INT32(0, strlen(body));

    AHR_DestroyResponse(&response);
    TEST_ASSERT_NULL(response);
}

