
//
// --------------------------------------------------------------------------------------------------------------------
//
#include <async_http_requests/request_processor.h>
#include <async_http_requests/async_http_requests.h>
#include <async_http_requests/stack.h>
#include "external/inc/external/async_http_requests/ahr_curl.h"
#include <async_http_requests/private/result.h>

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <string.h>

#include <curl/curl.h>
//
// --------------------------------------------------------------------------------------------------------------------
//

///
/// \brief  Defines the Number of Transaktionobjects created initially.
///
#define AHR_NRESULTS 10

#define AHR_PROCESSOR_MAX_URL_LEN (4096-1)
#define AHR_PROCESSOR_MAX_BODY_SIZE ((4096 * 16)-1)

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
struct AHR_Processor
{
    AHR_Logger_t logger;
    ///
    /// \brief  Thread Id.
    /// 
    pthread_t thread_id;
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
    pthread_mutex_t mutex;

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
static void AHR_RequestListAdd(AHR_RequestList *list, AHR_Result_t *result);
///
/// \brief  Remove one Element from the Request-List.
///
static bool AHR_RequestListRemove(AHR_RequestList *list, void *handle);
///
/// ≤brief  Find an Element in the Request-List.
///
static AHR_Result_t* AHR_RequestListFind(AHR_RequestList *list, void *handle);
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
//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Processor_t AHR_CreateProcessor(AHR_Logger_t logger)
{
    //
    // Create a AHR_Processor_t handle.
    // The internal Datastructure sizes are dericved from AHR_NRESULTS.
    //
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    //
    AHR_Processor_t processor = malloc(sizeof(struct AHR_Processor));
    processor->handle = AHR_CurlMultiInit(); 
    //processor->request_list.head = NULL;
    
    // TODO: 
    //  - Malloc only on init -> pre-create all Resultobjects
    //  - Make url reconfigurable in request objects
    //  - Make Callback reconfigurable in response objects
    processor->result_store = AHR_CreateResultStore(AHR_NRESULTS);
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
    processor->requests = AHR_CraeteStack(AHR_NRESULTS);
    processor->result_list.head = NULL;
    
    pthread_mutex_init(&(processor->mutex), NULL);
    atomic_store(&(processor->terminate), 0);

    processor->logger = logger;
    processor->thread_id = NULL;

    return processor;
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
    if(NULL != processor->thread_id)
    {
        AHR_LogWarning(
            processor->logger, 
            "Uable to start Processors Thread because it was alreay started...\n"
        );
        return true;
    }
    // ----
    atomic_store(&processor->terminate, 0);
    if(0 == pthread_create(&(processor->thread_id), NULL, AHR_ProcessorThreadFunc, processor))
    {
        return true;
    }
    // ----
    AHR_LogError(processor->logger, "pthread Error!\n");
    return false;
}

void AHR_ProcessorStop(AHR_Processor_t processor)
{
    if(processor->thread_id)
    {
        atomic_store(&(processor->terminate), 1);
        void *result;
        pthread_join(processor->thread_id, &result);
        processor->thread_id = NULL;
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
    // Assertions...
    //
    assert(NULL != request_data);
    assert(NULL != request_data->url);

    AHR_Result_t *result = AHR_ResultStoreGetResult(
        &processor->result_store,
        object
    );
    //
    // Error handling...
    //
    // ---- 
    if(!result)
    {
        return AHR_PROC_UNKNOWN_OBJECT;
    }
    // ---- 
    if(!AHR_ProcessorTryLockResult(result))
    {
        return AHR_PROC_OBJECT_BUSY;
    }
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
        memcpy(result->request_data.body, request_data->body, strlen(request_data->body));
    }
    memset(result->request_data.url, '\0', AHR_PROCESSOR_MAX_URL_LEN+1);
    memcpy(result->request_data.url, request_data->url, AHR_PROCESSOR_MAX_URL_LEN);

    result->user_data = data;

    AHR_ProcessorUnlockResult(result);
    return AHR_PROC_OK; 
}

AHR_ProcessorStatus_t AHR_ProcessorMakeRequest(AHR_Processor_t processor, size_t object)
{
    AHR_ProcessorStatus_t retval = AHR_PROC_OK;
    pthread_mutex_lock(&processor->mutex);
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
    AHR_StackPush(&processor->requests, result);
    AHR_CurlMultiWeakUp(processor->handle);

end:
    pthread_mutex_unlock(&processor->mutex);
    return retval;
}

AHR_ProcessorStatus_t AHR_ProcessorGet(AHR_Processor_t processor, size_t object)
{
    AHR_ProcessorStatus_t status = AHR_PROC_OK;
    pthread_mutex_lock(&processor->mutex);

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
    AHR_Get(result->request, result->request_data.url, result->response);
    AHR_ProcessorUnlockResult(result);
end:
    pthread_mutex_unlock(&processor->mutex);
    return status;
}

AHR_Id_t AHR_ProcessorPost(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data)
{
    /*
    assert(NULL != processor);
    assert(NULL != data);

    pthread_mutex_lock(&processor->mutex);
    AHR_Result_t result = AHR_StackPop(&processor->unused_results);
    if(!result)
    {
        AHR_LogWarning(processor->logger, "Warning: Unable to retrieve unused element.");
        return NULL;
    }

    AHR_RequestSetHeader(result->request, &data->header);
    AHR_Post(result->request, data->url, data->body, result->response);
    result->user_data = user_data;
    
    AHR_StackPush(&processor->requests, result);
    
    pthread_mutex_unlock(&processor->mutex);

    AHR_CurlMultiWeakUp(processor->handle);
    return AHR_RequestUUID(result->request);
    */
    return NULL;
}

AHR_Id_t AHR_ProcessorPut(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data)
{
    /*
    pthread_mutex_lock(&processor->mutex);
    AHR_Result_t result = AHR_StackPop(&processor->unused_results);
    if(!result)
    {
        AHR_LogWarning(processor->logger, "Warning: Unable to retrieve unused element.");
        return NULL;
    }
    AHR_RequestSetHeader(result->request, &data->header);
    AHR_Put(result->request, data->url, data->body, result->response);
    result->user_data = user_data;
    
    AHR_StackPush(&processor->requests, result);
    
    pthread_mutex_unlock(&processor->mutex);
    AHR_CurlMultiWeakUp(processor->handle);
    return AHR_RequestUUID(result->request);
    */
    return NULL;
}

AHR_Id_t AHR_ProcessorDelete(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data)
{
    /*
    pthread_mutex_lock(&processor->mutex);
    AHR_Result_t result = AHR_StackPop(&processor->unused_results);
    if(!result)
    {
        AHR_LogWarning(processor->logger, "Warning: Unable to retrieve unused element.");
        return NULL;
    }
    AHR_RequestSetHeader(result->request, &data->header);
    AHR_Delete(result->request, data->url, result->response);
    result->user_data = user_data;
    
    AHR_StackPush(&processor->requests, result);
    
    pthread_mutex_unlock(&processor->mutex);
    AHR_CurlMultiWeakUp(processor->handle);
    return AHR_RequestUUID(result->request);
}

bool AHR_ProcessorIsResultReady(AHR_Result_t result)
{
    return atomic_load(&(result->done)) == 1;
}

bool AHR_DestroyResult(AHR_Result_t *result)
{
    if(!(*result)) return false;

    AHR_DestroyRequest(&(*result)->request);
    AHR_DestroyResponse(&(*result)->response);
    free(*result);
    *result = NULL;
    */
    return NULL;
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

static void AHR_RequestListAdd(AHR_RequestList *list, AHR_Result_t *result)
{
    if(list->head)
    {
        struct AHR_RequestListNode *current = list->head;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = malloc(sizeof(struct AHR_RequestListNode));
        current->next->next = NULL;
        current->next->result = result;
    }
    else
    {
        list->head = malloc(sizeof(struct AHR_RequestListNode));
        list->head->next = NULL;
        list->head->result = result;
    }
} 

static bool AHR_RequestListRemove(AHR_RequestList *list, void *handle)
{
    if(NULL == list->head)
    {
        return false;
    }

    if(handle == AHR_RequestHandle(list->head->result->request).handle)
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
            if(handle == AHR_RequestHandle(current->next->result->request).handle)
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

static AHR_Result_t* AHR_RequestListFind(AHR_RequestList *list, void *handle)
{
    struct AHR_RequestListNode *current = list->head;
    while(current)
    {
        if(handle == AHR_RequestHandle(current->result->request).handle)
        {
            return current->result;
        }
        current = current->next;
    }
    return NULL;
}

static void AHR_HandleNewRequests(AHR_Processor_t processor)
{
    const int lresult = pthread_mutex_trylock(&processor->mutex);
    if(0 == lresult)
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
        pthread_mutex_unlock(&processor->mutex);
    }
}

static void AHR_CurlMultiInfoReadCallback(
    void *arg,
    AHR_Curl_t handle
)
{
    assert(NULL != arg);
    assert(NULL != handle.handle);

    AHR_Processor_t processor = (AHR_Processor_t)arg;
    AHR_Result_t *result = AHR_RequestListFind(
        &processor->result_list,
        handle.handle
    );
    if(!result)
    {
        AHR_LogError(processor->logger, "Error expecting to find Result Object, but do not found it.");
        return;
    }
    
    assert(NULL != result->user_data.on_success); 
    result->user_data.on_success(
        result->user_data.data,
        result->response
    );
    const bool r = AHR_RequestListRemove(
        &processor->result_list,
        AHR_RequestHandle(result->request).handle
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
                .on_success=AHR_CurlMultiInfoReadCallback,
                .on_error=AHR_CurlMultiInfoReadCallback
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
