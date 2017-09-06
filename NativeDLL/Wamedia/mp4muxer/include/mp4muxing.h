#pragma once
#include <stdbool.h>
#include <float.h>
#include "mp4errors.h"
#include "mp4videorotationmodes.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

#define UNKNOWN_FRAMES_PER_SECOND           (0)
#define DEFAULT_SUGGESTED_FRAMES_PER_SECOND (30)
#define NO_DURATION_SPECIFIED               (FLT_MAX)

uint32_t mp4muxAVStreams(const char* pStrAudioStreamInputFilename, 
                      const char* pStrVideoStreamInputFilename, 
                      const char* pStrOutputFilename,
                      float fAudioStartTime, // = 0.0f,
                      float fVideoStartTime, // = 0.0f,
                      float fTargetDuration, // = NO_DURATION_SPECIFIED,
                      float fSuggestedFPS,   // = UNKNOWN_FRAMES_PER_SECOND
                      eVideoRotationMode nVideoRotationMode,
                      bool bQuickTimeMuxFlavor);
#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus
