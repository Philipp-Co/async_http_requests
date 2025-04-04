
//
// --------------------------------------------------------------------------------------------------------------------
//

#include <external/async_http_requests/ahr_mutex.h>

#include <stdlib.h>
#include <assert.h>
#include <stdatomic.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

#define AHR_MUTEX_LOCKED 1
#define AHR_MUTEX_UNLOCKED 0

//
// --------------------------------------------------------------------------------------------------------------------
//

struct AHR_Mutex
{
    atomic_int lock;
};

AHR_Mutex_t AHR_CreateMutex(void)
{
    AHR_Mutex_t mutex = malloc(sizeof(struct AHR_Mutex));  
    atomic_store(&mutex->lock, AHR_MUTEX_UNLOCKED);
    return mutex;
}

void AHR_DestroyMutex(AHR_Mutex_t *mutex)
{
    assert(NULL != mutex);
    assert(NULL != *mutex);

    free(*mutex);
    *mutex = NULL;
}

bool AHR_MutexTryLock(AHR_Mutex_t mutex)
{
    int desired = AHR_MUTEX_UNLOCKED;
    return atomic_compare_exchange_strong(&mutex->lock, &desired, AHR_MUTEX_LOCKED);
}

void AHR_MutexLock(AHR_Mutex_t mutex)
{
    int desired = AHR_MUTEX_UNLOCKED;
    while(
        !atomic_compare_exchange_strong(&mutex->lock, &desired, AHR_MUTEX_LOCKED)
    );
}

void AHR_MutexUnlock(AHR_Mutex_t mutex)
{
    atomic_store(&mutex->lock, AHR_MUTEX_UNLOCKED);
}

//
// --------------------------------------------------------------------------------------------------------------------
//
