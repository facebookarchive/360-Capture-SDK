#pragma once
#include "mp4analysisreport.h"
#include "mp4repairreport.h"
#include "mp4flavoridentifier.h"
#include <stdbool.h>

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

typedef void* HMP4REPAIR;

HMP4REPAIR openMp4Repair(void);
uint32_t preliminaryRepairEstimate(HMP4REPAIR hMp4Repair,
                                   const char* pStrInputFilename,
                                   eMP4_REPAIR_SUMMARY_REPORT* pnRepairSummaryReport,
                                   eMP4_FORMAT_FLAVOR* pnDetectedMP4FormatFlavor,
                                   MP4_FILE_QUICK_INFO* pInputFileQuickInfo,
                                   EDIT_LIST_DURATIONS_INFO* pEditListDurationsInfo);
uint32_t doRepairMp4File(HMP4REPAIR hMp4Repair,
                         const char* pStrOutputFilename,
                         eMP4_REPAIR_SUMMARY_REPORT* pnRepairSummaryReport,
                         MP4_FILE_QUICK_INFO* pOutputFileQuickInfo);
uint32_t closeMp4Repair(HMP4REPAIR hMp4Repair);

char* suggestOutputFilename(const char* pStrInputFilename, const char* pStrOutputPath);
void deallocateOutputFilename(char** ppStrOutputFilename);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

