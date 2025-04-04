///
/// \brief  This Module implements a Stack.
///
#ifndef __AHR_STACK_H__
#define __AHR_STACK_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

#include <stdlib.h>
#include <stdbool.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

typedef struct
{
    void **data;
    size_t max_size;
    size_t top;
} AHR_Stack_t;

//
// --------------------------------------------------------------------------------------------------------------------
//
///
/// \brief  Create a Stack object.
/// \param[in] max_size - The maximum size in number of Elements the created Stack can contain at any time.
///
AHR_Stack_t AHR_CraeteStack(size_t max_size);
///
/// \brief  Pop the top Element from the Stack.
/// \returns    A valid Pointer if there was an Element at the Top.
///             NULL if the Stack was empty.
///
void* AHR_StackPop(AHR_Stack_t *stack);
///
/// \brief  Push a new Element to the Top of the Stack.
/// \returns    true on success, that is when the size of the Stack is smaller then "max_size".
///             false otherwise.
///
bool AHR_StackPush(AHR_Stack_t *stack, void *arg);
///
/// \brief  Destroy the Stack.
///
void AHR_DestroyStack(AHR_Stack_t *stack);
//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
