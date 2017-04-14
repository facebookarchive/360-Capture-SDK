#pragma once
#include "mp4trakdefinitions.h"
#include <float.h>
#include <vector>
#include <string.h>
using namespace std;

#include "errorcodes.h"
using namespace libwamediacommon;

#define AUDIO_ONLY_SAMPLES_PER_CHUNK   (50)
#define VIDEO_TIME_SCALE_MULTIPLICATOR (1000)
#define DEFAULT_VIDEO_TIMESCALE_VALUE VIDEO_TIME_SCALE_MULTIPLICATOR
#define DEFAULT_SUGGESTED_FRAMES_PER_SECOND (30)
#define NO_DURATION_SPECIFIED          (FLT_MAX)

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

typedef struct
{
  float fSampleStartTime;
  float fSampleDuration;
  
  // the same data, just in the original 
  // raw form, suitable for other scenarios
  uint64_t nSampleStartTime;
  uint32_t nTimescale;
  uint32_t nSampleDuration;
  
  //
  uint32_t nCTTS_sample_offset; // some traks have CTTS box
} SAMPLE_TIMING_INFO;

typedef struct 
{
  uint32_t           nAbsoluteSampleIndex;
  uint64_t           nSampleStartFileOffset;
  uint32_t           nSampleSize;
  uint32_t           nSamplePresentationTimeOffset;
  bool               bIsSeekPoint;
  SAMPLE_TIMING_INFO timingInfo;

  // H.264 specific (not used otherwise)
  bool               bStartOfISOSample; // Access Unit Delimiter, SEI, IDR - they all may start the actual sample
  eNAL_UNIT_TYPE     nNALIdentifier;
  bool               bThreeBytesNALUnitStart;
} MUX_SAMPLE_INFO;

typedef struct
{
  float              fStreamDuration;
  TRAK_DURATION      streamDuration;
  TRAK_AVSTREAM_INFO tai;
  MUX_SAMPLE_INFO*   pStreamTopology;  
  uint32_t           nStreamTopologyEntries;
} MUX_INPUT_STREAM_TOPOLOGY;

class CMuxInputStreamTopology
{
public:
  CMuxInputStreamTopology()
  {
    fStreamDuration = 0.0f;
    memset(&streamDuration, 0, sizeof(TRAK_DURATION));
    memset(&tai, 0, sizeof(TRAK_AVSTREAM_INFO));
  }

public:
  float                   fStreamDuration;
  TRAK_DURATION           streamDuration;
  TRAK_AVSTREAM_INFO      tai;
  vector<MUX_SAMPLE_INFO> streamTopology;  
};

typedef struct
{
  bool     bQuickTimeFlavor;
  uint32_t nAssociatedTrackIndex;
  uint32_t nBoxType;
  uint32_t nFileOffset;
  uint32_t nBoxSizeLo32Bits;
  uint32_t nBoxSizeHi32Bits;
} BOX_OF_INTEREST_INFO;

typedef void(*REPORT_REUSABLE_BOX)(bool, BOX_OF_INTEREST_INFO, void*);

typedef struct 
{
  uint8_t* pData;
  uint32_t nBytes;
} H264_HEADER; //used to store SPS or PPS

typedef void(*REPORT_H264_HEADER)(bool, H264_HEADER &, void*);

#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus
