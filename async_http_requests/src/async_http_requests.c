
//
// --------------------------------------------------------------------------------------------------------------------
//

#include <async_http_requests/async_http_requests.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <curl/curl.h>

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
    CURL *handle;

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

static void AHR_RequestPrepare(
    long http_method,
    AHR_HttpRequest_t request,
    const char* url,
    const char* body,
    AHR_HttpResponse_t response
);

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
    request->handle = curl_easy_init();
    curl_easy_setopt(request->handle, CURLOPT_WRITEFUNCTION, AHR_WriteCallback);
    curl_easy_setopt(request->handle, CURLOPT_HEADERFUNCTION, AHR_HeaderCallback);
   
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

    curl_easy_cleanup((*request)->handle);
    (*request)->url = NULL;
    (*request)->handle = NULL;
    *request = NULL;
}

AHR_Status_t AHR_MakeRequest(AHR_HttpRequest_t request, AHR_HttpResponse_t response) 
{
    curl_easy_setopt(request->handle, CURLOPT_URL, request->url);
    curl_easy_perform(request->handle);

    response->request = request;

    return AHR_OK;
} 

void AHR_RequestSetHeader(AHR_HttpRequest_t request, const AHR_Header_t *header)
{
    memcpy(&request->header, header, sizeof(AHR_Header_t));
}

void AHR_Post(AHR_HttpRequest_t request, const char *url, const char *body, AHR_HttpResponse_t response)
{
    //
    // append content-type Header
    //
    memset(request->header.header[request->header.nheaders].name, '\0', AHR_HEADERENTRY_NAME_LEN);
    memset(request->header.header[request->header.nheaders].value, '\0', AHR_HEADERENTRY_VALUE_LEN);
    strcat(request->header.header[request->header.nheaders].name, "Content-Type");
    strcat(request->header.header[request->header.nheaders].value, "application/json");

    AHR_RequestPrepare(
        CURLOPT_MIMEPOST,
        request,
        url,
        body,
        response
    );
    AHR_MakeRequest(request, NULL);
}

void AHR_Get(AHR_HttpRequest_t request, const char *url, AHR_HttpResponse_t response)
{
    AHR_RequestPrepare(
        CURLOPT_HTTPGET,
        request,
        url,
        NULL,
        response
    );
    AHR_MakeRequest(request, response);
}

void AHR_Put(AHR_HttpRequest_t request, const char *url, const char *body, AHR_HttpResponse_t response)
{
    AHR_RequestPrepare(
        CURLOPT_UPLOAD,
        request,
        url,
        body,
        response
    );
    AHR_MakeRequest(request, response);
}

void AHR_Delete(AHR_HttpRequest_t request, const char *url, AHR_HttpResponse_t response)
{
    AHR_RequestPrepare(
        CURLOPT_CUSTOMREQUEST,
        request,
        url,
        NULL,
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
    long redirect_count = 0;
    curl_easy_getinfo(response->request->handle, CURLINFO_REDIRECT_COUNT, &redirect_count);
    memcpy(info, &response->header, sizeof(AHR_Header_t));
}

long AHR_ResponseStatusCode(const AHR_HttpResponse_t response)
{
    long status = -1;
    curl_easy_getinfo(response->request->handle, CURLINFO_RESPONSE_CODE, &status);
    return status;
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
        return CURL_WRITEFUNC_ERROR;
    }
    // check if the given buffer can hold more bytes
    // this should be a \0 terminated string.
    const size_t nbytes = size * nmemb; 
    if((nbytes + response->body.nbytes - 1) >= response->body.maxbytes)
    {
        return CURL_WRITEFUNC_ERROR;
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
    if(!response) return CURLE_READ_ERROR;

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

static void AHR_RequestPrepare(
    long http_method,
    AHR_HttpRequest_t request, 
    const char* url, 
    const char* body,
    AHR_HttpResponse_t response
)
{
    char header[AHR_HEADERENTRY_NAME_LEN + AHR_HEADERENTRY_VALUE_LEN + 1];

    if(CURLOPT_CUSTOMREQUEST == http_method)
    {
        curl_easy_setopt(request->handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    else if(CURLOPT_MIMEPOST == http_method)
    {
        curl_easy_setopt(request->handle, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(request->handle, CURLOPT_POSTFIELDSIZE, strlen(body));
    }

    memset(request->url, '\0', 4096);
    memcpy(request->url, url, strlen(url));
    // set http method
    curl_easy_setopt(request->handle, http_method, 1L);
    // prepare headers list
    struct curl_slist *list = NULL;
    for(size_t i=0;request->header.nheaders; ++i)
    {
        sprintf(header, "%s:%s", request->header.header[i].name, request->header.header[i].value);
        list = curl_slist_append(list, header);
    }
    curl_easy_setopt(request->handle, CURLOPT_HTTPHEADER, list);
    curl_slist_free_all(list);
    // set userdata for callback
    curl_easy_setopt(request->handle, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(request->handle, CURLOPT_HEADERDATA, response);
}
//
// --------------------------------------------------------------------------------------------------------------------
//
