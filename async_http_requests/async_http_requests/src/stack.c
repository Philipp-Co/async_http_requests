
//
// --------------------------------------------------------------------------------------------------------------------
//

#include <assert.h>

#include <async_http_requests/stack.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Stack_t AHR_CraeteStack(size_t max_size)
{
    AHR_Stack_t stack = {.data=NULL, .max_size=max_size, .top=0};
    stack.data = malloc(sizeof(void*) * max_size);
    return stack;
}

void AHR_DestroyStack(AHR_Stack_t *stack)
{
    free(stack->data);
    stack->data = NULL;
    stack->max_size = 0;
}

void* AHR_StackPop(AHR_Stack_t *stack)
{
    assert(NULL != stack->data);
    if(stack->top <= 0)
    {
        return NULL;
    }
    return stack->data[--stack->top];
}

bool AHR_StackPush(AHR_Stack_t *stack, void *arg)
{
    assert(NULL != stack->data);
    if(stack->top < stack->max_size)
    {
        stack->data[stack->top++] = arg;
        return true;
    }
    return false;
}

//
// --------------------------------------------------------------------------------------------------------------------
//
