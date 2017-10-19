#pragma once
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdarg.h>

#ifdef __cplusplus
namespace libwamediacommon
{

extern "C"
{
#endif // __cplusplus

#define MAX_LOG_MSG_STRLEN (2048)

typedef enum
{
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO,
  LOG_LEVEL_NOTICE,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_ERROR
} eLOG_LEVEL;

typedef void* HTHRDLOGGER;
typedef void (*EXTERNAL_LOG_CALLBACK)(eLOG_LEVEL level, const char* strMessage, uint32_t nLength, void *user_data);
typedef void (*PROGRESS_INDICATION_CALLBACK)(uint32_t nPercent, void* user_data);
typedef HTHRDLOGGER (*IDENTIFY_LOGGER_THREAD_CALLBACK)(void* user_data);

HTHRDLOGGER attachCurrentThreadToLogStream(EXTERNAL_LOG_CALLBACK pLogCB,
                                   void* pLogCBToken,
                                   PROGRESS_INDICATION_CALLBACK pProgressCB,
                                   void* pProgressCBToken,
                                   IDENTIFY_LOGGER_THREAD_CALLBACK pLoggerCB,
                                   void* pLoggerCBToken);
void detachCurrentThreadFromLogStream(HTHRDLOGGER hThrdLogger);

#ifdef __cplusplus
}

}; // namespace libwamediacommon
#endif // __cplusplus

