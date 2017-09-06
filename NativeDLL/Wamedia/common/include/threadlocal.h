#pragma once
#include "logsrecipient.h"
#include "mp4repairprovider.h"
#include "logstreammonitor.h"
#include "repairdispatcher.h"

#ifdef __cplusplus
namespace libwamediacommon
{

extern "C"
{
#endif // __cplusplus

HTHRDLOGGER createThreadLocalLogger(EXTERNAL_LOG_CALLBACK pLogCB,
                                    void* pLogCBToken,
                                    PROGRESS_INDICATION_CALLBACK pProgressCB,
                                    void* pProgressCBToken);
void deleteThreadLocalLogger(HTHRDLOGGER hThrdLogger);

HTHRDREPAIR createThreadLocalRepairRequestDispatcher();
void deleteThreadLocalRepairRequestDispatcher(HTHRDREPAIR hThrdRepair);

#ifdef __cplusplus
}

}; // namespace libwamediacommon
#endif // __cplusplus

