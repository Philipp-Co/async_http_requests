#include <async_http_requests/async_http_requests.h>

#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#define AHR_HEADER_NMAX 256



typedef struct 
{
    AHR_HeaderEntry_t header[AHR_HEADER_NMAX];    
    size_t nheaders;
} AHR_Header_t;

typedef struct
{
    char *data;
    size_t nbytes;
    size_t maxbytes;
} AHR_Body_t;

struct AHR_HttpRequest
{
    int file_descriptor;
    char *url;
    CURL *handle;

    char **header;
    AHR_Body_t body;
};

struct AHR_HttpResponse
{
    AHR_Body_t body;
    AHR_HttpRequest_t request;
};

static size_t AHR_WriteCallback(char *data, size_t size, size_t nmemb, void *clientp);
static size_t AHR_HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata);

static size_t AHR_ResponseAddHeader(AHR_HttpResponse_t response, const char* header, size_t nbytes);

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
    
    return request;
}

void AHR_PrintRequest(const AHR_HttpRequest_t request)
{
    printf(
        "{\"file_descriptor\": %i}\n", request->file_descriptor
    );
}

void AHR_PrintResponse(const AHR_HttpResponse_t response)
{

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

void AHR_Post(AHR_HttpRequest_t request, const char **headers, const char *body)
{
    curl_easy_setopt(request->handle, CURLOPT_POSTFIELDS, request->body.data);
    curl_easy_setopt(request->handle, CURLOPT_POSTFIELDSIZE, request->body.nbytes);
    curl_easy_setopt(request->handle, CURLOPT_POST, 1L);
    
    struct curl_slist *list = NULL;
    list = curl_slist_append(list, "Content-Type: application/json");
    list = curl_slist_append(list, "Accept: application/json");
    for(size_t i=0;headers[i] != NULL; ++i)
    {
        list = curl_slist_append(list, headers[i]);
    }
    curl_easy_setopt(request->handle, CURLOPT_HTTPHEADER, list);
    
    AHR_MakeRequest(request, NULL);
}

void AHR_Get(AHR_HttpRequest_t request, const char *url, const char* const* headers, AHR_HttpResponse_t response)
{
    memset(request->url, '\0', 4096);
    memcpy(request->url, url, strlen(url));
    // set http method
    curl_easy_setopt(request->handle, CURLOPT_HTTPGET, 1L);
    // prepare headers list
    struct curl_slist *list = NULL;
    for(size_t i=0;headers[i] != NULL; ++i)
    {
        list = curl_slist_append(list, headers[i]);
    }
    curl_easy_setopt(request->handle, CURLOPT_HTTPHEADER, list);
    // set userdata for callback
    curl_easy_setopt(request->handle, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(request->handle, CURLOPT_HEADERDATA, response);
    // make request
    AHR_MakeRequest(request, response);
}

const char* AHR_ResponseBody(const AHR_HttpResponse_t response)
{
    return response->body.data;
}

void AHR_ResponseHeader(const AHR_HttpResponse_t response, AHR_HeaderInfo_t *info)
{
    size_t i = 0;
    struct curl_header *prev = NULL;
    struct curl_header *h;
    while(i < info->maxheader && (h = curl_easy_nextheader(response->request->handle, CURLH_HEADER, 0, prev))) {
      memcpy(info->entries[i].name, h->name, strlen(h->name));
      memcpy(info->entries[i].value, h->value, strlen(h->value));
      prev = h;
      ++i;
    }
 
    /* extract the normal headers + 1xx + trailers from the last request */
    unsigned int origin = CURLH_HEADER| CURLH_1XX | CURLH_TRAILER;
    while(i < info->maxheader && (h = curl_easy_nextheader(response->request->handle, origin, -1, prev))) {
      memcpy(info->entries[i].name, h->name, strlen(h->name));
      memcpy(info->entries[i].value, h->value, strlen(h->value));
      prev = h;      
      ++i;
    }
    info->nheader = i;
}

long AHR_ResponseStatusCode(const AHR_HttpResponse_t response)
{
    long status = -1;
    curl_easy_getinfo(response->request->handle, CURLINFO_RESPONSE_CODE, &status);
    return status;
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
    return size * nitems;
}
