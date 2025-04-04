
//
// --------------------------------------------------------------------------------------------------------------------
//

#include <async_http_requests/async_http_requests.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <curl/curl.h>
#include "async_http_requests/types.h"
#include "external/inc/external/async_http_requests/ahr_curl.h"

//
// --------------------------------------------------------------------------------------------------------------------
//

typedef struct
{
    char *data;
    size_t nbytes;
    size_t maxbytes;
} AHR_Body_t;

struct AHR_HttpRequest
{
    void *uuid;

    int file_descriptor;
    char *url;
    AHR_Curl_t handle;

    AHR_Logger_t logger;

    AHR_Body_t body;

    struct curl_slist *http_header;
};

struct AHR_HttpResponse
{
    AHR_Body_t body;
    AHR_HttpRequest_t request;
    AHR_Logger_t logger;
    AHR_Header_t header;
};

//
// --------------------------------------------------------------------------------------------------------------------
//

static size_t AHR_WriteCallback(char *data, size_t size, size_t nmemb, void *clientp);
static size_t AHR_HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata);

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Curl_t AHR_RequestHandle(AHR_HttpRequest_t request)
{
    return request->handle;
}

AHR_HttpResponse_t AHR_CreateResponse(void)
{
    struct AHR_HttpResponse* response = malloc(sizeof(struct AHR_HttpResponse));
    if(!response)
    {
        return NULL;
    }

    static const size_t nbytes = 4096 * 64;
    response->body.data = malloc(nbytes);
    if(!response->body.data)
    {
        goto on_error;
    }
    response->body.maxbytes = nbytes;
    memset(response->body.data, '\0', nbytes);
    response->request = NULL;
    response->logger = NULL;
    memset(&response->header, '\0', sizeof(response->header));

    return response;

    on_error:
    AHR_DestroyResponse(&response);
    return NULL;
}

AHR_HttpRequest_t AHR_CreateRequest(void)
{
    struct AHR_HttpRequest* request = malloc(sizeof(struct AHR_HttpRequest));
    if(!request)
    {
        return NULL;
    }
    request->url = malloc(4096);
    if(!request->url)
    {
        goto on_error;
    }
    memset(request->url, '\0', 4096);

    const size_t nbytes = 4096 * 4;
    request->body.data = malloc(nbytes);
    if(!request->body.data)
    {
        goto on_error;
    }
    request->body.maxbytes = nbytes;
    request->body.nbytes = nbytes;
    memset(request->body.data, '\0', nbytes);
    
    // curl stuff...
    request->handle = AHR_CurlEasyInit(
        AHR_WriteCallback,
        AHR_HeaderCallback
    );
    request->uuid = request;
    return request;

    on_error:
    AHR_DestroyRequest(&request);
    return NULL;
}

void AHR_RequestSetLogger(AHR_HttpRequest_t request, AHR_Logger_t logger)
{
    request->logger = logger;
}

void AHR_ResponseSetLogger(AHR_HttpResponse_t response, AHR_Logger_t logger)
{
    response->logger = logger;
}

void AHR_DestroyResponse(AHR_HttpResponse_t *response)
{
    free((*response)->body.data);
    (*response)->body.data = NULL;
    (*response)->body.maxbytes = 0;
    (*response)->body.nbytes = 0;
    *response = NULL;
}

void AHR_DestroyRequest(AHR_HttpRequest_t *request)
{
    if((*request)->http_header)
    {
        curl_slist_free_all((*request)->http_header);
    }
    free((*request)->url);
    free((*request)->body.data);

    AHR_CurlEasyCleanUp((*request)->handle);
    (*request)->url = NULL;
    (*request)->handle = NULL;
    *request = NULL;
}

AHR_Status_t AHR_MakeRequest(AHR_HttpRequest_t request, AHR_HttpResponse_t response) 
{
    assert(NULL != request);
    assert(NULL != response);
    assert(NULL != request->url);
    AHR_CurlEasySetUrl(request->handle, request->url);
    response->request = request;

    return AHR_OK;
} 

void AHR_ResponseReset(AHR_HttpResponse_t response)
{
    memset(response->body.data, '\0', response->body.maxbytes);
    response->body.nbytes = 0;
    response->header.nheaders = 0;
}

void AHR_RequestSetHeader(AHR_HttpRequest_t request, const AHR_Header_t *header)
{
    AHR_CurlSetHeader(request->handle, header);
}

void AHR_Post(AHR_HttpRequest_t request, const char *url, const char *body, AHR_HttpResponse_t response)
{
    //
    // Buffers are allocated only during Initialization, the Size can not Change.
    // No Boundschecks required if the sizes are respected.
    //
    
    assert(NULL != request);
    assert(NULL != response);
    assert(NULL != url);
    //
    // append content-type Header
    //
    // TODO: Check  if additional Headers are possible...
    request->http_header = curl_slist_append(request->http_header, "Content-Type: application/json"); 
    request->http_header = curl_slist_append(request->http_header, "Accept: application/json"); 
    request->http_header = curl_slist_append(request->http_header, "charset: utf-8"); 

    const size_t len = strnlen(url, AHR_PROCESSOR_MAX_URL_LEN);
    memset(request->url, '\0', 4096);
    memcpy(request->url, url, len); // flawfinder: ignore
    
    AHR_CurlSetHeader(request->handle, NULL);
    AHR_CurlSetHttpMethodPost(request->handle, body);
    AHR_CurlSetCallbackUserData(
        request->handle,
        response,
        response
    );
    AHR_MakeRequest(request, response);
}

