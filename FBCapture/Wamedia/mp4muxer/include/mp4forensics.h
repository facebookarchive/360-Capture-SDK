#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

uint32_t createForensicEvidence(uint32_t nReportedError,
	                            const char* pStrInputFilename,
	                            char** ppStrOutputFilename);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

