#include <unity.h>
#include <test_request.h>
#include <test_response.h>
#include <test_makerequest.h>
#include <test_stack.h>

#include "../../build/mocks/mock_ahr_curl.h"

void setUp(void) {
    //testValue = 40;
    mock_ahr_curl_Init();
}

void tearDown(void) {
    // clean stuff up here
    mock_ahr_curl_Destroy();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_AHR_CreateStack);
    RUN_TEST(test_AHR_StackPush1);
    RUN_TEST(test_AHR_StackPushN);
    RUN_TEST(test_AHR_StackPop0);
    RUN_TEST(test_AHR_StackPopN);
    RUN_TEST(test_AHR_CreateRequest);
    RUN_TEST(test_AHR_RequestSetHeader);
    RUN_TEST(test_AHR_CreateResponse);
    RUN_TEST(test_AHR_MakeRequest);
    RUN_TEST(test_AHR_Get);
    RUN_TEST(test_AHR_Post);
    RUN_TEST(test_AHR_Put);
    RUN_TEST(test_AHR_Delete);
    return UNITY_END();
}
