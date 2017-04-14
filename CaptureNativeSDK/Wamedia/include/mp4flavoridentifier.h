#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

typedef enum
{
  MP4_FORMAT_FLAVOR_UNIDENTIFIED = 0,
  MP4_FORMAT_FLAVOR_ISO_3G2_3GPP,
  MP4_FORMAT_FLAVOR_APPLE_QUICK_TIME_MOV,
  MP4_FORMAT_FLAVOR_FRAGMENTED_MP4
} eMP4_FORMAT_FLAVOR;

uint32_t determineMp4FormatFlavor(const char* pStrInputFilename,  eMP4_FORMAT_FLAVOR* pFlavor);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus
