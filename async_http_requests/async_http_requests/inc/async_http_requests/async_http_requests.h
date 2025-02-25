///
/// \brief  This implements the public interface to the asyn http requests module.
///
/// \example    AHR_Request_t request = AHR_CreateRequest();
///             ...
///
///             const AHR_Status_t request_status = AHR_MakeRequest(request);
///             ...
///
///             if(request_status == AHR_OK)
///             {
///                 AHR_Response response = AHR_CreateResponse();
///                 const AHR_Status response_status = AHR_GetResponse(request, response);
///                 printf("Status Code: %i\n", response_status);
///             }
///

#ifndef __ASYNC_HTTP_REQUESTS_H__
#define __ASYNC_HTTP_REQUESTS_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <async_http_requests/logging.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

#define AHR_HEADERENTRY_NAME_LEN 256
#define AHR_HEADERENTRY_VALUE_LEN (4096 - (AHR_HEADERENTRY_NAME_LEN))

#define AHR_HEADER_NMAX 256

//
// --------------------------------------------------------------------------------------------------------------------
//

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

struct AHR_HttpRequest;
typedef struct AHR_HttpRequest* AHR_HttpRequest_t;

struct AHR_HttpResponse;
typedef struct AHR_HttpResponse* AHR_HttpResponse_t;

typedef enum
{
    AHR_OK,
    AHR_UNKNOWN_ERROR
} AHR_Status_t;
//
// --------------------------------------------------------------------------------------------------------------------
//
///
/// \brief  Getter for an internal Handle.
///
void* AHR_RequestHandle(AHR_HttpRequest_t request);
///
/// \brief  Create a Response Object.
///         Each Object has to be destroyed with a call to AHR_DestroyResponse.
///
/// \example    AHR_HttpResponse r = AHR_CreateResponse();
///             ...
///             AHR_DestroyResponse(&r);
///             assert(NULL == r);
///
AHR_HttpResponse_t AHR_CreateResponse(void);
///
/// \brief  Create a Request Object.
///         Each Object has to be destroyed with a call to AHR_DestroyRequest.
///
/// \example    AHR_HttpRequest r = AHR_CreateRequest();
///             ...
///             AHR_DestroyRequest(&r);
///             assert(NULL == r);
///
AHR_HttpRequest_t AHR_CreateRequest(void);
///
/// \brief  Destroys a Response Object.
///
void AHR_DestroyResponse(AHR_HttpResponse_t *response);
///
/// \brief  Destroys a Request Object.
///
void AHR_DestroyRequest(AHR_HttpRequest_t *request);

void AHR_RequestSetLogger(AHR_HttpRequest_t request, AHR_Logger_t logger);
void AHR_ResponseSetLogger(AHR_HttpResponse_t response, AHR_Logger_t logger);

///
/// \brief  Set the given HTTP Header for the Request Object.
/// \param[in, out] request - Request Object.
/// \param[in] headers - A 2d char* Array that holds NULL-terminated c-strings.
///                      The first dimension must be terminated with a NULL pointer.
///                      Each Element by itselve must be a NULL-terminated c-string.
///                      Each Header is constructed by "name:value" which the user has to do.
///
/// \example    const char* const headers[] = {
///                 "Content-Type: application/json",
///                 "Authorization: Bearer ...",
///                 NULL
///             };
///             AHR_RequestSetHeader(request, headers);
///             ...
///
void AHR_RequestSetHeader(AHR_HttpRequest_t request, const AHR_Header_t *header);
void AHR_RequestGetHeader(AHR_HttpRequest_t request, AHR_Header_t *header);
///
/// \brief  Set the HTTP POST Method for this Object.
///         This Function appends 1 additional Header.
///         If there is no space for an additional Header, the last header if overridden.
///         See AHR_HEADER_NMAX for the maximum number of possible Entries.
///
void AHR_Post(AHR_HttpRequest_t request, const char *url, const char *body, AHR_HttpResponse_t response);
///
/// \brief  Set the HTTP PUT Method for this Object.
///
void AHR_Put(AHR_HttpRequest_t request, const char *url, const char *body, AHR_HttpResponse_t response);
///
/// \brief  Set the HTTP GET Method for this Object.
///
void AHR_Get(AHR_HttpRequest_t request, const char *url, AHR_HttpResponse_t response);
///
/// \brief  Set the HTTP DELETE Method for this Object.
///
void AHR_Delete(AHR_HttpRequest_t request, const char *url, AHR_HttpResponse_t response);
///
/// \brief  Get the HTTP Reponse Body of this Object.
///         The Pointer is valid until the Object is destroyed with a call to AHR_DestroyResponse(),
///         after this call it is forbidden to use this pointer.
/// \returns const char* - A NULL-terminated c-string.
///
const char* AHR_ResponseBody(const AHR_HttpResponse_t response);
///
/// \brief  Get the Response Headers for the given Object.
///
void AHR_ResponseHeader(const AHR_HttpResponse_t response, AHR_Header_t *info);
///
/// \brief  Get the HTTP Status Code for the given Object.
/// \returns long - On Success the value will be positive and contains a valid HTTP Status Code.
///                 If the Response is empty or on internal failure on the Client side this value is negative.
///
long AHR_ResponseStatusCode(const AHR_HttpResponse_t response);
///
/// \brief  Start a request. 
///         This function does not block. The request was started but is not
///         neccessarily executed yet.
///
/// \param[in, out] request - A Request-Object.
///
/// \returns    AHR_Status_t
///
AHR_Status_t AHR_MakeRequest(AHR_HttpRequest_t request, AHR_HttpResponse_t response); 
///
/// \brief  Get a unique Id which identifies this response object.
///
void* AHR_ResponseUUID(const AHR_HttpResponse_t response);
///
/// \brief  Get a unique Id which identifies this request object.
///
void* AHR_RequestUUID(const AHR_HttpRequest_t request);
//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
