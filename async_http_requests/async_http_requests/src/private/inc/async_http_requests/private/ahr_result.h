#ifndef __AHR_RESULT_H__
#define __AHR_RESULT_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

#include <async_http_requests/private/ahr_async_http_requests.h>
#include <async_http_requests/ahr_types.h>

#include <stdatomic.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

typedef struct
{
    AHR_HttpRequest_t request;
    AHR_HttpResponse_t response;

    AHR_RequestData_t request_data;
    AHR_UserData_t user_data;

    atomic_int busy;
} AHR_Result_t;

typedef struct
{
    atomic_int *current_state_of_result;
    AHR_Result_t *results;
    size_t nresults;
} AHR_ResultStore_t;


AHR_ResultStore_t AHR_CreateResultStore(size_t max_size);
void AHR_DestroyResultStore(AHR_ResultStore_t *store);
AHR_Result_t* AHR_ResultStoreGetResult(AHR_ResultStore_t *store, size_t index);
size_t AHR_ResultStoreSize(const AHR_ResultStore_t *store);
size_t AHR_ResultStoreObjectIndex(const AHR_ResultStore_t *store, const AHR_Result_t *result);

//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
