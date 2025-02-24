
//
// --------------------------------------------------------------------------------------------------------------------
//
#include <async_http_requests/request_processor.h>
#include <async_http_requests/async_http_requests.h>
#include <async_http_requests/stack.h>

#include "curl/curl.h"
#include "curl/multi.h"

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
    CURLM *handle;
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

AHR_Processor_t AHR_CreateProcessor(void)
{
    //
    // Create a AHR_Processor_t handle.
    // The internal Datastructure sizes are dericved from AHR_NRESULTS.
    //

    AHR_Processor_t processor = malloc(sizeof(struct AHR_Processor));
    processor->handle = curl_multi_init();
    //processor->request_list.head = NULL;
    
    // TODO: 
    //  - Malloc only on init -> pre-create all Resultobjects
    //  - Make url reconfigurable in request objects
    //  - Make Callback reconfigurable in response objects
    processor->unused_results = AHR_CraeteStack(AHR_NRESULTS);
    for(size_t i=0;i<AHR_NRESULTS;++i)
    {
        AHR_Result_t new = AHR_CreateResult();
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
    if(0 == pthread_create(&(processor->thread_id), NULL, AHR_ProcessorThreadFunc, processor))
        return processor;
    else
    {
        printf("%s\n", strerror(errno));
    }
    free(processor);
    return NULL;
}

void AHR_DestroyProcessor(AHR_Processor_t *processor)
{
    if(!*processor) return;

    atomic_store(&((*processor)->terminate), 1);
    void *result;
    pthread_join((*processor)->thread_id, &result);

    curl_multi_cleanup((*processor)->handle);

    printf("Processor %p destroyed\n", *processor);
    free(*processor);
    *processor = NULL;
}

void* AHR_ProcessorGet(AHR_Processor_t processor, const char *url, AHR_UserData_t user_data)
{
    const char* const headers[] ={
        NULL
    };
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
        printf("Error while poping unused result!\n");
        return NULL;
    }
    AHR_Get(result->request, url, headers, result->response);
    result->user_data = user_data;
    
    AHR_StackPush(&processor->requests, result);
    pthread_mutex_unlock(&processor->mutex);

    curl_multi_wakeup(processor->handle);
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
                CURLMcode c = curl_multi_add_handle(
                    processor->handle,
                    AHR_RequestHandle(new->request)
                );
                if(CURLM_OK == c)
                {
                    AHR_RequestListAdd(&processor->result_list, new);
                }
                else
                {
                    printf("Unable to integrate result...\n");
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

static void AHR_ExecuteAndPoll(AHR_Processor_t processor)
{
    int num_fds = 0;
    int running_handles;
    CURLMcode mc;
    mc = curl_multi_perform(processor->handle, &running_handles);
    if(CURLM_OK == mc)
    {
        if(running_handles <= 0)
        {
            int msg_in_queue = 0;
            CURLMsg *m;
            do
            {
                m = curl_multi_info_read(processor->handle, &msg_in_queue);
                if(m && (CURLMSG_DONE == m->msg))
                {
                    if(CURLE_OK != m->data.result)
                    {
                        printf("Error %i for handler %p\n", m->data.result, m->easy_handle);
                    }

                    AHR_Result_t result = AHR_RequestListFind(
                        &processor->result_list,
                        m->easy_handle
                    );
                    if(result)
                    {
                        result->user_data.on_success(
                            result->user_data.data,
                            result->response
                        );
                        const bool r = AHR_RequestListRemove(
                            &processor->result_list,
                            AHR_RequestHandle(result->request)
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
                            printf("Unable to remove...\n");
                        }
                    }
                    else
                    {
                        printf("handle not found!\n");
                    }
                    curl_multi_remove_handle(processor->handle, m->easy_handle);
                }
            }
            while(m);
        }
        mc = curl_multi_poll(processor->handle, NULL, 0, 1000, &num_fds); 
        if(CURLM_OK != mc)
        {
            printf("Error while polling!\n");
        }
    }
}
