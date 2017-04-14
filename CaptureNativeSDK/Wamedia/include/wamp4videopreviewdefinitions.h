#pragma once
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "mp4definitions.h"
#include "mp4synthesisdefinitions.h"
#include "whatsappSpecificDefinitions.h"

#ifdef __cplusplus
namespace libwhatsappspecific
{

#endif // __cplusplus

typedef struct
{
  uint32_t nReserved;
} MP4_WA_ANIMGIF_DATA;

#define WA_VIDEOPREVIEW_SPECIFIC_MP4_BOX_TYPE_STRING            ("vprw")
#define WA_VIDEOPREVIEW_SPECIFIC_MP4_BOX_START_BYTE_LENGTH      (MANDATORY_FULL_BOX_SIZE)

#define WA_VIDEOPREVIEW_SPECIFIC_MP4_BOX_VERSION                (1)
#define WA_VIDEOPREVIEW_BOX_DEPTH_LEVEL                         (0)
#define WA_VIDEOPREVIEW_SPECIFIC_MP4_BOX_PREFERRED_BOX_LOCATION (MP4_BOX_LOCATION_IMMEDIATELY_AFTER_VERSION_BOX)

#ifdef __cplusplus
}; // namespace libwhatsappspecific
#endif // __cplusplus

