#pragma once
#include "mp4analysisreport.h"
#include "mp4videorotationmodes.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

typedef enum
{
   ERRORS_TOLERANCE_LEVEL_NO_TOLERANCE = 0,
   ERRORS_TOLERANCE_LEVEL_SMALL_DURATION_ERRORS,
   ERRORS_TOLERANCE_LEVEL_SURGERY_CORRIGIBLE,
   ERRORS_TOLERANCE_LEVEL_TEMPORARY_IGNORE_KNOWN_PROBLEMS
} eERRORS_TOLERANCE_LEVEL;

uint32_t verifyMP4FileIntegrity(const char* pStrInputFilename,
                            MP4_FILE_QUICK_INFO* pQuickInfo,
                            eERRORS_TOLERANCE_LEVEL nErrorsToleranceLevel,
                            bool bExpertRepairmanInvestigation);

#ifndef DO_NOT_SUPPORT_MPEG_DASH
uint32_t verifyFragmentedMP4FileIntegrity(const char* pStrInputFilename,
                            MP4_FILE_QUICK_INFO* pQuickInfo,
                            eERRORS_TOLERANCE_LEVEL nErrorsToleranceLevel,
                            bool bExpertRepairmanInvestigation);
#endif // DO_NOT_SUPPORT_MPEG_DASH

typedef void* HSTRMCHK;

HSTRMCHK openMP4StreamChecker(const char* pStrInputFilename,
                              uint64_t nStreamedResourceContentLength);
uint32_t verifyMP4StreamBookkeepingIntegrity(HSTRMCHK hStrmChk,
                            MP4_FILE_QUICK_INFO* pQuickInfo,
                            eERRORS_TOLERANCE_LEVEL nErrorsToleranceLevel,
                            bool bLastMDATBoxHasBeenReceived);
uint32_t verifyMP4StreamPayloadIntegrity(HSTRMCHK hStrmChk,
                            MP4_FILE_QUICK_INFO* pQuickInfo,
                            eERRORS_TOLERANCE_LEVEL nErrorsToleranceLevel,
                            bool bLastMDATBoxHasBeenReceived,
                            uint64_t nAvailableStreamBytes);
void closeMP4StreamChecker(HSTRMCHK hStrmChk);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

