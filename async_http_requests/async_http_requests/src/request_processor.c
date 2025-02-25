
//
// --------------------------------------------------------------------------------------------------------------------
//
#include <async_http_requests/request_processor.h>
#include <async_http_requests/async_http_requests.h>
#include <async_http_requests/stack.h>
#include "external/inc/external/async_http_requests/ahr_curl.h"

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <string.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

///
/// \brief  Defines the Number of Transaktionobjects created initially.
///
#define AHR_NRESULTS 128

//
// --------------------------------------------------------------------------------------------------------------------
//

///
/// \brief  This Structure holds the state of a Transaktion with the Server.
///
struct AHR_Result
{
    atomic_int done;
    size_t error;
    AHR_HttpRequest_t request;
    AHR_HttpResponse_t response;

    AHR_UserData_t user_data;
};

struct AHR_RequestListNode;
///
/// \brief  Implementation of a simple List.
///
struct AHR_RequestListNode
{
    struct AHR_RequestListNode *next;
    AHR_Result_t result;
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
    /// \brief  Inactive and unused Objects.
    ///
    AHR_Stack_t unused_results;
    ///
    /// \brief Object which are requested to execute. 
    ///
    AHR_Stack_t requests;
    ///
    /// \brief Finished Requests, waiting to be put back to the unused_results. 
    ///
    AHR_RequestList result_list;
};

//
// --------------------------------------------------------------------------------------------------------------------
//

///
/// \brief  Add one Element to the Request-List.
///
static void AHR_RequestListAdd(AHR_RequestList *list, AHR_Result_t result);
///
/// \brief  Remove one Element from the Request-List.
///
static bool AHR_RequestListRemove(AHR_RequestList *list, void *handle);
///
/// ≤brief  Find an Element in the Request-List.
///
static AHR_Result_t AHR_RequestListFind(AHR_RequestList *list, void *handle);
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
/// \brief  Create a AHR_Result_t Object.
///
static AHR_Result_t AHR_CreateResult(void);
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
    AHR_Processor_t processor = malloc(sizeof(struct AHR_Processor));
    processor->handle = AHR_CurlMultiInit(); 
    //processor->request_list.head = NULL;
    
    // TODO: 
    //  - Malloc only on init -> pre-create all Resultobjects
    //  - Make url reconfigurable in request objects
    //  - Make Callback reconfigurable in response objects
    processor->unused_results = AHR_CraeteStack(AHR_NRESULTS);
    for(size_t i=0;i<AHR_NRESULTS;++i)
    {
        AHR_Result_t new = AHR_CreateResult();
        AHR_RequestSetLogger(new->request, logger);
        AHR_ResponseSetLogger(new->response, logger);
        AHR_StackPush(&processor->unused_results, new);
        //CURLMcode c = curl_multi_add_handle(
        //    processor->handle,
        //    AHR_RequestHandle(new->request)
        //);
    }
    processor->requests = AHR_CraeteStack(AHR_NRESULTS);
    processor->result_list.head = NULL;
    
    pthread_mutex_init(&(processor->mutex), NULL);
    atomic_store(&(processor->terminate), 0);

    processor->logger = logger;
    processor->thread_id = 0;

    return processor;
}

void AHR_DestroyProcessor(AHR_Processor_t *processor)
{
    if(!*processor) return;

    AHR_ProcessorStop(*processor);
    AHR_CurlMultiCleanUp((*processor)->handle);
    
    free(*processor);
    *processor = NULL;
}

bool AHR_ProcessorStart(AHR_Processor_t processor)
{
    if(0 == pthread_create(&(processor->thread_id), NULL, AHR_ProcessorThreadFunc, processor))
    {
        return true;
    }
    return false;
}

void AHR_ProcessorStop(AHR_Processor_t processor)
{
    if(processor->thread_id)
    {
        atomic_store(&(processor->terminate), 1);
        void *result;
        pthread_join(processor->thread_id, &result);
    }
}

AHR_Id_t AHR_ProcessorGet(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data)
{
    // TODO: Reuse Sockets...
    //  - Map: char* -> AHR_Result_t
    //
    // TODO: Error Handling if no unused result ist present!
    //
    pthread_mutex_lock(&processor->mutex);

    AHR_Result_t result = NULL;
    result = AHR_StackPop(&processor->unused_results);
    if(!result)
    {
        AHR_LogWarning(processor->logger, "Warning: Unable to retrieve unused element.");
        return NULL;
    }
    AHR_RequestSetHeader(result->request, &data->header);
    AHR_Get(result->request, data->url, result->response);
    result->user_data = user_data;
    
    AHR_StackPush(&processor->requests, result);
    pthread_mutex_unlock(&processor->mutex);

    AHR_CurlMultiWeakUp(processor->handle);
    return AHR_RequestUUID(result->request);
}

AHR_Id_t AHR_ProcessorPost(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data)
{
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
}

AHR_Id_t AHR_ProcessorPut(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data)
{
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
}

AHR_Id_t AHR_ProcessorDelete(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data)
{
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

    return true;
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
    }
    while(0 == atomic_load(&(processor->terminate)));
    return NULL;
}

static void AHR_RequestListAdd(AHR_RequestList *list, AHR_Result_t result)
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

    if(handle == AHR_RequestHandle(list->head->result->request))
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
            if(handle == AHR_RequestHandle(current->next->result->request))
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

static AHR_Result_t AHR_RequestListFind(AHR_RequestList *list, void *handle)
{
    struct AHR_RequestListNode *current = list->head;
    while(current)
    {
        if(handle == AHR_RequestHandle(current->result->request))
        {
            return current->result;
        }
        current = current->next;
    }
    return NULL;
}

static AHR_Result_t AHR_CreateResult(void)
{
    AHR_HttpRequest_t request = AHR_CreateRequest(); 
    AHR_HttpResponse_t response = AHR_CreateResponse();

    AHR_Result_t result = malloc(sizeof(struct AHR_Result)); 
    result->request = request;
    result->response = response;
    atomic_store(&(result->done), 0);
    result->user_data.on_success = NULL;

    return result;
}

static void AHR_HandleNewRequests(AHR_Processor_t processor)
{
    const int lresult = pthread_mutex_trylock(&processor->mutex);
    if(0 == lresult)
    {
        while(1)
        {
            AHR_Result_t new = AHR_StackPop(&processor->requests);
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
                    printf("Heeer\n");
                    AHR_LogWarning(processor->logger, "Unable to give previously used Element back.");
                    AHR_DestroyResult(&new);
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
    AHR_Processor_t processor = (AHR_Processor_t)arg;
    AHR_Result_t result = AHR_RequestListFind(
        &processor->result_list,
        handle
    );
     result->user_data.on_success(
        result->user_data.data,
        result->response
     );
     const bool r = AHR_RequestListRemove(
        &processor->result_list,
        AHR_RequestHandle(result->request)
     );
     AHR_CurlMultiRemoveHandle(
        processor->handle,
        handle
     );
     if(r)
     {
        //AHR_DestroyResult(&result);
        pthread_mutex_lock(&processor->mutex);
        AHR_StackPush(&processor->unused_results, result);
        pthread_mutex_unlock(&processor->mutex);
     }
     else
     {
        AHR_LogError(processor->logger, "Unable to remove Element from Request List.");
     }
}

static void AHR_ExecuteAndPoll(AHR_Processor_t processor)
{
    assert(NULL != processor);
    int num_fds = 0;
    int running_handles;
    if(
        AHR_CurlMultiPerform(processor->handle, &running_handles)
    )
    {
        if(running_handles <= 0)
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
    }
}
