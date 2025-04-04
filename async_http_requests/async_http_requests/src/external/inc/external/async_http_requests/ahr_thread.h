#ifndef __AHR_THREAD_H__
#define __AHR_THREAD_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

struct AHR_Thread;
typedef struct AHR_Thread* AHR_Thread_t;

typedef void* (AHR_ThreadMain_t(void *arg));

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Thread_t AHR_CreateThread(AHR_ThreadMain_t function, void *arg);
void AHR_DestroyThread(AHR_Thread_t *thread);
void AHR_JoinThread(AHR_Thread_t thread, void *result);

//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
