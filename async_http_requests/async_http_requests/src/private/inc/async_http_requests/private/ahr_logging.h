///
/// \brief  This Module implements logging.
///
/// \example    AHR_Logger_t logger = AHR_CreateLogger(...);
///             ...
///             AHR_LogInfo(logger, "Success!");
///             ...
///             AHR_DestroyLogger(&logger);
///             assert(NULL == logger);
///
#ifndef __AHR_LOGGING_H__
#define __AHR_LOGGING_H__

//
// --------------------------------------------------------------------------------------------------------------------
//

#include <async_http_requests/ahr_types.h>

#include <stddef.h>

//
// --------------------------------------------------------------------------------------------------------------------
//

#define AHR_LOGLEVEL_INFO 2U
#define AHR_LOGLEVEL_WARNING 1U
#define AHR_LOGLEVEL_ERROR 0U

//
// --------------------------------------------------------------------------------------------------------------------
//

typedef void (*AHR_LogInfo_t)(void *arg, const char *str);
typedef void (*AHR_LogWarning_t)(void *arg, const char *str);
typedef void (*AHR_LogError_t)(void *arg, const char *str);

//
// --------------------------------------------------------------------------------------------------------------------
//
///
/// \brief  Create a Logger object. Each Logger object created with this function must be destroyed 
///         with a call to AHR_DestroyLogger() in case it is no longer needed.
/// \param[in] arg      - User data, passed to each call of "info", "warning" and "error".
/// \param[in] info     - Info Callback.
/// \param[in] warning  - Warning Callback.
/// \param[in] error    - Error Callback.
///
AHR_Logger_t AHR_CreateLogger(
    void *arg,
    AHR_LogInfo_t info,
    AHR_LogWarning_t warning,
    AHR_LogError_t error
);
///
/// \brief  Destroy Logger object.
/// \post   After a call to this function *logger == NULL.
///
void AHR_DestroyLogger(AHR_Logger_t *logger);
///
/// \brief  Infolog.
///
void AHR_LogInfo(AHR_Logger_t logger, const char *msg);
///
/// \brief  Warninglog.
///
void AHR_LogWarning(AHR_Logger_t logger, const char *msg);
///
/// \brief  Errorlog.
///
void AHR_LogError(AHR_Logger_t logger, const char *msg);
///
/// \brief  Set the Loglevel.
///         Use AHR_LOGLEVEL_* Definitions.
///
void AHR_LoggerSetLoglevel(AHR_Logger_t logger, size_t loglevel);
//
// --------------------------------------------------------------------------------------------------------------------
//

#endif
