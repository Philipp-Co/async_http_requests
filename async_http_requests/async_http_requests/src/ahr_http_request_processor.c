///
/// \brief  Http Processor.
/// \architectural decisions    Here are some Decisions listed.
///     1. No public Access to internal Structures.
///     2. The Http Processor does not use dynamic Memory Allocation after Initialization. 
///     3. No Infologs for Debugging, Log all Errors
///         

//
// --------------------------------------------------------------------------------------------------------------------
//
#include <async_http_requests/ahr_http_request_processor.h>
#include <async_http_requests/private/ahr_async_http_requests.h>
#include <async_http_requests/private/ahr_stack.h>
#include "external/inc/external/async_http_requests/ahr_curl.h"
#include <async_http_requests/private/ahr_result.h>
#include <external/async_http_requests/ahr_mutex.h>
#include <external/async_http_requests/ahr_thread.h>

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdatomic.h>
#include <errno.h>
#include <string.h>

#include <curl/curl.h>
//
// --------------------------------------------------------------------------------------------------------------------
//

///
/// \brief  This Structure holds the state of a Transaktion with the Server.
///
struct AHR_RequestListNode;
///
/// \brief  Implementation of a simple List.
///
struct AHR_RequestListNode
{
    struct AHR_RequestListNode *next;
    AHR_Result_t *result;
};
///
/// \brief Implementation of a simple List.
///
typedef struct 
{
    struct AHR_RequestListNode *head;
} AHR_RequestList;

///
/// \brief  Key Structure. This holds the state of the modules.
///         
///
struct AHR_Processor
{
    AHR_Logger_t logger;
    ///
    /// \brief  Thread Id.
    /// 
    AHR_Thread_t thread;
    ///
    /// \brief Decide when to terminate the internal Thread. 0 = run, 1 = terminate.
    ///
    atomic_int terminate;
    ///
    /// \brief  The curl multi Handle.
    ///
    AHR_CurlM_t handle;
    ///
    /// \brief  Generic Mutex to guard access to Datastructures.
    ///
    AHR_Mutex_t mutex;

    ///
    /// \brief Object which are requested to execute. 
    ///
    AHR_Stack_t requests;
    ///
    /// \brief Finished Requests, waiting to be put back to the unused_results. 
    ///
    AHR_RequestList result_list;
    
    AHR_ResultStore_t result_store;
};

//
// --------------------------------------------------------------------------------------------------------------------
//

static bool AHR_ProcessorTryLockResult(AHR_Result_t *result);
static void AHR_ProcessorUnlockResult(AHR_Result_t *result);
static bool AHR_ProcessorIsResultBusy(AHR_Result_t *result);
///
/// \brief  Add one Element to the Request-List.
///
static bool AHR_RequestListAdd(AHR_RequestList *list, AHR_Result_t *result);
///
/// \brief  Remove one Element from the Request-List.
///
static bool AHR_RequestListRemove(AHR_RequestList *list, void *handle);
///
/// ≤brief  Find an Element in the Request-List.
///
static AHR_Result_t* AHR_RequestListFind(AHR_RequestList *list, void *handle); // cppcheck-suppress constParameterPointer
///
/// \brief  Handle new incomin Requests.
///         Read and remove all Elements from the Request-List and add them to the active Requests list. 
///
static void AHR_HandleNewRequests(AHR_Processor_t processor);
///
/// \brief  Process the Eventloop and wait for other incoming events.
///
static void AHR_ExecuteAndPoll(AHR_Processor_t processor);
///
/// \brief  This is the internal Function which executes the Eventloop inside a Thread.
///
static void* AHR_ProcessorThreadFunc(void *arg);

