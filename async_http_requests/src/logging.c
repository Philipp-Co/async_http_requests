
//
// --------------------------------------------------------------------------------------------------------------------
//

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>

#include <async_http_requests/logging.h>
#include <sys/_types/_va_list.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

struct AHR_Logger
{
    pthread_mutex_t mutex;
    void *arg;
    AHR_LogInfo_t info;
    AHR_LogWarning_t warning;
    AHR_LogError_t error;
    size_t loglevel;
};

//
// --------------------------------------------------------------------------------------------------------------------
//

AHR_Logger_t AHR_CreateLogger(
    void *arg,
    AHR_LogInfo_t info,
    AHR_LogWarning_t warning,
    AHR_LogError_t error
)
{
    assert(info != NULL);
    assert(warning != NULL);
    assert(error != NULL);

    struct AHR_Logger *logger = malloc(sizeof(struct AHR_Logger));    

    pthread_mutex_init(&logger->mutex, NULL);

    logger->arg = arg;
    logger->info = info;
    logger->warning = warning;
    logger->error = error;

    logger->loglevel = AHR_LOGLEVEL_INFO;

    return logger;
}

void AHR_DestroyLogger(AHR_Logger_t *logger)
{
    free(*logger);
    *logger = NULL;
}

void AHR_LoggerSetLoglevel(AHR_Logger_t logger, size_t loglevel)
{
    if(loglevel >= AHR_LOGLEVEL_ERROR && loglevel < AHR_LOGLEVEL_INFO)
    {
        logger->loglevel = loglevel;
    }
}

void AHR_LogInfo(AHR_Logger_t logger, const char *msg)
{
    if(logger && logger->loglevel >= AHR_LOGLEVEL_INFO)
    {
        logger->info(logger->arg, msg);
    }
}

void AHR_LogWarning(AHR_Logger_t logger, const char *msg)
{
    if(logger && logger->loglevel >= AHR_LOGLEVEL_WARNING)
        logger->warning(logger->arg, msg);
}

void AHR_LogError(AHR_Logger_t logger, const char *msg)
{
    if(logger && logger->loglevel >= AHR_LOGLEVEL_ERROR)
        logger->error(logger->arg, msg);
}
//
// --------------------------------------------------------------------------------------------------------------------
//
