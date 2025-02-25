
#include <string.h>

#include <test_makerequest.h>

#include <async_http_requests/async_http_requests.h>
#include "../../build/mocks/mock_ahr_curl.h"
#include "async_http_requests/request_processor.h"


#include "unity.h"


void test_AHR_MakeRequest(void)
{
    AHR_CurlEasyInit_IgnoreAndReturn((void*)42);
    AHR_CurlEasyCleanUp_Expect((void*)42);
    AHR_CurlEasySetUrl_Ignore();
    AHR_CurlEasyPerform_ExpectAndReturn((void*)42, 0);
    AHR_CurlEasyStatusCode_ExpectAndReturn((void*)42, 200);

    AHR_HttpRequest_t request = AHR_CreateRequest();
    AHR_HttpResponse_t response = AHR_CreateResponse(); 

    AHR_Status_t status = AHR_MakeRequest(request, response);
    
    void* handle = AHR_RequestHandle(request);
    TEST_ASSERT_NOT_NULL(handle);

    long status_code = AHR_ResponseStatusCode(response);
    TEST_ASSERT_GREATER_THAN_INT32(0, status_code);

    const char *body = AHR_ResponseBody(response);
    TEST_ASSERT_NOT_NULL(body);
    TEST_ASSERT_EQUAL_INT32(0, strlen(body));

    AHR_DestroyResponse(&response);
    AHR_DestroyRequest(&request);
}

void test_AHR_Get(void)
{
    AHR_CurlEasyInit_IgnoreAndReturn((void*)42);
    AHR_CurlEasyCleanUp_Expect((void*)42);
    AHR_CurlEasySetUrl_Ignore();
    AHR_CurlEasyPerform_ExpectAndReturn((void*)42, 0);
    AHR_CurlEasyStatusCode_ExpectAndReturn((void*)42, 200);
    AHR_CurlSetHttpMethodGet_Expect((void*)42);
    AHR_CurlSetHeader_Expect((void*)42, NULL);
    AHR_CurlSetHeader_IgnoreArg_header();
    AHR_CurlSetCallbackUserData_Ignore();
    
    AHR_HttpRequest_t request = AHR_CreateRequest();
    AHR_HttpResponse_t response = AHR_CreateResponse(); 

    AHR_Get(request, "www.google.de", response);
    AHR_Header_t header;
    AHR_RequestGetHeader(request, &header);
    TEST_ASSERT_EQUAL_INT32(0, header.nheaders);

    AHR_DestroyResponse(&response);
    AHR_DestroyRequest(&request);
}

void test_AHR_Post(void)
{
    AHR_CurlEasyInit_IgnoreAndReturn((void*)42);
    AHR_CurlEasyCleanUp_Expect((void*)42);
    AHR_CurlEasySetUrl_Ignore();
    AHR_CurlEasyPerform_ExpectAndReturn((void*)42, 0);
    AHR_CurlEasyStatusCode_ExpectAndReturn((void*)42, 200);
    AHR_CurlSetHttpMethodPost_Expect((void*)42, NULL);
    AHR_CurlSetHttpMethodPost_IgnoreArg_body();
    AHR_CurlSetHeader_Expect((void*)42, NULL);
    AHR_CurlSetHeader_IgnoreArg_header();
    AHR_CurlSetCallbackUserData_Ignore();
   
    AHR_HttpRequest_t request = AHR_CreateRequest();
    AHR_HttpResponse_t response = AHR_CreateResponse(); 
    
    AHR_Header_t header;
    memset(&header, '\0', sizeof(header));
    for(size_t i=0;i<AHR_HEADER_NMAX;++i)
    {
        strcat(header.header[i].name, "empty");
        strcat(header.header[i].value, "empty");
    }
    header.nheaders = AHR_HEADER_NMAX;
    AHR_RequestSetHeader(request, &header);
    AHR_Post(request, "www.google.de", "", response);
    AHR_RequestGetHeader(request, &header);
    TEST_ASSERT_EQUAL_INT32(AHR_HEADER_NMAX, header.nheaders);
    TEST_ASSERT_EQUAL_STRING("Content-Type", header.header[AHR_HEADER_NMAX-1].name);
    TEST_ASSERT_EQUAL_STRING("application/json", header.header[AHR_HEADER_NMAX-1].value);
    
    AHR_DestroyResponse(&response);
    AHR_DestroyRequest(&request);
}

void test_AHR_Put(void)
{
    AHR_CurlEasyInit_IgnoreAndReturn((void*)42);
    AHR_CurlEasyCleanUp_Expect((void*)42);
    AHR_CurlEasySetUrl_Ignore();
    AHR_CurlEasyPerform_ExpectAndReturn((void*)42, 0);
    AHR_CurlEasyStatusCode_ExpectAndReturn((void*)42, 200);
    AHR_CurlSetHttpMethodPut_Expect((void*)42, NULL);
    AHR_CurlSetHttpMethodPut_IgnoreArg_body();
    AHR_CurlSetHeader_Expect((void*)42, NULL);
    AHR_CurlSetHeader_IgnoreArg_header();
    AHR_CurlSetCallbackUserData_Ignore();
    
    AHR_HttpRequest_t request = AHR_CreateRequest();
    AHR_HttpResponse_t response = AHR_CreateResponse(); 

    AHR_Put(request, "www.google.de", "", response);
    
    AHR_DestroyResponse(&response);
    AHR_DestroyRequest(&request);
}

void test_AHR_Delete(void)
{
    AHR_CurlEasyInit_IgnoreAndReturn((void*)42);
    AHR_CurlEasyCleanUp_Expect((void*)42);
    AHR_CurlEasySetUrl_Ignore();
    AHR_CurlEasyPerform_ExpectAndReturn((void*)42, 0);
    AHR_CurlEasyStatusCode_ExpectAndReturn((void*)42, 200);
    AHR_CurlSetHeader_Expect((void*)42, NULL);
    AHR_CurlSetHeader_IgnoreArg_header();
    AHR_CurlSetHttpMethodDelete_Expect((void*)42);
    AHR_CurlSetCallbackUserData_Ignore();
    
    AHR_HttpRequest_t request = AHR_CreateRequest();
    AHR_HttpResponse_t response = AHR_CreateResponse(); 
    
    AHR_Delete(request, "www.google.de", response);

    AHR_DestroyResponse(&response);
    AHR_DestroyRequest(&request);
}

