#pragma once
#include <inttypes.h>
#include "mp4repairprovider.h"
#include "mp4repairrequest.h"

#ifdef DEBUG
#undef DEBUG
#endif

namespace libwamediacommon
{
	
class CRepairsCallCenter
{
public:
  ~CRepairsCallCenter();
  static CRepairsCallCenter & GetInstance();
  static void setCheckAndRepairCallback(PROBLEM_REPORTING_CALLBACK cb, void* user_data);

public:
  void setIdentifyRepairThreadCB(IDENTIFY_REPAIR_THREAD_CALLBACK pIdentifyRepairThreadCB,
                                 void* pIdentifyRepairThreadCBToken);
  void setCurrentThreadCheckAndRepairCB(PROBLEM_REPORTING_CALLBACK pRepairReportCB, void* pRepairReportCBToken);
  void dispatchProblemReport(eMP4_FORMAT_PROBLEM_TYPE formatProblemType, uint32_t nTrackIndex);
  void dispatchSimpleReplacementProblemReport(eMP4_FORMAT_PROBLEM_TYPE formatProblemType,
                                             uint32_t nTrackIndex,
                                             uint32_t nDesiredNewValue);
  
private:
  CRepairsCallCenter();
  CRepairsCallCenter(CRepairsCallCenter const &){};
  CRepairsCallCenter operator=(CRepairsCallCenter const &);
  
private:
  static CRepairsCallCenter s_repairCenter;
  IDENTIFY_REPAIR_THREAD_CALLBACK m_pIdentifyRepairThreadCB;
  void*                           m_pIdentifyRepairThreadCBToken;
};

}; // namespace libwamediacommon
