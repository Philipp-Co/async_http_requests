#ifndef __AHR_TEST_REQUEST_H__
#define __AHR_TEST_REQUEST_H__

#include <unity.h>

///
/// \brief  Create a Requet Object und destroy it.
/// 
/// \expect The Curl Handle is initialized and destroyed with the
///         Request Object. 
///
void test_AHR_CreateRequest(void);

void test_AHR_RequestSetHeader(void);

#endif
