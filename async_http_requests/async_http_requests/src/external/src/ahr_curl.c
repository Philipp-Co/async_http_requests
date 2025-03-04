
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

struct AHR_Curl
{
    CURL* handle;
    struct curl_slist *http_header;
};

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Curl_t AHR_CurlEasyInit(
    AHR_WriteCallback_t write_callback,
    AHR_HeaderCallback_t header_callback
)
{
    CURL *handle = curl_easy_init();

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_callback);
    
    AHR_Curl_t result = {
        .handle = handle,
        .http_header = NULL
    };
    return result;
}

void AHR_CurlSetCallbackUserData(
    AHR_Curl_t handle, 
    void *write_callback_user_data,
    void *header_callback_user_data
)
{
    curl_easy_setopt(handle.handle, CURLOPT_WRITEDATA, write_callback_user_data);
    curl_easy_setopt(handle.handle, CURLOPT_HEADERDATA, header_callback_user_data);
}

void AHR_CurlEasyCleanUp(AHR_Curl_t handle)
{
    curl_easy_cleanup(handle.handle);
}

void AHR_CurlSetHeader(AHR_Curl_t *handle, const AHR_Header_t *header)
{
    // TODO: Transfer Code here...

    if(handle->http_header)
    {
        printf("Free curl list..\n");
        curl_slist_free_all(handle->http_header);
    }
    
    handle->http_header = NULL;
    for(size_t i=0;i<header->nheaders;++i)
    {
        char *buffer = malloc(AHR_HEADERENTRY_NAME_LEN + AHR_HEADERENTRY_VALUE_LEN + 2);
        printf("Appedn Header %s\n", buffer);
        sprintf(buffer, "%s:%s", header->header[i].name, header->header[i].value);
        handle->http_header = curl_slist_append(handle->http_header, buffer);
    }
    const CURLcode c = curl_easy_setopt(handle->handle, CURLOPT_HTTPHEADER, handle->http_header);
}

void AHR_CurlEasySetUrl(AHR_Curl_t handle, const char *url)
{
    curl_easy_setopt(handle.handle, CURLOPT_URL, url);
}

bool AHR_CurlEasyPerform(AHR_Curl_t handle)
{
    return CURLE_OK == curl_easy_perform(handle.handle);
}

long AHR_CurlEasyStatusCode(AHR_Curl_t handle)
{
    long status_code = -1;
    curl_easy_getinfo(handle.handle, CURLINFO_RESPONSE_CODE, &status_code);
    return status_code;
}

void AHR_CurlSetHttpMethodGet(AHR_Curl_t handle)
{
    curl_easy_setopt(handle.handle, CURLOPT_HTTPGET, 1L);
}

void AHR_CurlSetHttpMethodPost(AHR_Curl_t handle, const char *body)
{
    assert(NULL != handle.handle);
    if(body)
    {
        assert(
            CURLE_OK == curl_easy_setopt(handle.handle, CURLOPT_POSTFIELDSIZE, (long)strlen(body))
        );
        assert(
            CURLE_OK == curl_easy_setopt(handle.handle, CURLOPT_POSTFIELDS, body)
        );
    }
}

void AHR_CurlSetHttpMethodPut(AHR_Curl_t handle, const char *body)
{
    curl_easy_setopt(handle.handle, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(handle.handle, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(handle.handle, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
}

void AHR_CurlSetHttpMethodDelete(AHR_Curl_t handle)
{
    curl_easy_setopt(handle.handle, CURLOPT_CUSTOMREQUEST, "DELETE");
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
    CURLMcode c = curl_multi_add_handle(
        handle,
        ehandle.handle
    );
    return CURLM_OK == c;
}

void AHR_CurlMultiRemoveHandle(
    AHR_CurlM_t handle,
    AHR_Curl_t ehandle
)
{
     curl_multi_remove_handle(
        handle,
        ehandle.handle
     );
}

AHR_CurlM_t AHR_CurlMultiInit(void)
{
    return curl_multi_init(); 
}

void AHR_CurlMultiCleanUp(AHR_CurlM_t handle)
{
    curl_multi_cleanup(handle);
}

bool AHR_CurlMultiInfoRead(
    AHR_CurlM_t handle,
    AHR_CurlMultiInfoReadData_t callback 
)
{
    int messages_in_queue = 0;
    CURLMsg *m;
    do
    {
        m = curl_multi_info_read(handle, &messages_in_queue);
        if(m && (CURLMSG_DONE == m->msg))
        {
            AHR_Curl_t handle = {.handle=m->easy_handle, .http_header=NULL};
            if(CURLE_OK == m->data.result)
            {
                callback.on_success(callback.data, handle);
            }
            else
            {
                printf("Error detected! Error Code: %s\n", curl_easy_strerror(m->data.result));
                callback.on_error(callback.data, handle);
            }
        }
    } while(m);
    return true;
}

bool AHR_CurlMultiPerform(AHR_CurlM_t handle, int *running_handles)
{
    const CURLcode c = curl_multi_perform(handle, running_handles);
    return c == CURLE_OK;
}

void AHR_CurlMultiPoll(AHR_CurlM_t handle)
{
    int num_fds;
    curl_multi_poll(handle, NULL, 0, 1000, &num_fds);
}

void AHR_CurlMultiWeakUp(AHR_CurlM_t handle)
{
    curl_multi_wakeup(handle);
}

//
// --------------------------------------------------------------------------------------------------------------------
//
