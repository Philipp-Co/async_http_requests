#include <test_stack.h>

#include <async_http_requests/stack.h>

#include <unity.h>


void test_AHR_CreateStack(void)
{
    AHR_Stack_t stack = AHR_CraeteStack(8);
    TEST_ASSERT_EQUAL_INT32(8, stack.max_size);
    TEST_ASSERT_EQUAL_INT32(0, stack.top);

    AHR_DestroyStack(&stack);
    TEST_ASSERT_NULL(stack.data);
    TEST_ASSERT_EQUAL_INT32(0, stack.max_size);
    TEST_ASSERT_EQUAL_INT32(0, stack.top);
}


void test_AHR_StackPush1(void)
{
    size_t elements[5] = {
        0, 1, 2, 3, 4
    };
    AHR_Stack_t stack = AHR_CraeteStack(8);
    
    AHR_StackPush(&stack, elements);
    const void *top = AHR_StackPop(&stack);
    TEST_ASSERT_EQUAL_PTR(elements, top);
    TEST_ASSERT_EQUAL_INT32(0, stack.top);

    AHR_DestroyStack(&stack);
}

void test_AHR_StackPushN(void)
{
    size_t elements[5] = {
        0, 1, 2, 3, 4
    };
    AHR_Stack_t stack = AHR_CraeteStack(5);
    
    for(size_t i=0;i<5;++i)
    {
        AHR_StackPush(&stack, &elements[i]);

    }
    TEST_ASSERT_EQUAL_INT32(5, stack.top);

    AHR_DestroyStack(&stack);
}


void test_AHR_StackPop0(void)
{
    AHR_Stack_t stack = AHR_CraeteStack(5);
    
    TEST_ASSERT_NULL(
        AHR_StackPop(&stack)
    );

    AHR_DestroyStack(&stack);
}

void test_AHR_StackPopN(void)
{
    size_t elements[5] = {
        0, 1, 2, 3, 4
    };
    AHR_Stack_t stack = AHR_CraeteStack(5);
    
    for(size_t i=0;i<5;++i)
    {
        AHR_StackPush(&stack, &elements[i]);
    }
    
    for(size_t i=0;i<5;++i)
    {
        TEST_ASSERT_EQUAL_INT32(elements[4-i], *((size_t*)AHR_StackPop(&stack)));
    }
}
