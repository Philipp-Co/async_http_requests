
#include <unity.h>
#include "../../build/mocks/processor/mock_ahr_curl.h"
#include "../../build/mocks/processor/mock_logging.h"
#include "../../build/mocks/processor/mock_async_http_requests.h"

#include <test_processor.h>

void setUp(void) {
    //testValue = 40;
    mock_ahr_curl_Init();
    mock_logging_Init();
    mock_async_http_requests_Init();
}

void tearDown(void) {
    // clean stuff up here
    mock_async_http_requests_Destroy();
    mock_logging_Destroy();
    mock_ahr_curl_Destroy();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_AHR_CreateProcessor);
    RUN_TEST(test_AHR_ProcessorGet);
    RUN_TEST(test_AHR_ProcessorPost);
    RUN_TEST(test_AHR_ProcessorPut);
    RUN_TEST(test_AHR_ProcessorDelete);
    return UNITY_END();
}
