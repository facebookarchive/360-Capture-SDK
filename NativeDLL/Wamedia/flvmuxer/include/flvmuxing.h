#pragma once
#include <stdbool.h>
#include <float.h>
#include "flverrors.h"
#include "mp4trakdefinitions.h"
using namespace libwamediastreams;

#ifdef __cplusplus
namespace libflvoperations
{

extern "C"
{
#endif // __cplusplus

#define UNKNOWN_FRAMES_PER_SECOND           (0)
#define DEFAULT_SUGGESTED_FRAMES_PER_SECOND (30)
#define NO_DURATION_SPECIFIED               (FLT_MAX)

uint32_t flvmuxAVStreams(const char* pStrAudioStreamInputFilename, 
                      const char* pStrVideoStreamInputFilename, 
                      const char* pStrOutputFilename,
                      float fAudioStartTime, // = 0.0f,
                      float fVideoStartTime, // = 0.0f,
                      float fTargetDuration, // = NO_DURATION_SPECIFIED,
                      float fSuggestedFPS    // = UNKNOWN_FRAMES_PER_SECOND
                     );

// Network muxing support
typedef struct
{
  uint32_t nIndex;
  uint8_t* pBuffer;
  uint32_t nBytes;
  uint32_t nTimestampMilliseconds;
  bool     bEndOfStream;
} AV_STREAM_DATA;

typedef enum
{
  VIDEO_PAYLOAD_TYPE_BITSTREAM = 0,
  VIDEO_PAYLOAD_TYPE_SPS,
  VIDEO_PAYLOAD_TYPE_PPS
} eVIDEO_PAYLOAD_TYPE;

typedef bool (*GET_AUDIO_PAYLOAD)(AV_STREAM_DATA* pAudioData, bool bDecoderSpecificConfig, void* pCBToken);
typedef bool (*GET_VIDEO_PAYLOAD)(AV_STREAM_DATA* pVideoData, eVIDEO_PAYLOAD_TYPE videoPayloadType, void* pCBToken);

uint32_t muxAVStreamsNetwork(GET_AUDIO_PAYLOAD pGetAudioData,
                             void* pGetAudioDataToken,
                             GET_VIDEO_PAYLOAD pGetVideoData,
                             void* pGetVideoDataToken,
                             const char* pStrOutputFilename,
                             AUDIO_STREAM_INFO asi,
                             VIDEO_STREAM_INFO vsi,
                             float fAudioStartTime, // = 0.0f,
                             float fVideoStartTime, // = 0.0f,
                             float fTargetDuration, // = NO_DURATION_SPECIFIED,
                             float fSuggestedFPS    // = UNKNOWN_FRAMES_PER_SECOND
                            );

#ifdef __cplusplus
}

}; // namespace libflvoperations
#endif // __cplusplus
