#pragma once
#include "flvstreamreport.h"

#ifdef __cplusplus
namespace libflvoperations
{
#endif // __cplusplus

typedef struct
{
  FLV_AUDIO_STREAM_REPORT asr;
  FLV_VIDEO_STREAM_REPORT vsr;
} FLV_FILE_QUICK_INFO;

#ifdef __cplusplus
}; // namespace flvoperations
#endif // __cplusplus
