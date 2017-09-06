#pragma once
#include "commonDefinitions.h"

#ifdef __cplusplus
namespace libwamediacommon
{

string fourCharsToUTF8String(uint32_t nBoxType);

extern "C"
{
#endif // __cplusplus

uint16_t swapBytes16(uint16_t nInput);
uint32_t swapBytes32(uint32_t nInput);
uint64_t swapBytes64(uint64_t nInput);
float convertFixed32BitToFloat(int32_t nInput, uint32_t nPrecision);
uint32_t byteOffset(uint32_t nOffset);
uint32_t extraBitsOffset(uint32_t nOffset);

#ifdef __cplusplus
}

}; // namespace libwamediacommon
#endif // __cplusplus
