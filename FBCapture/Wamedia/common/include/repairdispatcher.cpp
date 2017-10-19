#include "repairdispatcher.h"

namespace libwamediacommon
{

CRepairRequestDispatcher::CRepairRequestDispatcher()
  : m_pProblemReportingCB(NULL)
  , m_pProblemReportingCBToken(NULL)
{

}
CRepairRequestDispatcher::~CRepairRequestDispatcher()
{

}

void
CRepairRequestDispatcher::setCurrentThreadCheckAndRepairCB(PROBLEM_REPORTING_CALLBACK pProblemReportingCB,
                                                           void* pProblemReportingCBToken)
{
  m_pProblemReportingCB      = pProblemReportingCB;
  m_pProblemReportingCBToken = pProblemReportingCBToken;
}

void CRepairRequestDispatcher::dispatchProblemReport(eMP4_FORMAT_PROBLEM_TYPE formatProblemType, uint32_t nTrackIndex)
{
  if(NULL == m_pProblemReportingCB)
    return;

  CMp4ProblemInfo pi;
  pi.problemType = formatProblemType;
  pi.nTrackIndex = nTrackIndex;
  m_pProblemReportingCB(pi, m_pProblemReportingCBToken);
}

void CRepairRequestDispatcher::dispatchSimpleReplacementProblemReport(eMP4_FORMAT_PROBLEM_TYPE formatProblemType,
                                              uint32_t nTrackIndex,
                                              uint32_t nDesiredNewValue)
{
  if(NULL == m_pProblemReportingCB)
    return;

  CMp4ProblemInfo pi;
  pi.problemType      = formatProblemType;
  pi.nTrackIndex      = nTrackIndex;
  pi.nDesiredNewValue = nDesiredNewValue;
  m_pProblemReportingCB(pi, m_pProblemReportingCBToken);
}

}; // namespace libwamediacommon
