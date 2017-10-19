#include "performancetracer.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "commonDefinitions.h"
using namespace std;

namespace libwamediacommon
{

bool initializePerformanceTracing(TRACING_INFO ti)
{
  return CPerformanceTracer::GetInstance().initializePerformanceTracing(ti);
}

void PERFTRACELOG(const char* fmt, ...)
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

  CPerformanceTracer::GetInstance().logPerfTrace(msgBuffer, strlen(msgBuffer));
}

uint64_t getTracingQueueBytes()
{
  return CPerformanceTracer::GetInstance().getTracingQueueBytes();
}

bool serializeCurrentTracesQueue()
{
  return CPerformanceTracer::GetInstance().serializeCurrentTracesQueue();
}

void terminatePerformanceTracing()
{
  CPerformanceTracer::GetInstance().terminatePerformanceTracing();
}

CPerformanceTracer CPerformanceTracer::s_tracer;

CPerformanceTracer::CPerformanceTracer()
  : m_nTracesReceived(0)
  , m_nTotalQueueBytes(0)
{
  memset(&m_tracingInfo, 0, sizeof(TRACING_INFO));
}

CPerformanceTracer::~CPerformanceTracer()
{
  terminatePerformanceTracing();
}

CPerformanceTracer &
CPerformanceTracer::GetInstance(void)
{
  return s_tracer;
}

bool
CPerformanceTracer::initializePerformanceTracing(TRACING_INFO ti)
{
  if((NULL == ti.pLock) || (NULL == ti.pUnlock))
    return false;

  m_tracingInfo = ti;
  m_timer.startTimekeeping(ti.pGetCurrentTime);
  return true;
}

void
CPerformanceTracer::logPerfTrace(const char* pStrMessage, uint32_t nMsgLength)
{
  if((NULL == m_tracingInfo.pLock) || (NULL == m_tracingInfo.pUnlock))
    return;

  m_tracingInfo.pLock();

  //if(false == m_bInitialized)
  //{
  //  m_bInitialized = startMessageReadingThread();
  //  if(false == m_bInitialized)
  //  {
  //    return false;
  //  }
  //}

  do
  {
    WA_TRACE trace;
    memset(&trace, 0, sizeof(WA_TRACE));

    if(false == m_timer.getCurrentTime(trace.traceTime))
      break;

    uint32_t nStorageBytes = 1 + nMsgLength;
    trace.pStrMessage = new (std::nothrow) char[nStorageBytes];
    if(NULL == trace.pStrMessage)
      break;
    memset(trace.pStrMessage, 0, nStorageBytes);
    strcpy(trace.pStrMessage, pStrMessage);

    m_traceQueue.push_back(trace);
    m_nTracesReceived++;

    m_nTotalQueueBytes += nStorageBytes + sizeof(WA_TRACE);

  } while(false);

  m_tracingInfo.pUnlock();
}

uint64_t
CPerformanceTracer::getTracingQueueBytes()
{
  uint64_t nRetValue;
  if(m_tracingInfo.pLock)
    m_tracingInfo.pLock();

  nRetValue = m_nTotalQueueBytes;

  if(m_tracingInfo.pUnlock)
    m_tracingInfo.pUnlock();

  return nRetValue;
}

bool
CPerformanceTracer::serializeCurrentTracesQueue()
{
  if((NULL == m_tracingInfo.pLock) || (NULL == m_tracingInfo.pUnlock) || (NULL == m_tracingInfo.pFileWrite))
    return false;

  m_tracingInfo.pLock();

  deque<WA_TRACE>::iterator it;
  bool bSuccess = true;
  for(it = m_traceQueue.begin(); it != m_traceQueue.end(); ++it)
  {
    string strRecord = 
      m_timer.getElapsedHrMinSec(it->traceTime) +
      string(": ") + string(it->pStrMessage) +
      string("\n");
    if(false == m_tracingInfo.pFileWrite((char*)strRecord.c_str(), strRecord.length()))
    {
      bSuccess = false;
      break;
    }
  }

  m_tracingInfo.pUnlock();
  return bSuccess;
}

void
CPerformanceTracer::terminatePerformanceTracing()
{
  if(m_tracingInfo.pLock)
    m_tracingInfo.pLock();

  clearTraceQueue();

  if(m_tracingInfo.pUnlock)
    m_tracingInfo.pUnlock();
}

void
CPerformanceTracer::clearTraceQueue()
{
  if(0 == m_traceQueue.size())
    return;

  deque<WA_TRACE>::iterator it;
  for(it = m_traceQueue.begin(); it != m_traceQueue.end(); ++it)
  {
    SAFE_DELETE_ARRAY(it->pStrMessage);
  }
  m_traceQueue.clear();
}

}; // namespace libwamediacommon
