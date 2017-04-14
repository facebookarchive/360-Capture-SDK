#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{
extern "C"
{
#endif // __cplusplus

uint32_t adaptMp4FileForStreaming(const char* pStrInputFilename, const char* pStrOutputFilename);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

