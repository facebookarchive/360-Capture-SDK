#pragma once
#include "mp4errors.h"
#include "mp4videorotationmodes.h"

#ifdef __cplusplus
namespace libmp4operations
{
extern "C"
{
#endif // __cplusplus

uint32_t rotateMp4File(const char* pStrInputFilename, eVideoRotationMode nVideoRotationMode);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

