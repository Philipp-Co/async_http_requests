#ifndef __AHR_MUTEX_H__
#define __AHR_MUTEX_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

#include <stdint.h>
#include <stdbool.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

struct AHR_Mutex;
typedef struct AHR_Mutex* AHR_Mutex_t;

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Mutex_t AHR_CreateMutex(void);
void AHR_DestroyMutex(AHR_Mutex_t *mutex);

bool AHR_MutexTryLock(AHR_Mutex_t mutex);
void AHR_MutexLock(AHR_Mutex_t mutex);
void AHR_MutexUnlock(AHR_Mutex_t mutex);

//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
