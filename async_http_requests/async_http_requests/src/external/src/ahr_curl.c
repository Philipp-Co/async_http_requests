
//
// --------------------------------------------------------------------------------------------------------------------
//

#include "async_http_requests/types.h"
#include <string.h>
#include <assert.h>

#include <external/async_http_requests/ahr_curl.h>

#include <curl/curl.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

typedef size_t (*AHR_CurlReadFunction_t)(char *ptr, size_t size, size_t nmemb, void *stream);

struct AHR_Curl
{
    void* handle;
    struct curl_slist *http_header;

    AHR_FileTransfer_t file_transfer;
};


struct AHR_CurlEasyHandleList;
struct AHR_CurlEasyHandleList
{
    AHR_Curl_t easy_handle;
    struct AHR_CurlEasyHandleList *next;
};

struct AHR_CurlM
{
    CURLM *handle;
    struct AHR_CurlEasyHandleList *easy_handles;
};

//
// --------------------------------------------------------------------------------------------------------------------
//

#define MAX_UPLOAD_SIZE 4096

//
// --------------------------------------------------------------------------------------------------------------------
//

static struct AHR_CurlEasyHandleList* AHR_CurlEasyHandleListAppendEasyHandle(
    struct AHR_CurlEasyHandleList *list,
    AHR_Curl_t easy_handle
);

static struct AHR_CurlEasyHandleList* AHR_CurlEasyHandleListRemoveEasyHandle(
    struct AHR_CurlEasyHandleList *list,
    AHR_Curl_t easy_handle
);

static AHR_Curl_t AHR_CurlEasyHandleListFindEasyHandle(
    struct AHR_CurlEasyHandleList *list,
    void *easy_handle 
);

static struct AHR_CurlEasyHandleList* AHR_CurlEasyHandleListRemoveAll(
    struct AHR_CurlEasyHandleList *list
);

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_CurlM_t AHR_CurlGetHandle(AHR_Curl_t handle)
{
    return handle->handle;
}

AHR_Curl_t AHR_CurlEasyInit(
    AHR_WriteCallback_t write_callback,
    AHR_HeaderCallback_t header_callback
)
{
    CURL *handle = curl_easy_init();

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5);

    const struct AHR_Curl content = {
        .handle = handle,
        .http_header = NULL,
        .file_transfer = {
            .data = malloc(MAX_UPLOAD_SIZE),
            .current_pos = 0,
            .size = 0
        }
    };
    AHR_Curl_t result = (AHR_Curl_t)malloc(sizeof(struct AHR_Curl));
    *result = content;
    return result;
}

void AHR_CurlSetCallbackUserData(
    AHR_Curl_t handle, 
    void *write_callback_user_data,
    void *header_callback_user_data
)
{
    curl_easy_setopt(handle->handle, CURLOPT_WRITEDATA, write_callback_user_data);
    curl_easy_setopt(handle->handle, CURLOPT_HEADERDATA, header_callback_user_data);
}

void AHR_CurlEasyCleanUp(AHR_Curl_t handle)
{
    if(handle->file_transfer.data)
    {
        free(handle->file_transfer.data);
    }
    if(handle->http_header)
    {
        curl_slist_free_all(handle->http_header);
        handle->http_header = NULL;
    }
    curl_easy_cleanup(handle->handle);
}

void AHR_CurlSetHeader(AHR_Curl_t handle, const AHR_Header_t *header)
{
    // TODO: Transfer Code here...
    if(!header)
    {
        return;
    }

    if(handle->http_header)
    {
        curl_slist_free_all(handle->http_header);
    }
    
    char buffer[AHR_HEADERENTRY_NAME_LEN + AHR_HEADERENTRY_VALUE_LEN + 2];
    handle->http_header = NULL;
    for(size_t i=0;i<header->nheaders;++i)
    {
        sprintf(buffer, "%s:%s", header->header[i].name, header->header[i].value);
        handle->http_header = curl_slist_append(handle->http_header, buffer);
    }
    curl_easy_setopt(handle->handle, CURLOPT_HTTPHEADER, handle->http_header);
}

void AHR_CurlEasySetUrl(AHR_Curl_t handle, const char *url)
{
    curl_easy_setopt(handle->handle, CURLOPT_URL, url);
}

bool AHR_CurlEasyPerform(AHR_Curl_t handle)
{
    return CURLE_OK == curl_easy_perform(handle->handle);
}

long AHR_CurlEasyStatusCode(AHR_Curl_t handle)
{
    long status_code = -1;
    curl_easy_getinfo(handle->handle, CURLINFO_RESPONSE_CODE, &status_code);
    return status_code;
}

