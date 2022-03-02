#ifndef _PHIDGET_LOG_HELPERS
#define _PHIDGET_LOG_HELPERS

#include "phidget22extra_int.h"

#define PSUPLS "proj"

#ifdef NDEBUG
#define suplogdebug(...)
#define suplogcrit(...) PhidgetLog_loge(NULL, 0, __func__, PSUPLS, PHIDGET_LOG_CRITICAL, __VA_ARGS__)
#define suplogerr(...) PhidgetLog_loge(NULL, 0, __func__, PSUPLS, PHIDGET_LOG_ERROR, __VA_ARGS__)
#define suplogwarn(...) PhidgetLog_loge(NULL, 0, __func__, PSUPLS, PHIDGET_LOG_WARNING, __VA_ARGS__)
#define suploginfo(...) PhidgetLog_loge(NULL, 0, __func__, PSUPLS, PHIDGET_LOG_INFO, __VA_ARGS__)
#define suplogverbose(...) PhidgetLog_loge(NULL, 0, __func__, PSUPLS, PHIDGET_LOG_VERBOSE, __VA_ARGS__)
#else
#define suplogcrit(...) PhidgetLog_loge(__FILE__, __LINE__, __func__, PSUPLS, PHIDGET_LOG_CRITICAL, __VA_ARGS__)
#define suplogerr(...) PhidgetLog_loge(__FILE__, __LINE__, __func__, PSUPLS, PHIDGET_LOG_ERROR, __VA_ARGS__)
#define suplogwarn(...) PhidgetLog_loge(__FILE__, __LINE__, __func__, PSUPLS, PHIDGET_LOG_WARNING, __VA_ARGS__)
#define suploginfo(...) PhidgetLog_loge(__FILE__, __LINE__, __func__, PSUPLS, PHIDGET_LOG_INFO, __VA_ARGS__)
#define suplogdebug(...) PhidgetLog_loge(__FILE__, __LINE__, __func__, PSUPLS, PHIDGET_LOG_DEBUG, __VA_ARGS__)
#define suplogverbose(...) PhidgetLog_loge(__FILE__, __LINE__, __func__, PSUPLS, PHIDGET_LOG_VERBOSE, __VA_ARGS__)
#endif /* NDEBUG */

#endif /* _PHIDGET_LOG_HELPERS */
