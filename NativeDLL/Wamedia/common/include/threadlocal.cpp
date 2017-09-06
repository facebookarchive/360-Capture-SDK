#include "threadlocal.h"
#include "commonDefinitions.h"

namespace libwamediacommon
{

HTHRDLOGGER createThreadLocalLogger(EXTERNAL_LOG_CALLBACK pLogCB,
                                    void* pLogCBToken,
                                    PROGRESS_INDICATION_CALLBACK pProgressCB,
                                    void* pProgressCBToken)
{
  CLogStreamMonitor* pLogStreamMonitor =
    new (std::nothrow) CLogStreamMonitor(pLogCB,
                                         pLogCBToken,
                                         pProgressCB,
                                         pProgressCBToken);
  return (HTHRDLOGGER)pLogStreamMonitor;
}

void deleteThreadLocalLogger(HTHRDLOGGER hThrdLogger)
{
  CLogStreamMonitor* pLogStreamMonitor = (CLogStreamMonitor*)hThrdLogger;
  SAFE_DELETE(pLogStreamMonitor);
}

HTHRDREPAIR createThreadLocalRepairRequestDispatcher()
{
  CRepairRequestDispatcher* pRepairDispatcher =
    new (std::nothrow) CRepairRequestDispatcher;
  return (HTHRDREPAIR)pRepairDispatcher;
}

void deleteThreadLocalRepairRequestDispatcher(HTHRDREPAIR hThrdRepair)
{
  CRepairRequestDispatcher* pRepairDispatcher = (CRepairRequestDispatcher*)hThrdRepair;
  SAFE_DELETE(pRepairDispatcher);
}

}; // namespace libwamediacommon
