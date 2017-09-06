#pragma once
#include "logsrecipient.h"
#include "logssender.h"
#include "logstreammonitor.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

namespace libwamediacommon
{

class CLoggingCallCenter
{
public:
  virtual ~CLoggingCallCenter(){};
  static CLoggingCallCenter & GetInstance();

public:
  void setCurrentThreadLoggerCB(IDENTIFY_LOGGER_THREAD_CALLBACK pIdentifyLoggerThreadCB,
                                void* pIdentifyLoggerThreadCBToken);
  void dispatchMessage(eLOG_LEVEL level, const char* strMessage, uint32_t nMsgLength);
  void dispatchRepeatedMessage(eLOG_LEVEL level, uint32_t nSourceFileLineNumber, const char* pStrMessage, uint32_t nMsgLength);
  
  // support for progress indication
  void reportCompletionPercent(uint32_t nPercent);

private:
  CLoggingCallCenter();
  CLoggingCallCenter(CLoggingCallCenter const &){};
  CLoggingCallCenter operator=(CLoggingCallCenter const &);
  
private:
  static CLoggingCallCenter  s_logger;

  IDENTIFY_LOGGER_THREAD_CALLBACK m_pIdentifyLoggerThreadCB;
  void*                           m_pIdentifyLoggerThreadCBToken;
};

}; // namespace libwamediacommon
