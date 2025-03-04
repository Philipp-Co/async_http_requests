#ifndef __TYPES_H__
#define __TYPES_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

#include <stddef.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

#define AHR_HEADERENTRY_NAME_LEN 256
#define AHR_HEADERENTRY_VALUE_LEN (4096 - (AHR_HEADERENTRY_NAME_LEN))

#define AHR_HEADER_NMAX 256

//
// --------------------------------------------------------------------------------------------------------------------
//

struct AHR_HttpRequest;
typedef struct AHR_HttpRequest* AHR_HttpRequest_t;

struct AHR_HttpResponse;
typedef struct AHR_HttpResponse* AHR_HttpResponse_t;


typedef struct
{
    char name[AHR_HEADERENTRY_NAME_LEN];
    char value[AHR_HEADERENTRY_VALUE_LEN];
} AHR_HeaderEntry_t;

typedef struct 
{
    AHR_HeaderEntry_t header[AHR_HEADER_NMAX];    
    size_t nheaders;
} AHR_Header_t;

typedef struct
{
    void* handle;
    struct curl_slist *http_header;
} AHR_Curl_t;

typedef struct
{ 
    AHR_Header_t header;
    char *url;
    char *body;
    size_t loglevel;
} AHR_RequestData_t;

typedef void* AHR_Id_t;
typedef void (*AHR_ResponseReadyCallback)(void *user_data, AHR_HttpResponse_t response);
typedef struct
{
    void *data;
    AHR_ResponseReadyCallback on_success;
} AHR_UserData_t;

//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
