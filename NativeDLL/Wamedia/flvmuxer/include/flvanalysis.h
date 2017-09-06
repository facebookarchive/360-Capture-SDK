#pragma once
#include "flvanalysisreport.h"

#ifdef __cplusplus
namespace libflvoperations
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

uint32_t verifyFLVFileIntegrity(const char* pStrInputFilename,
                                FLV_FILE_QUICK_INFO* pQuickInfo,
                                eERRORS_TOLERANCE_LEVEL nErrorsToleranceLevel,
                                bool bPrintScriptTagContents);

typedef void* HFLVSTRMCHK;

HFLVSTRMCHK openFLVStreamChecker(const char* pStrInputFilename,
                                 uint64_t nStreamedResourceContentLength);
uint32_t verifyFLVStreamBookkeepingIntegrity(HFLVSTRMCHK hFLVStrmChk,
                                 FLV_FILE_QUICK_INFO* pQuickInfo,
                                 eERRORS_TOLERANCE_LEVEL nErrorsToleranceLevel,
                                 bool bLastMDATBoxHasBeenReceived);
uint32_t verifyFLVStreamPayloadIntegrity(HFLVSTRMCHK hFLVStrmChk,
                                 FLV_FILE_QUICK_INFO* pQuickInfo,
                                 eERRORS_TOLERANCE_LEVEL nErrorsToleranceLevel,
                                 bool bLastMDATBoxHasBeenReceived,
                                 uint64_t nAvailableStreamBytes);
void closeFLVStreamChecker(HFLVSTRMCHK hFLVStrmChk);

#ifdef __cplusplus
}

}; // namespace libflvoperations
#endif // __cplusplus