static AHR_ProcessorStatus_t AHR_ProcessorPrepareRequest(
    AHR_Processor_t processor,
    size_t object, 
    const AHR_RequestData_t *request_data,
    AHR_UserData_t data
);
//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Processor_t AHR_CreateProcessor(size_t max_objects, AHR_Logger_t logger)
{
    //
    // Create a AHR_Processor_t handle.
    //
    if(max_objects > 25)
    {
        return NULL;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    AHR_Processor_t processor = malloc(sizeof(struct AHR_Processor));
    //
    // If the Object was not allocated, return immediately...
    //
    if(!processor)
    {
        return NULL;
    }
    processor->handle = AHR_CurlMultiInit(); 
    //
    // If the Curl Handle was not allocated, there is no point in going on...
    //
    if(!processor->handle)
    {
        goto on_error;
    }
    
    // TODO: 
    //  - Malloc only on init -> pre-create all Resultobjects
    //  - Make url reconfigurable in request objects
    //  - Make Callback reconfigurable in response objects
    processor->result_store = AHR_CreateResultStore(max_objects);
    // 
    // TODO: Check if the Result Store is valid...
    //
    for(size_t i = 0;i<AHR_ResultStoreSize(&processor->result_store); ++i)
    {
        AHR_Result_t *result = AHR_ResultStoreGetResult(&processor->result_store, i);
        result->request_data.body = malloc(AHR_PROCESSOR_MAX_BODY_SIZE);
        result->request_data.url = malloc(AHR_PROCESSOR_MAX_URL_LEN);
        memset(&result->request_data.header, '\0', sizeof(AHR_Header_t));
        result->request = AHR_CreateRequest();
        result->response = AHR_CreateResponse();
    }
    //
    processor->requests = AHR_CraeteStack(max_objects);
    processor->result_list.head = NULL;
    
    processor->mutex = AHR_CreateMutex();
    atomic_store(&(processor->terminate), 0);

    processor->logger = logger;
    processor->thread = (AHR_Thread_t)NULL;

    return processor;

    //
    // On Error all Ressources shall be freed.
    //
    on_error:
    AHR_DestroyProcessor(&processor);
    return NULL;
}

void AHR_DestroyProcessor(AHR_Processor_t *processor)
{
    if(!*processor) return;

    AHR_ProcessorStop(*processor);
    AHR_DestroyResultStore(&(*processor)->result_store);
    AHR_CurlMultiCleanUp((*processor)->handle);
    
    free(*processor);
    *processor = NULL;
}

bool AHR_ProcessorStart(AHR_Processor_t processor)
{
    // ----
    if((AHR_Thread_t)NULL != processor->thread)
    {
        AHR_LogWarning(
            processor->logger, 
            "Uable to start Processors Thread because it was alreay started...\n"
        );
        return true;
    }
    // ----
    atomic_store(&processor->terminate, 0);
    processor->thread = AHR_CreateThread(AHR_ProcessorThreadFunc, processor);
    if(NULL != processor->thread)
    {
        return true;
    }
    // ----
    AHR_LogError(processor->logger, "Thread Error!\n");
    return false;
}

void AHR_ProcessorStop(AHR_Processor_t processor)
{
    if(processor->thread)
    {
        atomic_store(&(processor->terminate), 1);
        void *result;
        AHR_JoinThread(processor->thread, &result);
        AHR_DestroyThread(&processor->thread);
    }
}

size_t AHR_ProcessorNumberOfRequestObjects(const AHR_Processor_t processor)
{
    return AHR_ResultStoreSize(&processor->result_store);
}

AHR_Id_t AHR_ProcessorTransactionId(AHR_Processor_t processor, size_t object)
{
    return AHR_RequestUUID(AHR_ResultStoreGetResult(&processor->result_store, object)->request);
}

AHR_ProcessorStatus_t AHR_ProcessorPrepareRequest(
    AHR_Processor_t processor,
    size_t object,
    const AHR_RequestData_t *request_data,
    AHR_UserData_t data
)
{
    //
    // All buffers are constructed and assigned accourding to the defined Macros.
    // Bufferchecks for size are not neccessarry if the initial Macro Values are respected.
    //

    //
    // Assertions...
    //
    assert(NULL != request_data);
    assert(NULL != request_data->url);

    AHR_Result_t *result = AHR_ResultStoreGetResult(
        &processor->result_store,
        object
    );
    // ---- 
    // ---- 
    //
    // Processing...
    //
    AHR_RequestSetHeader(
        result->request,
        &request_data->header
    );
    if(request_data->body)
    {
        memset(result->request_data.body, '\0', AHR_PROCESSOR_MAX_BODY_SIZE+1);
        memcpy( // flawfinder: ignore
            result->request_data.body, 
            request_data->body, 
            strnlen(
                request_data->body, 
                AHR_PROCESSOR_MAX_BODY_SIZE
            )
        );
    }
    memset(result->request_data.url, '\0', AHR_PROCESSOR_MAX_URL_LEN+1);
    memcpy(result->request_data.url, request_data->url, AHR_PROCESSOR_MAX_URL_LEN); // flawfinder: ignore

    result->user_data = data;
    return AHR_PROC_OK; 
}

AHR_ProcessorStatus_t AHR_ProcessorMakeRequest(AHR_Processor_t processor, size_t object)
{
    AHR_ProcessorStatus_t retval = AHR_PROC_OK;
    AHR_MutexLock(processor->mutex);
    AHR_Result_t *result = AHR_ResultStoreGetResult(
        &processor->result_store,
        object
    );
    //
    // Error Handling...
    //
    // ---- 
    if(!result)
    {
        AHR_LogWarning(processor->logger, "Warning: Unable to retrieve unused element.");
        retval = AHR_PROC_UNKNOWN_OBJECT;
        goto end;
    }
    // ---- 
    if(AHR_ProcessorIsResultBusy(result))
    {
        retval = AHR_PROC_OBJECT_BUSY;
        goto end;
    }
    // ---- 
    //
    // Process...
    //
    AHR_ResponseReset(result->response);
    AHR_StackPush(&processor->requests, result);
    AHR_CurlMultiWeakUp(processor->handle);

end:
    AHR_MutexUnlock(processor->mutex);
    return retval;
}

AHR_ProcessorStatus_t AHR_ProcessorGet(
    AHR_Processor_t processor, 
    size_t object,
    const AHR_RequestData_t *request_data,
    AHR_UserData_t data
)
{
    AHR_ProcessorStatus_t status = AHR_PROC_OK;
    AHR_MutexLock(processor->mutex);

    AHR_Result_t *result = AHR_ResultStoreGetResult(
        &processor->result_store, object
    );
    //
    // Error Handling...
    //
    // ---- 
    if(!result)
    {
        AHR_LogWarning(
            processor->logger,
            "Warning: Unable to retrieve unused element."
        );
        status = AHR_PROC_UNKNOWN_OBJECT;
        goto end;
    }
    // ---- 
    if(!AHR_ProcessorTryLockResult(result))
    {
        AHR_LogInfo(
            processor->logger,
            "Sorry the Object you are requesting is busy."
        );
        status = AHR_PROC_OBJECT_BUSY;
        goto end;
    }
    // ---- 
    //
    // Process...
    //
    AHR_ProcessorPrepareRequest(
        processor,
        object,
        request_data,
        data
    );
    AHR_Get(result->request, result->request_data.url, result->response);
    AHR_ProcessorUnlockResult(result);
end:
    AHR_MutexUnlock(processor->mutex);
    return status;
}

AHR_ProcessorStatus_t AHR_ProcessorPost(
    AHR_Processor_t processor, 
    size_t object,
    const AHR_RequestData_t *data, 
    AHR_UserData_t user_data
)
{
    AHR_ProcessorStatus_t status = AHR_PROC_OK;
    AHR_MutexLock(processor->mutex);

    AHR_Result_t *result = AHR_ResultStoreGetResult(
        &processor->result_store, object
    );
    //
    // Error Handling...
    //
    // ---- 
    if(!result)
    {
        AHR_LogWarning(
            processor->logger,
            "Warning: Unable to retrieve unused element."
        );
        status = AHR_PROC_UNKNOWN_OBJECT;
        goto end;
    }
    // ---- 
    if(!AHR_ProcessorTryLockResult(result))
    {
        AHR_LogInfo(
            processor->logger,
            "Sorry the Object you are requesting is busy."
        );
        status = AHR_PROC_OBJECT_BUSY;
        goto end;
    }
    // ---- 
    //
    // Process...
    //
    AHR_ProcessorPrepareRequest(
        processor,
        object,
        data,
        user_data
    );
    AHR_Post(result->request, result->request_data.url, result->request_data.body, result->response);
    AHR_ProcessorUnlockResult(result);
end:
    AHR_MutexUnlock(processor->mutex);
    return status;
}

AHR_ProcessorStatus_t AHR_ProcessorPut(
    AHR_Processor_t processor, 
    size_t object,
    const AHR_RequestData_t *data, 
    AHR_UserData_t user_data
)
{
    AHR_ProcessorStatus_t status = AHR_PROC_OK;
    AHR_MutexLock(processor->mutex);

    AHR_Result_t *result = AHR_ResultStoreGetResult(
        &processor->result_store, object
    );
    //
    // Error Handling...
    //
    // ---- 
    if(!result)
    {
        AHR_LogWarning(
            processor->logger,
            "Warning: Unable to retrieve unused element."
        );
        status = AHR_PROC_UNKNOWN_OBJECT;
        goto end;
    }
    // ---- 
    if(!AHR_ProcessorTryLockResult(result))
    {
        AHR_LogInfo(
            processor->logger,
            "Sorry the Object you are requesting is busy."
        );
        status = AHR_PROC_OBJECT_BUSY;
        goto end;
    }
    // ---- 
    //
    // Process...
    //
    AHR_ProcessorPrepareRequest(
        processor,
        object,
        data,
        user_data
    );
    AHR_Put(result->request, result->request_data.url, result->request_data.body, result->response);
    AHR_ProcessorUnlockResult(result);
end:
    AHR_MutexUnlock(processor->mutex);
    return status;
}

AHR_ProcessorStatus_t AHR_ProcessorDelete(
    AHR_Processor_t processor, 
    size_t object,
    const AHR_RequestData_t *data, 
    AHR_UserData_t user_data
)
{
    AHR_ProcessorStatus_t status = AHR_PROC_OK;
    AHR_MutexLock(processor->mutex);

    AHR_Result_t *result = AHR_ResultStoreGetResult(
        &processor->result_store, object
    );
    //
    // Error Handling...
    //
    // ---- 
    if(!result)
    {
        AHR_LogWarning(
            processor->logger,
            "Warning: Unable to retrieve unused element."
        );
        status = AHR_PROC_UNKNOWN_OBJECT;
        goto end;
    }
    // ---- 
    if(!AHR_ProcessorTryLockResult(result))
    {
        AHR_LogInfo(
            processor->logger,
            "Sorry the Object you are requesting is busy."
        );
        status = AHR_PROC_OBJECT_BUSY;
        goto end;
    }
    // ---- 
    //
    // Process...
    //
    AHR_ProcessorPrepareRequest(
        processor,
        object,
        data,
        user_data
    );
    AHR_Delete(result->request, result->request_data.url, result->response);
    AHR_ProcessorUnlockResult(result);
end:
    AHR_MutexUnlock(processor->mutex);
    return status;
}

//
// --------------------------------------------------------------------------------------------------------------------
//

static void* AHR_ProcessorThreadFunc(void *arg)
{
    if(!arg) return NULL;
    AHR_Processor_t processor = (AHR_Processor_t)arg;
    do
    {
        AHR_HandleNewRequests(processor);
        AHR_ExecuteAndPoll(processor);
        
        AHR_CurlMultiPoll(processor->handle);
    }
    while(0 == atomic_load(&(processor->terminate)));
    return NULL;
}

static bool AHR_RequestListAdd(AHR_RequestList *list, AHR_Result_t *result)
{
    if(list->head)
    {
        struct AHR_RequestListNode *current = list->head;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = malloc(sizeof(struct AHR_RequestListNode));
        if(current->next)
        {
            current->next->next = NULL;
            current->next->result = result;
            return true;
        }
    }
    else
    {
        list->head = malloc(sizeof(struct AHR_RequestListNode));
        if(list->head)
        {
            list->head->next = NULL;
            list->head->result = result;
            return true;
        }
    }
    return false;
} 

static bool AHR_RequestListRemove(AHR_RequestList *list, void *handle) // cppcheck-suppress constParameterPointer
{
    //
    // The Argument "handle" is not made const because someone can retrive it later on 
    // via another call to one of teh List Functions and do work with it.
    //
    if(NULL == list->head)
    {
        return false;
    }

    if(handle == AHR_CurlGetHandle(AHR_RequestHandle(list->head->result->request)))
    {
        struct AHR_RequestListNode *next = list->head->next;
        free(list->head);
        list->head = next;
        return true;
    }
    
    struct AHR_RequestListNode trick = {.next=list->head, .result=NULL};
    struct AHR_RequestListNode *current = &trick;
    while(current)
    {
        {
            if(handle == AHR_CurlGetHandle(AHR_RequestHandle(current->next->result->request)))
            {
                struct AHR_RequestListNode *tmp = current->next->next;
                free(current->next);
                current->next = tmp;
                return true;
            }
        }
        current = current->next;
    }
    return false;
}

static AHR_Result_t* AHR_RequestListFind(AHR_RequestList *list, void *handle) // cppcheck-suppress constParameterPointer
{
    //
    // The Argument "handle" is not made const because someone can retrive it later on 
    // via another call to one of teh List Functions and do work with it.
    //
    struct AHR_RequestListNode *current = list->head;
    while(current)
    {
        if(handle == AHR_CurlGetHandle(AHR_RequestHandle(current->result->request)))
        {
            return current->result;
        }
        current = current->next;
    }
    return NULL;
}

static void AHR_HandleNewRequests(AHR_Processor_t processor)
{
    if(AHR_MutexTryLock(processor->mutex))
    {
        while(1)
        {
            AHR_Result_t *new = AHR_StackPop(&processor->requests);
            if(new)
            {
                if(
                    AHR_CurlMultiAddHandle(
                        processor->handle,
                        AHR_RequestHandle(new->request)
                    )
                )
                {
                    AHR_RequestListAdd(&processor->result_list, new);
                }
                else
                {
                    AHR_LogWarning(processor->logger, "Unable to give previously used Element back.");
                }
            }
            else
            {
                break;
            }
        }
        AHR_MutexUnlock(processor->mutex);
    }
}

static void AHR_CurlMultiInfoReadErrorCallback(
    void *arg,
    AHR_Curl_t handle,
    size_t error_code
)
{
    assert(NULL != arg);
    assert(NULL != AHR_CurlGetHandle(handle));

    AHR_Processor_t processor = (AHR_Processor_t)arg;
    AHR_Result_t *result = AHR_RequestListFind(
        &processor->result_list,
        AHR_CurlGetHandle(handle)
    );

    if(!result)
    {
        AHR_LogError(processor->logger, "Error expecting to find Result Object, but do not found it.");
        return;
    }
    assert(NULL != result->user_data.on_error);
    result->user_data.on_error(
        result->user_data.data,
        AHR_ResultStoreObjectIndex(&processor->result_store, result),
        error_code
    );
    const bool r = AHR_RequestListRemove(
        &processor->result_list,
        AHR_CurlGetHandle(AHR_RequestHandle(result->request))
    );
    
    AHR_LogInfo(processor->logger, "Remove Handle fom CURLM on Error...");
    AHR_CurlMultiRemoveHandle(
        processor->handle,
        handle
    );
    if(r)
    {
        //AHR_DestroyResult(&result);
        AHR_ProcessorUnlockResult(result);
    }
    else
    {
        AHR_LogError(processor->logger, "Unable to remove Element from Request List.");
    }
}

static void AHR_CurlMultiInfoReadSuccessCallback(
    void *arg,
    AHR_Curl_t handle
)
{
    assert(NULL != arg);
    assert(NULL != AHR_CurlGetHandle(handle));

    AHR_Processor_t processor = (AHR_Processor_t)arg;
    AHR_Result_t *result = AHR_RequestListFind(
        &processor->result_list,
        AHR_CurlGetHandle(handle)
    );
    if(!result)
    {
        AHR_LogError(processor->logger, "Error expecting to find Result Object, but do not found it.");
        return;
    }
    
    assert(NULL != result->user_data.on_success); 
    result->user_data.on_success(
        result->user_data.data,
        AHR_ResultStoreObjectIndex(&processor->result_store, result),
        AHR_ResponseStatusCode(result->response),
        AHR_ResponseBody(result->response),
        AHR_ResponseBodyLength(result->response)
    );
    const bool r = AHR_RequestListRemove(
        &processor->result_list,
        AHR_CurlGetHandle(AHR_RequestHandle(result->request))
    );
    AHR_CurlMultiRemoveHandle(
        processor->handle,
        handle
    );
    if(r)
    {
        //AHR_DestroyResult(&result);
        AHR_ProcessorUnlockResult(result);
    }
    else
    {
        AHR_LogError(processor->logger, "Unable to remove Element from Request List.");
    }
}

static void AHR_ExecuteAndPoll(AHR_Processor_t processor)
{
    assert(NULL != processor);
    
    int running_handles;
    //do
    //{
        const bool curl_perform = AHR_CurlMultiPerform(processor->handle, &running_handles);
        if(curl_perform && running_handles)
        {
            //AHR_CurlMultiPoll(processor->handle);
        }
        else if(!curl_perform)
        {
            AHR_LogError(processor->logger, "Unable to Poll...\n");
        }
        else
        {
            AHR_CurlMultiInfoReadData_t data = {
                .data = processor,
                .on_success=AHR_CurlMultiInfoReadSuccessCallback,
                .on_error=AHR_CurlMultiInfoReadErrorCallback
            };
            AHR_CurlMultiInfoRead(
                processor->handle,
                data
            ); 
        }
        AHR_CurlMultiPoll(processor->handle);
    //}
    //while(running_handles);
}

static bool AHR_ProcessorTryLockResult(AHR_Result_t *result)
{
    int expected = 0;
    return atomic_compare_exchange_strong(&result->busy, &expected, 1);
}

static void AHR_ProcessorUnlockResult(AHR_Result_t *result)
{
    atomic_store(&result->busy, 0);
}

static bool AHR_ProcessorIsResultBusy(AHR_Result_t *result)
{
    return 1 == atomic_load(&result->busy);
}
