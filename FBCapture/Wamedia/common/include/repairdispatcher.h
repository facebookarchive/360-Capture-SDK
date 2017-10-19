#pragma once
#include "commonDefinitions.h"
#include "mp4repairprovider.h"

namespace libwamediacommon
{

class CRepairRequestDispatcher
{
public:
  CRepairRequestDispatcher();
  virtual ~CRepairRequestDispatcher();

  void setCurrentThreadCheckAndRepairCB(PROBLEM_REPORTING_CALLBACK pProblemReportingCB,
                                        void* pProblemReportingCBToken);
  void dispatchProblemReport(eMP4_FORMAT_PROBLEM_TYPE formatProblemType, uint32_t nTrackIndex);
  void dispatchSimpleReplacementProblemReport(eMP4_FORMAT_PROBLEM_TYPE formatProblemType,
                                              uint32_t nTrackIndex,
                                              uint32_t nDesiredNewValue);

private:
  PROBLEM_REPORTING_CALLBACK m_pProblemReportingCB;
  void*                      m_pProblemReportingCBToken;
};

}; // namespace libwamediacommon
