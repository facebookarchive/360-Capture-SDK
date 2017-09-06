#include "loggingcallcenter.h"
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "threadlocal.h"
#include "commonDefinitions.h"
using namespace std;

namespace libwamediacommon
{

HTHRDLOGGER attachCurrentThreadToLogStream(EXTERNAL_LOG_CALLBACK pLogCB,
                                           void* pLogCBToken,
                                           PROGRESS_INDICATION_CALLBACK pProgressCB,
                                           void* pProgressCBToken,
                                           IDENTIFY_LOGGER_THREAD_CALLBACK pLoggerCB,
                                           void* pLoggerCBToken)
{
  if((NULL == pLogCB) && (NULL == pProgressCB)) // one or the other is OK
    return NULL;

  HTHRDLOGGER hRetValue = createThreadLocalLogger(pLogCB, pLogCBToken, pProgressCB, pProgressCBToken);
  if(NULL == hRetValue)
    return NULL;

  CLoggingCallCenter::GetInstance().setCurrentThreadLoggerCB(pLoggerCB, pLoggerCBToken);
  return hRetValue;
}

void detachCurrentThreadFromLogStream(HTHRDLOGGER hThrdLogger)
{
  deleteThreadLocalLogger(hThrdLogger);
}

void DEBUG(const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_DEBUG, msgBuffer, strlen(msgBuffer));
}

void INFO(const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_INFO, msgBuffer, strlen(msgBuffer));
}

void NOTICE(const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_NOTICE, msgBuffer, strlen(msgBuffer));
}

void WARNING(const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_WARNING, msgBuffer, strlen(msgBuffer));
}

void ERROR(const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_ERROR, msgBuffer, strlen(msgBuffer));
}

void DEBUG_R(uint32_t nSourceFileLineNumber, const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchRepeatedMessage(LOG_LEVEL_DEBUG, nSourceFileLineNumber, msgBuffer, strlen(msgBuffer));
}

void INFO_R(uint32_t nSourceFileLineNumber, const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchRepeatedMessage(LOG_LEVEL_INFO, nSourceFileLineNumber, msgBuffer, strlen(msgBuffer));
}

void NOTICE_R(uint32_t nSourceFileLineNumber, const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchRepeatedMessage(LOG_LEVEL_NOTICE, nSourceFileLineNumber, msgBuffer, strlen(msgBuffer));
}

void WARNING_R(uint32_t nSourceFileLineNumber, const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchRepeatedMessage(LOG_LEVEL_WARNING, nSourceFileLineNumber, msgBuffer, strlen(msgBuffer));
}

void ERROR_R(uint32_t nSourceFileLineNumber, const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system error (n = %d)", n);

  CLoggingCallCenter::GetInstance().dispatchRepeatedMessage(LOG_LEVEL_ERROR, nSourceFileLineNumber, msgBuffer, strlen(msgBuffer));
}

void REPORT_COMPLETION_PERCENT(uint32_t nPercent)
{
  CLoggingCallCenter::GetInstance().reportCompletionPercent(nPercent);
}

CLoggingCallCenter CLoggingCallCenter::s_logger;

CLoggingCallCenter::CLoggingCallCenter()
  : m_pIdentifyLoggerThreadCB(NULL)
  , m_pIdentifyLoggerThreadCBToken(NULL)
{

}

CLoggingCallCenter &
CLoggingCallCenter::GetInstance(void)
{
  return s_logger;
}

void
CLoggingCallCenter::setCurrentThreadLoggerCB(IDENTIFY_LOGGER_THREAD_CALLBACK pLoggerCB,
                                             void* pLoggerCBToken)
{
  m_pIdentifyLoggerThreadCB      = pLoggerCB;
  m_pIdentifyLoggerThreadCBToken = pLoggerCBToken;
}

void
CLoggingCallCenter::dispatchMessage(eLOG_LEVEL logLevel, const char* pStrMessage, uint32_t nMsgLength)
{
  if(NULL == m_pIdentifyLoggerThreadCB)
    return;

  CLogStreamMonitor* pLogStreamMonitor = (CLogStreamMonitor*)m_pIdentifyLoggerThreadCB(m_pIdentifyLoggerThreadCBToken);
  if(pLogStreamMonitor)
    pLogStreamMonitor->forwardMessage(logLevel, pStrMessage, nMsgLength);
}

void
CLoggingCallCenter::dispatchRepeatedMessage(eLOG_LEVEL logLevel, uint32_t nSourceFileLineNumber, const char* pStrMessage, uint32_t nMsgLength)
{
  if(NULL == m_pIdentifyLoggerThreadCB)
    return;

  CLogStreamMonitor* pLogStreamMonitor = (CLogStreamMonitor*)m_pIdentifyLoggerThreadCB(m_pIdentifyLoggerThreadCBToken);
  if(pLogStreamMonitor)
    pLogStreamMonitor->forwardRepeatedMessage(logLevel, nSourceFileLineNumber, pStrMessage, nMsgLength);
}

void
CLoggingCallCenter::reportCompletionPercent(uint32_t nPercent)
{
  if(NULL == m_pIdentifyLoggerThreadCB)
    return;

  CLogStreamMonitor* pLogStreamMonitor = (CLogStreamMonitor*)m_pIdentifyLoggerThreadCB(m_pIdentifyLoggerThreadCBToken);
  if(pLogStreamMonitor)
    pLogStreamMonitor->reportCompletionPercent(nPercent);
}


}; // namespace libwamediacommon
