#pragma once
#include "avstreamsdefinitions.h"

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

typedef enum
{
  TRAK_TYPE_UNDEFINED = 0,
  TRAK_TYPE_VIDEO,
  TRAK_TYPE_AUDIO,
  TRAK_TYPE_HINT,
  TRAK_TYPE_META,
  TRAK_TYPE_AUXVIDEO,
  TRAK_TYPE_OBJECT_DESCRIPTOR_STREAM,
  TRAK_TYPE_CLOCK_REFERENCE_STREAM,
  TRAK_TYPE_SCENE_DESCRIPTION_STREAM,
  TRAK_TYPE_MPEG7_STREAM,
  TRAK_TYPE_OBJECT_CONTENT_INFO_STREAM,
  TRAK_TYPE_IPMP_STREAM,
  TRAK_TYPE_MPEG_J_STREAM
} eTRAK_TYPE;

typedef struct
{
  eVIDEO_STREAM_TYPE videoStreamType;
  uint32_t nReserved;  // in the case of H.264 used to store stream profile
  uint32_t nReserved1; // in the case of H.264 used to store stream level
  uint32_t nRotationNeeded;
 
  uint16_t nVideoWidth;
  uint16_t nVideoHeight;
  uint32_t nNumberOfVideoFrames;
  uint32_t nDeclaredAverageBitsPerSecond;
  uint32_t nCalculatedAverageBitsPerSecond;
} VIDEO_STREAM_INFO;

typedef struct
{
  eAUDIO_STREAM_TYPE audioStreamType;
  uint32_t audioStreamSubType;
  uint32_t nMPEGAudioObjectType;
  uint32_t nMPEGAudioToolsPresent; // SBR= bit0, PS=bit1

  uint16_t nNumberOfChannels;
  uint16_t nBitsPerSample;
  uint32_t nSamplingRate;
  uint32_t nDeclaredAverageBitsPerSecond;
  uint32_t nCalculatedAverageBitsPerSecond;
} AUDIO_STREAM_INFO;

typedef struct
{
  eTRAK_TYPE trakType;
  uint8_t    handler_type[5]; // string, actually
  union
  {
    VIDEO_STREAM_INFO video;
    AUDIO_STREAM_INFO audio;
  } mediaStreamInfo;
} TRAK_AVSTREAM_INFO;

typedef struct
{
  uint32_t timescale;
  uint32_t duration;
} TRAK_DURATION;

#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus

