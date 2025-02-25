#include <test_processor.h>

#include <async_http_requests/request_processor.h>

#include "../../build/mocks/processor/mock_ahr_curl.h"
#include "../../build/mocks/processor/mock_async_http_requests.h"
#include "async_http_requests/async_http_requests.h"
#include <unity.h>
#include <string.h>


static void test_AHR_ProcessorSetUp(void);
static void test_AHR_ProcessorTearDown(void);


AHR_HttpRequest_t test_AHR_CreateRequest_CALLBACK(int cmock_num_calls)
{
    return (AHR_HttpRequest_t)(((char*)200) + cmock_num_calls);
}

AHR_HttpResponse_t test_AHR_CreateResponse_CALLBACK(int cmock_num_calls)
{
    return (AHR_HttpResponse_t)(((char*)1000) + cmock_num_calls);
}

void test_AHR_CreateProcessor(void)
{
    test_AHR_ProcessorSetUp();
    // ------------------------------------------------------------------------- 
    AHR_Processor_t processor = AHR_CreateProcessor(NULL);    
    AHR_DestroyProcessor(&processor);
    
    TEST_ASSERT_NULL(processor);
    // ------------------------------------------------------------------------- 
    test_AHR_ProcessorTearDown();
}

static bool CMOCK_AHR_CurlMultiPerform(AHR_CurlM_t handle, int* running_handles, int cmock_num_calls)
{
    *running_handles = 1;
    return true;
}
void test_AHR_ProcessorGet(void)
{
    AHR_UserData_t user_data;
    AHR_RequestData_t request_data;
    request_data.url = "www.google.de";
    test_AHR_ProcessorSetUp();
    AHR_RequestSetHeader_Ignore();
    AHR_Get_Expect(NULL, request_data.url, NULL);
    AHR_Get_IgnoreArg_request();
    AHR_Get_IgnoreArg_response();
    AHR_CurlMultiWeakUp_Ignore();
    AHR_RequestUUID_IgnoreAndReturn((void*)1);
    // ------------------------------------------------------------------------- 
    AHR_Processor_t processor = AHR_CreateProcessor(NULL);    
    AHR_Id_t id = AHR_ProcessorGet(
        processor,
        &request_data,
        user_data
    ); 
    TEST_ASSERT_NOT_NULL(id);

    AHR_DestroyProcessor(&processor);
   // ------------------------------------------------------------------------- 
    test_AHR_ProcessorTearDown();
}

void test_AHR_ProcessorPost(void)
{
    AHR_UserData_t user_data;
    AHR_RequestData_t request_data;
    request_data.url = "www.google.de";
    request_data.header.nheaders = 0;
    test_AHR_ProcessorSetUp();
    AHR_RequestSetHeader_Expect(NULL, NULL);
    AHR_RequestSetHeader_IgnoreArg_header();
    AHR_RequestSetHeader_IgnoreArg_request();
    AHR_Post_Expect(NULL, request_data.url, NULL, NULL);
    AHR_Post_IgnoreArg_request();
    AHR_Post_IgnoreArg_response();
    AHR_Post_IgnoreArg_body();
    AHR_CurlMultiWeakUp_Ignore();
    AHR_RequestUUID_IgnoreAndReturn((void*)1);
    // ------------------------------------------------------------------------- 
    AHR_Processor_t processor = AHR_CreateProcessor(NULL);    
    AHR_Id_t id = AHR_ProcessorPost(
        processor,
        &request_data,
        user_data
    ); 
    TEST_ASSERT_NOT_NULL(id);

    AHR_DestroyProcessor(&processor);
   // ------------------------------------------------------------------------- 
    test_AHR_ProcessorTearDown();
}

void test_AHR_ProcessorPut(void)
{
    AHR_UserData_t user_data;
    AHR_RequestData_t request_data;
    request_data.url = "www.google.de";
    request_data.header.nheaders = 0;
    test_AHR_ProcessorSetUp();
    AHR_RequestSetHeader_Expect(NULL, NULL);
    AHR_RequestSetHeader_IgnoreArg_request();
    AHR_RequestSetHeader_IgnoreArg_header();
    AHR_Put_Expect(NULL, request_data.url, NULL, NULL);
    AHR_Put_IgnoreArg_request();
    AHR_Put_IgnoreArg_response();
    AHR_Put_IgnoreArg_body();
    AHR_CurlMultiWeakUp_Ignore();
    AHR_RequestUUID_IgnoreAndReturn((void*)1);
    // ------------------------------------------------------------------------- 
    AHR_Processor_t processor = AHR_CreateProcessor(NULL);    
    AHR_Id_t id = AHR_ProcessorPut(
        processor,
        &request_data,
        user_data
    ); 
    TEST_ASSERT_NOT_NULL(id);

    AHR_DestroyProcessor(&processor);
   // ------------------------------------------------------------------------- 
    test_AHR_ProcessorTearDown();
}

void test_AHR_ProcessorDelete(void)
{
    AHR_UserData_t user_data;
    AHR_RequestData_t request_data;
    request_data.url = "www.google.de";
    test_AHR_ProcessorSetUp();
    AHR_RequestSetHeader_Ignore();
    AHR_Delete_Expect(NULL, request_data.url, NULL);
    AHR_Delete_IgnoreArg_request();
    AHR_Delete_IgnoreArg_response();
    AHR_CurlMultiWeakUp_Ignore();
    AHR_RequestUUID_IgnoreAndReturn((void*)1);
    // ------------------------------------------------------------------------- 
    AHR_Processor_t processor = AHR_CreateProcessor(NULL);    
    AHR_Id_t id = AHR_ProcessorDelete(
        processor,
        &request_data,
        user_data
    ); 
    TEST_ASSERT_NOT_NULL(id);

    AHR_DestroyProcessor(&processor);
   // ------------------------------------------------------------------------- 
    test_AHR_ProcessorTearDown();
}

static void test_AHR_ProcessorSetUp(void)
{
    AHR_CurlMultiInit_ExpectAndReturn((void*)42);
    AHR_CurlMultiCleanUp_Expect((void*)42);
    AHR_CreateRequest_StubWithCallback(test_AHR_CreateRequest_CALLBACK);
    AHR_CreateResponse_StubWithCallback(test_AHR_CreateResponse_CALLBACK);
    AHR_RequestSetLogger_Ignore();
    AHR_ResponseSetLogger_Ignore();
}

static void test_AHR_ProcessorTearDown(void)
{

}
