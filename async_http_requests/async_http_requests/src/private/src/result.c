
//
// --------------------------------------------------------------------------------------------------------------------
//

#include "async_http_requests/types.h"
#include <async_http_requests/private/result.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_ResultStore_t AHR_CreateResultStore(size_t max_size)
{
    AHR_ResultStore_t store = {
        .results = malloc(max_size * sizeof(AHR_Result_t)),
        .nresults = max_size
    };
    
    for(size_t i=0;i<max_size;++i)
    {
        atomic_store(&store.results[i].busy, 0);
    }
    return store;
}

void AHR_DestroyResultStore(AHR_ResultStore_t *store)
{
    assert(NULL != store);
    assert(NULL != store->results);

    free((*store).results);
    (*store).nresults = 0;
    (*store).results = NULL;
}

AHR_Result_t* AHR_ResultStoreGetResult(AHR_ResultStore_t *store, size_t index)
{
    assert(((uint64_t)index) < store->nresults);
    return &store->results[index];
}

size_t AHR_ResultStoreSize(const AHR_ResultStore_t *store)
{
    return store->nresults;
}

size_t AHR_ResultStoreObjectIndex(const AHR_ResultStore_t *store, const AHR_Result_t *result)
{
    //
    // Assert that store->results <= result < (store->results + store->nresults)
    //
    assert(result >= store-results);
    assert(result < (store-results + store->nresults));
    //
    // Return Index into Resultarray.
    // Pointerarithmetic!!!
    //
    return (result - store->results);
}

//
// --------------------------------------------------------------------------------------------------------------------
//
