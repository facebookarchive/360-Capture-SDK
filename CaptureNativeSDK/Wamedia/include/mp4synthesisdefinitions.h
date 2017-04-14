#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{
#endif // __cplusplus

typedef enum
{
  MP4_BOX_LOCATION_UNDEFINED = 0,
  MP4_BOX_LOCATION_IMMEDIATELY_AFTER_FTYP = 1,
  MP4_BOX_LOCATION_IMMEDIATELY_AFTER_VERSION_BOX,
  MP4_BOX_LOCATION_LAST_BEFORE_MOOV_OR_MDAT,
  MP4_BOX_LOCATION_AFTER_MOOV_BEFORE_MDAT,
  MP4_BOX_LOCATION_LAST_TOP_LEVEL_BOX
} eMP4_BOX_LOCATION;

#ifdef __cplusplus
}; // namespace libmp4operations
#endif // __cplusplus

