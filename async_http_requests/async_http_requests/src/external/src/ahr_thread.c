//
// --------------------------------------------------------------------------------------------------------------------
//

#include <external/async_http_requests/ahr_thread.h>

#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

struct AHR_Thread
{
    pthread_t thread_id; 
};

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Thread_t AHR_CreateThread(AHR_ThreadMain_t function, void *arg)
{
    AHR_Thread_t thread = (AHR_Thread_t)malloc(sizeof(struct AHR_Thread));
    pthread_create(&thread->thread_id, NULL, function, arg);
    return thread;
}

void AHR_DestroyThread(AHR_Thread_t *thread)
{
    (void)thread;
    if(*thread)
    {
        free(*thread);
        *thread = NULL;
    }
}

void AHR_JoinThread(AHR_Thread_t thread, void *result)
{
    pthread_join(thread->thread_id, &result);
}


//
// --------------------------------------------------------------------------------------------------------------------
//
