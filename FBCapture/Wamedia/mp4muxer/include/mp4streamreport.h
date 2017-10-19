#pragma once
#include "mp4errors.h"
#include "mp4streamsdefinitions.h"

#ifdef __cplusplus
namespace libmp4operations
{
using namespace libwamediastreams;

extern "C"
{
#endif // __cplusplus

#define INVALID_FRAMES_PER_SECOND_VALUE (-1.0f)

typedef struct _VIDEO_STREAM_REPORT
{
  eVIDEO_STREAM_TYPE videoStreamType;
  uint32_t           nProfile;
  uint32_t           nLevel;
  int32_t            nRotationAngleDegrees;

  uint16_t           nVideoWidth;
  uint16_t           nVideoHeight;
  float              fNominalFPS;
  float              fCalculatedFPS;
  float              fDuration;
  uint32_t           nAvgBitsPerSecond;

  struct _VIDEO_STREAM_REPORT* pNext;
} VIDEO_STREAM_REPORT;

typedef struct _AUDIO_STREAM_REPORT
{
  eAUDIO_STREAM_TYPE audioStreamType;
  uint32_t           audioStreamSubType;
  uint32_t           nMPEGAudioObjectType;
  uint32_t           nMPEGAudioToolsPresent; // SBR= bit0, PS=bit1

  uint16_t           nNumberOfChannels;
  uint32_t           nSamplingRate;
  float              fReserved;
  float              fReserved1;
  float              fDuration;
  uint32_t           nAvgBitsPerSecond;

  struct _AUDIO_STREAM_REPORT* pNext;
} AUDIO_STREAM_REPORT;

void printAVStreamsReports(AUDIO_STREAM_REPORT asr, VIDEO_STREAM_REPORT vsr, bool bQuickTimeFlavor);
const char* videoStreamTypeToString(eVIDEO_STREAM_TYPE videoStreamType);
const char* audioStreamTypeToString(eAUDIO_STREAM_TYPE audioStreamType,
                                    eAUDIO_STREAM_OBJECT_SUBTYPE audioStreamSubType,
                                    uint32_t nMPEGAudioToolsPresent);

#ifdef __cplusplus
} // extern "C"

}; // namespace mp4operations
#endif // __cplusplus
