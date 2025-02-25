//
// \brief   This implements the HTTP Request Processor. The Processor will handle the Applications HTTP Requests asynchronously 
//          and will inform the application via a given Callback.
//
// \example application
//          
//          static void MyCallback(AHR_Response_t response);
//
//          AHR_Processor_t p = AHR_CreateProcessor();
//
//          ...
//          AHR_ProcessorGet(p, "www.google.de", MyCallback);
//          ...
//
//          AHR_DestroyProcessor(&p);
//

#ifndef __AHR_REQUEST_PROCESSOR_H__
#define __AHR_REQUEST_PROCESSOR_H__

//
// --------------------------------------------------------------------------------------------------------------------
//
#include "async_http_requests/async_http_requests.h"
#include <async_http_requests/logging.h>

#include <stdlib.h>
#include <stdbool.h>
//
// --------------------------------------------------------------------------------------------------------------------
//

typedef void* AHR_Id_t;

struct AHR_Processor;
typedef struct AHR_Processor* AHR_Processor_t;

struct AHR_Result;
typedef struct AHR_Result* AHR_Result_t;

typedef void (*AHR_ResponseReadyCallback)(void *user_data, AHR_HttpResponse_t response);

typedef struct
{
    void *data;
    AHR_ResponseReadyCallback on_success;
} AHR_UserData_t;

typedef struct
{ 
    AHR_Header_t header;
    char *url;
    char *body;
} AHR_RequestData_t;

//
// --------------------------------------------------------------------------------------------------------------------
//
///
/// \brief  Create a new AHR_Processor_t Object.  
///
AHR_Processor_t AHR_CreateProcessor(AHR_Logger_t logger);
///
/// \brief  Destroy the given Processor-Object.
///
void AHR_DestroyProcessor(AHR_Processor_t *processor);
bool AHR_ProcessorStart(AHR_Processor_t processor);
void AHR_ProcessorStop(AHR_Processor_t processor);
///
/// \brief  Execute a GET-Request.
///
AHR_Id_t AHR_ProcessorGet(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data);
AHR_Id_t AHR_ProcessorPost(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data);
AHR_Id_t AHR_ProcessorPut(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data);
AHR_Id_t AHR_ProcessorDelete(AHR_Processor_t processor, const AHR_RequestData_t *data, AHR_UserData_t user_data);
//
// --------------------------------------------------------------------------------------------------------------------
//


#endif
