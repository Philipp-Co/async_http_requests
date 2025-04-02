#ifndef __AHR_CURL_H__
#define __AHR_CURL_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

#include <async_http_requests/async_http_requests.h>
#include <stdbool.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

typedef size_t (*AHR_WriteCallback_t)(char *data, size_t size, size_t nmemb, void *clientp);
typedef size_t (*AHR_HeaderCallback_t)(char *buffer, size_t size, size_t nitems, void *userdata);

struct AHR_CurlM;
typedef struct AHR_CurlM* AHR_CurlM_t;

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_CurlM_t AHR_CurlGetHandle(AHR_Curl_t handle);

AHR_Curl_t AHR_CurlEasyInit(
    AHR_WriteCallback_t write_callback,
    AHR_HeaderCallback_t header_callback
);
void AHR_CurlEasyCleanUp(AHR_Curl_t handle);

void AHR_CurlSetHeader(AHR_Curl_t handle, const AHR_Header_t *header);
void AHR_CurlEasySetUrl(AHR_Curl_t handle, const char *url);

bool AHR_CurlEasyPerform(AHR_Curl_t handle);
long AHR_CurlEasyStatusCode(AHR_Curl_t handle);

void AHR_CurlSetCallbackUserData(
    AHR_Curl_t handle, 
    void *write_callback_user_data,
    void *header_callback_user_data
);

void AHR_CurlSetHttpMethodGet(AHR_Curl_t handle);
void AHR_CurlSetHttpMethodPost(AHR_Curl_t handle, const char *body);
void AHR_CurlSetHttpMethodPut(AHR_Curl_t handle, const char *body);
void AHR_CurlSetHttpMethodDelete(AHR_Curl_t handle);

int AHR_CurlWriteError(void);
int AHR_CurlReadError(void);

//
// --------------------------------------------------------------------------------------------------------------------
//

typedef void (*AHR_CurlMultiInfoReadSuccessCallback_t)(void *arg, AHR_Curl_t handle);
typedef void (*AHR_CurlMultiInfoReadErrorCallback_t)(void *arg, AHR_Curl_t handle, size_t error_code);
typedef struct
{
    void *data;
    AHR_CurlMultiInfoReadSuccessCallback_t on_success;
    AHR_CurlMultiInfoReadErrorCallback_t on_error;
} AHR_CurlMultiInfoReadData_t;

bool AHR_CurlMultiAddHandle(AHR_CurlM_t handle, AHR_Curl_t ehandle);
void AHR_CurlMultiRemoveHandle(
    AHR_CurlM_t handle,
    AHR_Curl_t ehandle
);
AHR_CurlM_t AHR_CurlMultiInit(void);
void AHR_CurlMultiCleanUp(AHR_CurlM_t handle);
bool AHR_CurlMultiInfoRead(
    AHR_CurlM_t handle,
    AHR_CurlMultiInfoReadData_t callback 
);
bool AHR_CurlMultiPerform(AHR_CurlM_t handle, int *running_handles);
void AHR_CurlMultiPoll(AHR_CurlM_t handle);
void AHR_CurlMultiWeakUp(AHR_CurlM_t handle);

//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
