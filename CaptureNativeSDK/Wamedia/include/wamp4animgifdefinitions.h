#pragma once
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "mp4topology.h"
#include "mp4synthesisdefinitions.h"
using namespace libmp4operations;

#ifdef __cplusplus
namespace libwhatsappspecific
{

#endif // __cplusplus

typedef struct
{
  uint32_t nReserved;
} MP4_WA_ANIMGIF_DATA;

typedef struct
{
public:
  MINIMUM_BOX_IDENTIFIER minBox;
  MP4_WA_ANIMGIF_DATA    animatedGIFData;
} WHATSAPP_ANIMATED_GIF_TAG_BOX;

#define WA_ANIMGIF_SPECIFIC_MP4_BOX_TYPE_STRING ("loop")
#define WA_ANIMGIF_METADATA_BYTES (MANDATORY_BOX_SIZE + sizeof(MP4_WA_ANIMGIF_DATA))

#define WA_ANIMGIF_SPECIFIC_MP4_BOX_DEPTH_LEVEL            (0)
#define WA_ANIMGIF_SPECIFIC_MP4_BOX_PREFERRED_BOX_LOCATION (MP4_BOX_LOCATION_IMMEDIATELY_AFTER_VERSION_BOX)

#ifdef __cplusplus
}; // namespace libwhatsappspecific
#endif // __cplusplus

