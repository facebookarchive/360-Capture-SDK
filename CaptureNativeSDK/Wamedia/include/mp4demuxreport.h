#pragma once
#include "mp4streamreport.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

typedef struct _VIDEO_DEMUX_STREAM_REPORT
{
  uint32_t nTrackID;
  VIDEO_STREAM_REPORT vsr;
  struct _VIDEO_DEMUX_STREAM_REPORT* pNext;
} VIDEO_DEMUX_STREAM_REPORT;

typedef struct _AUDIO_DEMUX_STREAM_REPORT
{
  uint32_t nTrackID;
  AUDIO_STREAM_REPORT asr;
  struct _AUDIO_DEMUX_STREAM_REPORT* pNext;
} AUDIO_DEMUX_STREAM_REPORT;

typedef struct
{
  AUDIO_DEMUX_STREAM_REPORT adsr;
  VIDEO_DEMUX_STREAM_REPORT vdsr;
} MP4_FILE_QUICK_DEMUX_INFO;

#ifdef __cplusplus
} // extern "C"

}; // namespace mp4operations
#endif // __cplusplus
