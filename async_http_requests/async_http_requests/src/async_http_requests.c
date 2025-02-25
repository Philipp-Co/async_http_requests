
//
// --------------------------------------------------------------------------------------------------------------------
//

#include <async_http_requests/async_http_requests.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <curl/curl.h>
//#include <external/async_http_requests/ahr_curl.h>
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

    AHR_Header_t header;
    AHR_Body_t body;
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

static size_t AHR_ResponseAddHeader(AHR_HttpResponse_t response, const char* header, size_t nbytes);

//
// --------------------------------------------------------------------------------------------------------------------
//

void* AHR_RequestHandle(AHR_HttpRequest_t request)
{
    return request->handle;
}

AHR_HttpResponse_t AHR_CreateResponse(void)
{
    struct AHR_HttpResponse* response = malloc(sizeof(struct AHR_HttpResponse));
    static const size_t nbytes = 4096 * 64;
    response->body.data = malloc(nbytes);
    response->body.maxbytes = nbytes;
    memset(response->body.data, '\0', nbytes);
    response->request = NULL;
    response->logger = NULL;
    memset(&response->header, '\0', sizeof(response->header));

    return response;
}

AHR_HttpRequest_t AHR_CreateRequest(void)
{
    struct AHR_HttpRequest* request = malloc(sizeof(struct AHR_HttpRequest));
    request->url = malloc(4096);
    memset(request->url, '\0', 4096);

    const size_t nbytes = 4096 * 4;
    request->body.data = malloc(nbytes);
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
    free((*request)->url);
    free((*request)->body.data);

    AHR_CurlEasyCleanUp((*request)->handle);
    (*request)->url = NULL;
    (*request)->handle = NULL;
    *request = NULL;
}

AHR_Status_t AHR_MakeRequest(AHR_HttpRequest_t request, AHR_HttpResponse_t response) 
{
    AHR_CurlEasySetUrl(request->handle, request->url);
    AHR_CurlEasyPerform(request->handle);
    response->request = request;

    return AHR_OK;
} 

void AHR_RequestSetHeader(AHR_HttpRequest_t request, const AHR_Header_t *header)
{
    memcpy(&request->header, header, sizeof(AHR_Header_t));
}

void AHR_RequestGetHeader(AHR_HttpRequest_t request, AHR_Header_t *header)
{
    memcpy(header, &request->header, sizeof(AHR_Header_t));
}

void AHR_Post(AHR_HttpRequest_t request, const char *url, const char *body, AHR_HttpResponse_t response)
{
    assert(NULL != request);
    assert(NULL != response);
    assert(NULL != url);
    //
    // append content-type Header
    //
    // TODO: Check  if additional Headers are possible...
    if(request->header.nheaders >= AHR_HEADER_NMAX)
    {
        request->header.nheaders = AHR_HEADER_NMAX - 1;
    }
    static const char *content_type = "Content-Type\0";
    static const char *content_type_value = "application/json\0";
    const size_t header_idx = request->header.nheaders;
    memcpy(request->header.header[header_idx].name, content_type, strlen(content_type) + 1);
    memcpy(request->header.header[header_idx].value, content_type_value, strlen(content_type_value) + 1);
    request->header.nheaders++;
    
    memset(request->url, '\0', 4096);
    memcpy(request->url, url, strlen(url));
    
    AHR_CurlSetHeader(request->handle, &request->header);
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
    memset(request->url, '\0', 4096);
    memcpy(request->url, url, strlen(url));
    AHR_CurlSetHttpMethodGet(request->handle);
    AHR_CurlSetHeader(request->handle, &request->header);
    AHR_CurlSetCallbackUserData(
        request->handle,
        response,
        response
    );
    AHR_MakeRequest(request, response);
}

void AHR_Put(AHR_HttpRequest_t request, const char *url, const char *body, AHR_HttpResponse_t response)
{
    memset(request->url, '\0', 4096);
    memcpy(request->url, url, strlen(url));
    AHR_CurlSetHttpMethodPut(request->handle, body);
    AHR_CurlSetHeader(request->handle, &request->header);
    AHR_CurlSetCallbackUserData(
        request->handle,
        response,
        response
    );
    AHR_MakeRequest(request, response);
}

void AHR_Delete(AHR_HttpRequest_t request, const char *url, AHR_HttpResponse_t response)
{
    memset(request->url, '\0', 4096);
    memcpy(request->url, url, strlen(url));
    AHR_CurlSetHttpMethodDelete(request->handle);
    AHR_CurlSetHeader(request->handle, &request->header);
    AHR_CurlSetCallbackUserData(
        request->handle,
        response,
        response
    );
    AHR_MakeRequest(request, response);
}

const char* AHR_ResponseBody(const AHR_HttpResponse_t response)
{
    return response->body.data;
}

void AHR_ResponseHeader(const AHR_HttpResponse_t response, AHR_Header_t *info)
{
    memcpy(info, &response->header, sizeof(AHR_Header_t));
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

static size_t AHR_WriteCallback(char *data, size_t size, size_t nmemb, void *clientp)
{
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
        return AHR_CurlWriteError();
    }
    // copy to userbuffer
    memcpy(
        response->body.data + response->body.nbytes,
        data,
        nbytes
    );
    response->body.nbytes = response->body.nbytes + nbytes;
    return nbytes;
}

static size_t AHR_HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    AHR_HttpResponse_t response = (AHR_HttpResponse_t)userdata; 
    if(!response) return AHR_CurlReadError();

    if(strlen(buffer) < 3)
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
    memcpy(
        response->header.header[response->header.nheaders].name,
        buffer,
        token - buffer
    );
    memcpy(
        response->header.header[response->header.nheaders].value,
        token,
        strlen(token)
    );
    response->header.nheaders++;
    return size * nitems;
}
//
// --------------------------------------------------------------------------------------------------------------------
//
