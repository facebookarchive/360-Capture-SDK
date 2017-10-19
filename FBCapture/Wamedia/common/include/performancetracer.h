#pragma once
#include "logsrecipient.h"
#include "perftracelog.h"
#include "performancetime.h"
#include <deque>
using namespace std;

namespace libwamediacommon
{

class CPerformanceTracer
{
public:
  virtual ~CPerformanceTracer();
  static CPerformanceTracer & GetInstance();

public:
  bool initializePerformanceTracing(TRACING_INFO ti);
  void logPerfTrace(const char* pStrMessage, uint32_t nMsgLength);
  bool serializeCurrentTracesQueue();
  uint64_t getTracingQueueBytes();

  void terminatePerformanceTracing();

private:
  CPerformanceTracer();
  CPerformanceTracer(CPerformanceTracer const &){};
  CPerformanceTracer operator=(CPerformanceTracer const &);
  
private:
  static CPerformanceTracer  s_tracer;

private:
  void clearTraceQueue();

private:
  uint32_t         m_nTracesReceived;
  uint64_t         m_nTotalQueueBytes;
  TRACING_INFO     m_tracingInfo;

  typedef struct
  {
    WA_PERF_TIME traceTime;
    char*        pStrMessage;
  } WA_TRACE;

  CPerformanceTime m_timer;
  deque<WA_TRACE>  m_traceQueue;
};

}; // namespace libwamediacommon
