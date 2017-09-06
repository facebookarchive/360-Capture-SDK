#pragma once
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef __cplusplus
namespace libmp4operations
{
extern "C"
{
#endif // __cplusplus

const char* errorCodeToString(uint32_t errorCode);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

