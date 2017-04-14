#pragma once
#include "mp4streamreport.h"
#include "mp4version.h"

#ifdef __cplusplus
namespace libmp4operations
{
#endif // __cplusplus

typedef struct
{
  AUDIO_STREAM_REPORT asr;
  VIDEO_STREAM_REPORT vsr;
  MP4_OPERATIONS_VERSION_INFO ver;
} MP4_FILE_QUICK_INFO;

#ifdef __cplusplus
}; // namespace mp4operations
#endif // __cplusplus
