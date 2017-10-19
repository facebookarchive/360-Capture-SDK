#pragma once
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef __cplusplus
namespace libflvoperations
{
extern "C"
{
#endif // __cplusplus

const char* errorCodeToString(uint32_t errorCode);

#ifdef __cplusplus
}

}; // namespace libflvoperations
#endif // __cplusplus