void AHR_Get(AHR_HttpRequest_t request, const char *url, AHR_HttpResponse_t response)
{
    //
    // Buffers are allocated only during Initialization, the Size can not Change.
    // No Boundschecks required if the sizes are respected.
    //
    
    assert(request != NULL);
    assert(response != NULL);
    assert(url != NULL);

    memset(request->url, '\0', 4096);
    const size_t len = strnlen(url, AHR_PROCESSOR_MAX_URL_LEN);
    const size_t nbytes = len >= 4095 ? 4095 : len;
    memcpy(request->url, url, nbytes); // flawfinder: ignore
    AHR_CurlSetHttpMethodGet(request->handle);
    AHR_CurlSetCallbackUserData(
        request->handle,
        response,
        response
    );
    AHR_MakeRequest(request, response);
}

void AHR_Put(AHR_HttpRequest_t request, const char *url, const char *body, AHR_HttpResponse_t response)
{
    //
    // Buffers are allocated only during Initialization, the Size can not Change.
    // No Boundschecks required if the sizes are respected.
    //
    
    assert(request != NULL);
    assert(response != NULL);
    assert(url != NULL);
    
    memset(request->url, '\0', 4096);
    const size_t len = strnlen(url, AHR_PROCESSOR_MAX_URL_LEN);
    const size_t nbytes = len >= 4095 ? 4095 : len;
    memcpy(request->url, url, nbytes); // flawfinder: ignore
    AHR_CurlSetHttpMethodPut(request->handle, body);
    AHR_CurlSetHeader(request->handle, NULL);
    AHR_CurlSetCallbackUserData(
        request->handle,
        response,
        response
    );
    AHR_MakeRequest(request, response);
}

void AHR_Delete(AHR_HttpRequest_t request, const char *url, AHR_HttpResponse_t response)
{
    //
    // Buffers are allocated only during Initialization, the Size can not Change.
    // No Boundschecks required if the sizes are respected.
    //
    
    memset(request->url, '\0', 4096);
    memcpy(request->url, url, strnlen(url, AHR_PROCESSOR_MAX_URL_LEN)); // flawfinder: ignore
    AHR_CurlSetHttpMethodDelete(request->handle);
    AHR_CurlSetHeader(request->handle, NULL);
    AHR_CurlSetCallbackUserData(
        request->handle,
        response,
        response
    );
    AHR_MakeRequest(request, response);
}

size_t AHR_ResponseBodyLength(const AHR_HttpResponse_t response)
{
    return response->body.nbytes;
}

const char* AHR_ResponseBody(const AHR_HttpResponse_t response)
{
    static const char *emptystr = "";
    return response->body.data ? response->body.data : emptystr;
}

void AHR_ResponseHeader(const AHR_HttpResponse_t response, AHR_Header_t *info)
{
    assert(NULL != response);

    memcpy(info, &response->header, sizeof(AHR_Header_t)); // flawfinder: ignore
}

long AHR_ResponseStatusCode(const AHR_HttpResponse_t response)
{
    if(response->request)
        return AHR_CurlEasyStatusCode(response->request->handle);
    return -1;
}

void* AHR_ResponseUUID(const AHR_HttpResponse_t response)
{
    return response->request->uuid;
}

void* AHR_RequestUUID(const AHR_HttpRequest_t request)
{
    return request->uuid;
}

//
// --------------------------------------------------------------------------------------------------------------------
//

static size_t AHR_WriteCallback(char *data, size_t size, size_t nmemb, void *clientp) // cppcheck-suppress constParameterCallback
{
    //
    // Buffers are allocated only during Initialization, the Size can not Change.
    // No Boundschecks required if the sizes are respected.
    //

    AHR_HttpResponse_t response = (AHR_HttpResponse_t)clientp;
    // check if the userdata is valid
    if(!response)
    {
        return AHR_CurlWriteError();
    }
    // check if the given buffer can hold more bytes
    // this should be a \0 terminated string.
    const size_t nbytes = size * nmemb; 
    if((nbytes + response->body.nbytes - 1) >= response->body.maxbytes)
    {
        printf("Write ERROR in AHR_WriteCallback!\n");
        return AHR_CurlWriteError();
    }
    // copy to userbuffer
    memcpy( // flawfinder: ignore
        response->body.data + response->body.nbytes,
        data,
        nbytes
    );
    response->body.nbytes = response->body.nbytes + nbytes;
    return nbytes;
}

static size_t AHR_HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    //
    // Buffers are allocated only during Initialization, the Size can not Change.
    // No Boundschecks required if the sizes are respected.
    //

    AHR_HttpResponse_t response = (AHR_HttpResponse_t)userdata; 
    if(!response) return AHR_CurlReadError();

    if(strnlen(buffer, AHR_HEADERENTRY_NAME_LEN + AHR_HEADERENTRY_VALUE_LEN - 2) < 3)
    {
        AHR_LogWarning(response->logger, "Unable to parse HTTP header...");
        return size * nitems;
    }

    char *token = strchr(buffer, ':');
    if(!token)
    {
        return size * nitems;
    }
    
    //
    // remove ':' and  ltrim
    //
    token += 1;
    while(!isalnum(*token) && '\0' != *token)
    {
        ++token;
    }

    memset(
        &response->header.header[response->header.nheaders],
        '\0',
        sizeof(response->header.header[response->header.nheaders])
    );
    memcpy( // flawfinder: ignore
        response->header.header[response->header.nheaders].name,
        buffer,
        token - buffer
    );
    memcpy( // flawfinder: ignore
        response->header.header[response->header.nheaders].value,
        token,
        strnlen(token, AHR_HEADERENTRY_VALUE_LEN-1)
    );
    response->header.nheaders++;
    return size * nitems;
}
//
// --------------------------------------------------------------------------------------------------------------------
//