void AHR_CurlSetHttpMethodGet(AHR_Curl_t handle)
{
    handle->http_header = curl_slist_append(handle->http_header, "Accept: application/json");
    curl_easy_setopt(handle->handle, CURLOPT_HTTPGET, 1L);
}

void AHR_CurlSetHttpMethodPost(AHR_Curl_t handle, const char *body)
{
    assert(NULL != handle.handle);
    handle->http_header = curl_slist_append(handle->http_header, "Accept: application/json");
    handle->http_header = curl_slist_append(handle->http_header, "Content-Type: application/json");
    curl_easy_setopt(handle->handle, CURLOPT_HTTPHEADER, handle->http_header);
    if(body)
    {
        curl_easy_setopt(handle->handle, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        curl_easy_setopt(handle->handle, CURLOPT_COPYPOSTFIELDS, body);
    }
}

//
// --------------------------------------------------------------------------------------------------------------------
//

static size_t AHR_PutReadCallback(char *ptr, size_t size, size_t nmemb, void *stream)
{
    AHR_Curl_t handle = (AHR_Curl_t)stream;
    if(handle->file_transfer.data && handle->file_transfer.size)
    {
        const size_t output_buffer_size = size * nmemb;
        const size_t bytes_left_to_transfer = handle->file_transfer.size - handle->file_transfer.current_pos;
        size_t bytes_to_transfer = bytes_left_to_transfer;
        if(output_buffer_size <= bytes_left_to_transfer)
        {
            bytes_to_transfer = output_buffer_size;
        }
        memcpy(
            ptr, 
            &handle->file_transfer.data[handle->file_transfer.current_pos], 
            bytes_to_transfer
        );

        handle->file_transfer.size -= bytes_to_transfer;
        handle->file_transfer.current_pos += bytes_to_transfer;
        return bytes_to_transfer;
    }
    return 0;
}

void AHR_CurlSetHttpMethodPut(AHR_Curl_t handle, const char *body)
{
    assert(NULL != body);

    const size_t body_size = strlen(body);
    if(body_size >= (MAX_UPLOAD_SIZE - 1))
    {
        return;
    }

    handle->http_header = curl_slist_append(handle->http_header, "Accept: application/json");
    handle->http_header = curl_slist_append(handle->http_header, "Content-Type: application/json");
    handle->http_header = curl_slist_append(handle->http_header, "Expect:");
    handle->http_header = curl_slist_append(handle->http_header, "Transfer-Encoding:");


    curl_easy_setopt(handle->handle, CURLOPT_HTTPHEADER, handle->http_header);
    curl_easy_setopt(handle->handle, CURLOPT_PUT, 1L);
    curl_easy_setopt(handle->handle, CURLOPT_UPLOAD, 1L);
    memset(handle->file_transfer.data, '\0', MAX_UPLOAD_SIZE);
    memcpy(handle->file_transfer.data, body, strlen(body));
    handle->file_transfer.current_pos = 0;
    handle->file_transfer.size = body_size;
    curl_easy_setopt(handle->handle, CURLOPT_INFILESIZE, (long)body_size);
    curl_easy_setopt(handle->handle, CURLOPT_READFUNCTION, AHR_PutReadCallback);
    curl_easy_setopt(handle->handle, CURLOPT_READDATA, handle);
}

void AHR_CurlSetHttpMethodDelete(AHR_Curl_t handle)
{
    handle->http_header = curl_slist_append(handle->http_header, "Accept: application/json");
    curl_easy_setopt(handle->handle, CURLOPT_CUSTOMREQUEST, "DELETE");
}

int AHR_CurlWriteError(void)
{
    return CURLE_WRITE_ERROR;
}

int AHR_CurlReadError(void)
{
    return CURLE_READ_ERROR;
}

//
// --------------------------------------------------------------------------------------------------------------------
//

bool AHR_CurlMultiAddHandle(AHR_CurlM_t handle, AHR_Curl_t ehandle)
{
    assert(NULL != handle);
    assert(NULL != handle->handle);
    assert(NULL != ehandle);
    
    handle->easy_handles = AHR_CurlEasyHandleListAppendEasyHandle(
        handle->easy_handles,
        ehandle
    );
    assert(NULL != handle->easy_handles);

    CURLMcode c = curl_multi_add_handle(
        handle->handle,
        ehandle->handle
    );
    return CURLM_OK == c;
}

void AHR_CurlMultiRemoveHandle(
    AHR_CurlM_t handle,
    AHR_Curl_t ehandle
)
{
    assert(NULL != handle);
    assert(NULL != handle->handle);
    assert(NULL != ehandle);

    handle->easy_handles = AHR_CurlEasyHandleListRemoveEasyHandle(
        handle->easy_handles,
        ehandle
    );
    curl_multi_remove_handle(
       handle->handle,
       ehandle->handle
    );
}

AHR_CurlM_t AHR_CurlMultiInit(void)
{
    AHR_CurlM_t result = malloc(sizeof(struct AHR_CurlM));
    
    result->handle = curl_multi_init(); 
    result->easy_handles = NULL;

    return result;
}

void AHR_CurlMultiCleanUp(AHR_CurlM_t handle)
{
    // TODO: clean up structs internals!
    handle->easy_handles = AHR_CurlEasyHandleListRemoveAll(handle->easy_handles);
    curl_multi_cleanup(handle->handle);
}

bool AHR_CurlMultiInfoRead(
    AHR_CurlM_t handle,
    AHR_CurlMultiInfoReadData_t callback 
)
{
    assert(NULL != handle);
    assert(NULL != handle->handle);

    int messages_in_queue = 0;
    CURLMsg *m;
    do
    {
        m = curl_multi_info_read(handle->handle, &messages_in_queue);
        if(m && (CURLMSG_DONE == m->msg))
        {
            AHR_Curl_t easy_handle = AHR_CurlEasyHandleListFindEasyHandle(
                handle->easy_handles,
                m->easy_handle
            ); 
            assert(NULL != easy_handle);

            if(CURLE_OK == m->data.result)
            {
                callback.on_success(callback.data, easy_handle);
            }
            else
            {
                callback.on_error(callback.data, easy_handle, 0);
            }
        }
    } while(m);
    return true;
}

bool AHR_CurlMultiPerform(AHR_CurlM_t handle, int *running_handles)
{
    assert(NULL != handle);
    assert(NULL != handle->handle);
    
    const CURLcode c = curl_multi_perform(handle->handle, running_handles);
    return c == CURLE_OK;
}

void AHR_CurlMultiPoll(AHR_CurlM_t handle)
{
    assert(NULL != handle);
    assert(NULL != handle->handle);
    
    int num_fds;
    curl_multi_poll(handle->handle, NULL, 0, 1000, &num_fds);
}

void AHR_CurlMultiWeakUp(AHR_CurlM_t handle)
{
    assert(NULL != handle);
    assert(NULL != handle->handle);
    
    curl_multi_wakeup(handle->handle);
}

//
// --------------------------------------------------------------------------------------------------------------------
//

static struct AHR_CurlEasyHandleList* AHR_CurlEasyHandleListAppendEasyHandle(
    struct AHR_CurlEasyHandleList *list,
    AHR_Curl_t easy_handle
)
{
    assert(NULL != easy_handle);
    assert(NULL != easy_handle->handle);

    struct AHR_CurlEasyHandleList *result = malloc(sizeof(struct AHR_CurlEasyHandleList));
    result->easy_handle = easy_handle;
    result->next = NULL;
    if(list)
    {
        struct AHR_CurlEasyHandleList *next = list;
        while(next)
        {
            if(next->next)
            {
                next = next->next;
            }
            else
            {
                next->next = result;
                return list;
            }
        }
        return list;
    }
    return result;
}

static struct AHR_CurlEasyHandleList* AHR_CurlEasyHandleListRemoveEasyHandle(
    struct AHR_CurlEasyHandleList *list,
    AHR_Curl_t easy_handle
)
{
    assert(NULL != easy_handle);
    assert(NULL != easy_handle->handle);

    if(list)
    {
        if(easy_handle == list->easy_handle)
        {
            struct AHR_CurlEasyHandleList *tmp = list->next;
            free(list);
            return tmp;
        }
    }
    
    struct AHR_CurlEasyHandleList *next = list;
    while(next)
    {
        if(next->next)
        {
            if(easy_handle == next->next->easy_handle)
            {
                struct AHR_CurlEasyHandleList *tmp = next->next;
                next->next = tmp->next;
                free(tmp);
                return list;
            }
        }
        next = next->next;
    }
    return list;
}

static AHR_Curl_t AHR_CurlEasyHandleListFindEasyHandle(
    struct AHR_CurlEasyHandleList *list,
    void *easy_handle 
)
{
    assert(NULL != easy_handle);

    struct AHR_CurlEasyHandleList *next = list;
    while(next)
    {
        if(easy_handle == next->easy_handle->handle)
        {
            return next->easy_handle;
        }
        next = next->next;
    }
    return NULL;
}

static struct AHR_CurlEasyHandleList* AHR_CurlEasyHandleListRemoveAll(
    struct AHR_CurlEasyHandleList *list
)
{
    struct AHR_CurlEasyHandleList *next = list;
    struct AHR_CurlEasyHandleList *tmp;
    while(next)
    {
        tmp = next;
        next = next->next;
        free(tmp);
    }
    return NULL; 
}
//
// --------------------------------------------------------------------------------------------------------------------
//
