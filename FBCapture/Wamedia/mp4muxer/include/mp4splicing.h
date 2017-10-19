#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

typedef void* HMP4SPLICER;

HMP4SPLICER openMp4Splicer(void);
uint32_t spliceMp4Files(HMP4SPLICER hSplicer, uint32_t nArgc, char* ppStrArgv[]);
uint32_t closeMp4Splicer(HMP4SPLICER hSplicer);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

