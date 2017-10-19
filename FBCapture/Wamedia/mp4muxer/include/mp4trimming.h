#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

#define MEGABYTE (1024*1024)
#define DEFAULT_TRIMMING_BYTE_LENGTH (16*MEGABYTE)

uint32_t trimMp4File(const char* pStrInputFilename, 
                     const char* pStrOutputFilename,
                     uint32_t nByteLimit = DEFAULT_TRIMMING_BYTE_LENGTH,
                     bool bExpertRepairmanInspection = false);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

