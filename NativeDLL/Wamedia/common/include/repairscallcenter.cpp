#include "repairscallcenter.h"
#include "loggingcallcenter.h"
#include "threadlocal.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
using namespace std;

namespace libwamediacommon
{

HTHRDREPAIR attachCurrentThreadToRepairDispatch(IDENTIFY_REPAIR_THREAD_CALLBACK pCurrentThreadRepairCB,
                                                void* pCurrentThreadRepairCBToken)
{
  if(NULL == pCurrentThreadRepairCB)
    return NULL;

  HTHRDREPAIR hRetValue = createThreadLocalRepairRequestDispatcher();
  if(NULL == hRetValue)
    return NULL;

  CRepairsCallCenter::GetInstance().setIdentifyRepairThreadCB(pCurrentThreadRepairCB, pCurrentThreadRepairCBToken);
  return hRetValue;
}

void detachCurrentThreadFromRepairDispatch(HTHRDREPAIR hThrdRepair)
{
  deleteThreadLocalRepairRequestDispatcher(hThrdRepair);
}

void setCurrentThreadCheckAndRepairCB(PROBLEM_REPORTING_CALLBACK pRepairReportCB,
                                      void* pRepairReportCBToken)
{
  CRepairsCallCenter::GetInstance().setCurrentThreadCheckAndRepairCB(pRepairReportCB, pRepairReportCBToken);
}

void reportProblem(eMP4_FORMAT_PROBLEM_TYPE formatProblemType,
                   uint32_t nTrackIndex,
                   bool bRelaxedCriterion,
                   const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));
  
  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if(n <= 0 || n >= sizeof(msgBuffer))
    snprintf(msgBuffer, sizeof(msgBuffer), "Logging system reportProblems (n = %d)", n);

  if(false == bRelaxedCriterion)
    CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_ERROR, msgBuffer, strlen(msgBuffer));
  else
    CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_WARNING, msgBuffer, strlen(msgBuffer));

  CRepairsCallCenter::GetInstance().dispatchProblemReport(formatProblemType, nTrackIndex);
}

void reportSimpleReplacementProblem(eMP4_FORMAT_PROBLEM_TYPE formatProblemType,
                                    uint32_t nTrackIndex,
                                    uint32_t nDesiredNewValue,
                                    bool bRelaxedCriterion,
                                    const char* fmt, ...)
{
  char msgBuffer[MAX_LOG_MSG_STRLEN + 1];
  uint32_t n;
  va_list args;

  memset(msgBuffer, 0, sizeof(msgBuffer));

  va_start(args, fmt);
  n = vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
  va_end(args);

  if (n <= 0 || n >= sizeof(msgBuffer))
     snprintf(msgBuffer, sizeof(msgBuffer), "Logging system reportProblems (n = %d)", n);

  if(false == bRelaxedCriterion)
    CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_ERROR, msgBuffer, strlen(msgBuffer));
  else
    CLoggingCallCenter::GetInstance().dispatchMessage(LOG_LEVEL_WARNING, msgBuffer, strlen(msgBuffer));

  CRepairsCallCenter::GetInstance().dispatchSimpleReplacementProblemReport(formatProblemType, nTrackIndex, nDesiredNewValue);
}

CRepairsCallCenter CRepairsCallCenter::s_repairCenter;

CRepairsCallCenter::CRepairsCallCenter()
  : m_pIdentifyRepairThreadCB(NULL)
  , m_pIdentifyRepairThreadCBToken(NULL)
{
}

CRepairsCallCenter::~CRepairsCallCenter()
{
}

CRepairsCallCenter& CRepairsCallCenter::GetInstance(void)
{
  return s_repairCenter;
}

void CRepairsCallCenter::setIdentifyRepairThreadCB(IDENTIFY_REPAIR_THREAD_CALLBACK pCurrentThreadRepairCB,
                                                  void* pCurrentThreadRepairCBToken)
{
  m_pIdentifyRepairThreadCB      = pCurrentThreadRepairCB;
  m_pIdentifyRepairThreadCBToken = pCurrentThreadRepairCBToken;
}

void
CRepairsCallCenter::setCurrentThreadCheckAndRepairCB(PROBLEM_REPORTING_CALLBACK pRepairReportCB, void* pRepairReportCBToken)
{
  if(NULL == m_pIdentifyRepairThreadCB)
    return;

  CRepairRequestDispatcher* pRepairRequestDispatcher =
    (CRepairRequestDispatcher*)m_pIdentifyRepairThreadCB(m_pIdentifyRepairThreadCBToken);
  if(pRepairRequestDispatcher)
    pRepairRequestDispatcher->setCurrentThreadCheckAndRepairCB(pRepairReportCB, pRepairReportCBToken);
}

void CRepairsCallCenter::dispatchProblemReport(eMP4_FORMAT_PROBLEM_TYPE formatProblemType, uint32_t nTrackIndex)
{
  if(NULL == m_pIdentifyRepairThreadCB)
    return;

  CRepairRequestDispatcher* pRepairRequestDispatcher =
    (CRepairRequestDispatcher*)m_pIdentifyRepairThreadCB(m_pIdentifyRepairThreadCBToken);
  if(pRepairRequestDispatcher)
    pRepairRequestDispatcher->dispatchProblemReport(formatProblemType, nTrackIndex);
}

void CRepairsCallCenter::dispatchSimpleReplacementProblemReport(eMP4_FORMAT_PROBLEM_TYPE formatProblemType,
                                                                uint32_t nTrackIndex,
                                                                uint32_t nDesiredNewValue)
{
  if(NULL == m_pIdentifyRepairThreadCB)
    return;

  CRepairRequestDispatcher* pRepairRequestDispatcher =
    (CRepairRequestDispatcher*)m_pIdentifyRepairThreadCB(m_pIdentifyRepairThreadCBToken);
  if(pRepairRequestDispatcher)
    pRepairRequestDispatcher->dispatchSimpleReplacementProblemReport(formatProblemType, nTrackIndex, nDesiredNewValue);
}
 
}; // namespace libwamediacommon
