#pragma once
#include "flverrors.h"
#include "flvstreamsdefinitions.h"

#ifdef __cplusplus
namespace libflvoperations
{
using namespace libwamediastreams;

extern "C"
{
#endif // __cplusplus

#define INVALID_FRAMES_PER_SECOND_VALUE (-1.0f)

typedef struct _VIDEO_STREAM_REPORT
{
  eFLV_VIDEO_CODEC_ID videoStreamType;
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
} FLV_VIDEO_STREAM_REPORT;

typedef struct _AUDIO_STREAM_REPORT
{
  eFLV_SOUND_FORMAT  audioStreamType;
  uint32_t           audioStreamSubType;
  uint32_t           nMPEGAudioObjectType;
  uint32_t           nMPEGAudioToolsPresent; // SBR= bit0, PS=bit1

  uint16_t           nNumberOfChannels;
  uint32_t           nSamplingRate;
  uint32_t           nBitsPerSample;
  float              fReserved1;
  float              fDuration;
  uint32_t           nAvgBitsPerSecond;

  struct _AUDIO_STREAM_REPORT* pNext;
} FLV_AUDIO_STREAM_REPORT;

void printFLVAVStreamsReports(FLV_AUDIO_STREAM_REPORT asr, FLV_VIDEO_STREAM_REPORT vsr);
const char* videoStreamTypeToString(eFLV_VIDEO_CODEC_ID videoStreamType);
const char* audioStreamTypeToString(eFLV_SOUND_FORMAT audioStreamType);

#ifdef __cplusplus
} // extern "C"

}; // namespace flvoperations
#endif // __cplusplus
