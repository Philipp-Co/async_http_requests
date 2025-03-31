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
//          const size_t object = 0;
//          AHR_RequestData request_data;
//          AHR_UserData user_data;
//          AHR_ProcessorPrepareRequest(
//              p,
//              object,
//              &request_data,
//              user_data
//          )
//          AHR_ProcessorGet(
//              p, 
//              object
//          );
//          AHR_MakeRequest(
//              p,
//              object
//          );
//          ...
//
//          AHR_DestroyProcessor(&p);
//

#ifndef __AHR_REQUEST_PROCESSOR_H__
#define __AHR_REQUEST_PROCESSOR_H__

//
// --------------------------------------------------------------------------------------------------------------------
//
#include <async_http_requests/async_http_requests.h>
#include <async_http_requests/logging.h>
#include <async_http_requests/types.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

struct AHR_Processor;
typedef struct AHR_Processor* AHR_Processor_t;

typedef enum
{
    AHR_PROC_OK = 0,
    AHR_PROC_OBJECT_BUSY = 1,
    AHR_PROC_UNKNOWN_OBJECT = 2,
    AHR_PROC_NOT_ENOUGH_MEMORY = 3,
    AHR_PROC_UNKNOWN_ERROR = 4
} AHR_ProcessorStatus_t;

//
// --------------------------------------------------------------------------------------------------------------------
//
///
/// \brief  Create a new AHR_Processor_t Object.  
///
AHR_Processor_t AHR_CreateProcessor(size_t max_objects, AHR_Logger_t logger);
///
/// \brief  Destroy the given Processor-Object.
///
void AHR_DestroyProcessor(AHR_Processor_t *processor);
///
/// \brief  Start this Instance.
///         This Instance will not start working until this Funktion is called.
/// 
/// \returns bool - true if this Instance was started though this call or if it is already running.
///                 false otherwise.
///
bool AHR_ProcessorStart(AHR_Processor_t processor);
///
/// \brief  Stop this Instance.
///
void AHR_ProcessorStop(AHR_Processor_t processor);
///
/// \brief  Get the Id of a Request/Response Object Pair.
/// 
AHR_Id_t AHR_ProcessorTransactionId(AHR_Processor_t processor, size_t object);
///
/// \brief  Get the Number of Requestobjects which are managed by this Instance.
///         Objects are addressed with Integers 0 <= x < AHR_ProcessorNumberOfRequestObjects().
/// 
/// \param[in] processor - This Instance.
///
/// \returns size_t - Number of managed Objects.
///
size_t AHR_ProcessorNumberOfRequestObjects(const AHR_Processor_t processor);
///
/// \brief  Prepares a Requestobject.
///         If the given Object is currently in use, you can not changes its contents.
///
/// \param[in] object - The Object to change.
/// \param[in] request_data - Url, Header, Body to set for the given Object.
/// \param[in] data - Userdata and Callback which is called in case a Request finishes for hits Object.
/// \returns    AHR_PROC_OK on success.
///             AHR_PROC_UNKNOWN_OBJECT if the given "object" is not managed by this instance.
///             AHR_PROC_OBJECT_BUSY if the object for which this change is requested is currently in use.
/// \pre    NULL != request_data
///         NULL != request_data->url
///
/*
AHR_ProcessorStatus_t AHR_ProcessorPrepareRequest(
    AHR_Processor_t processor,
    size_t object, 
    const AHR_RequestData_t *request_data,
    AHR_UserData_t data
);
*/
///
/// \brief  Set the HTTP Method for the given Object.
///         If the Object is currently in use you can not change it.
///
/// \param[in] processor - The managing Instance.
/// \param[in] object - The Object for which the HTTP Method should be set.
///
AHR_ProcessorStatus_t AHR_ProcessorGet(
    AHR_Processor_t processor, 
    size_t object,
    const AHR_RequestData_t *request_data,
    AHR_UserData_t data
);

AHR_ProcessorStatus_t AHR_ProcessorPost(
    AHR_Processor_t processor, 
    size_t object,
    const AHR_RequestData_t *data, 
    AHR_UserData_t user_data
);
AHR_ProcessorStatus_t AHR_ProcessorPut(
    AHR_Processor_t processor, 
    size_t object,
    const AHR_RequestData_t *data, 
    AHR_UserData_t user_data
);
AHR_ProcessorStatus_t AHR_ProcessorDelete(
    AHR_Processor_t processor, 
    size_t object,
    const AHR_RequestData_t *data, 
    AHR_UserData_t user_data
);

///
/// \brief  Make a Request.
///         Requestparameter where previously configured for the given Object through
///         calls to AHR_ProcessorPrepareRequest() and AHR_ProcessorGet/Post/Put/Delete().
///         You can not make a request with an Object which is busy.
///
/// \param[in] processor - The managing Instance.
/// \param[in] object - The Requestobject.
///
/// \returns    AHR_PROC_OK on success.
///             AHR_PROC_UNKNOWN_OBJECT if the given Object is not known to this Instance. 
///             AHR_PROC_OBJECT_BUSY if the Object to be used is currently busy.
///
///
AHR_ProcessorStatus_t AHR_ProcessorMakeRequest(AHR_Processor_t processor, size_t object);

//
// --------------------------------------------------------------------------------------------------------------------
//


#endif
