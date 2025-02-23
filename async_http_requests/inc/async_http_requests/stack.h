#ifndef __AHR_STACK_H__
#define __AHR_STACK_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

#include <stdlib.h>
#include<stdbool.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

typedef struct
{
    void **data;
    size_t max_size;
    size_t top;
} AHR_Stack_t;


AHR_Stack_t AHR_CraeteStack(size_t max_size);
void* AHR_StackPop(AHR_Stack_t *stack);
bool AHR_StackPush(AHR_Stack_t *stack, void *arg);

//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
