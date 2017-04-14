#pragma once
#include "mp4errors.h"
#include "avstreamsmuxquality.h"
using namespace libwamediastreams;

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

typedef void* HMP4EDIT;

typedef struct
{
  uint64_t     nFileOffset;
  float        fTimestamp;
  uint32_t     nVideoSampleIndex;
  uint32_t     nPertinentChunkIndex; // needed mostly for internal operations
} EDIT_POINT;

typedef struct
{
  float    fStartTime;
  float    fDuration;
  float    fStopTime;
  
  uint32_t nStartVideoFrameIndex;
  uint32_t nVideoFramesCount;
  uint32_t nStopVideoFrameIndex;

  uint32_t nPertinentChunkIndex; // needed mostly for internal operations
} EDIT_RANGE;

typedef struct
{
  uint32_t     nNumberOfVideoFrames;
  float        fFramesPerSecond;
  float        fDuration;
} INPUT_FILE_TIMING_INFO;

HMP4EDIT openMp4Editor(void);
uint32_t examineInputFile(HMP4EDIT hMp4Edit,
                          const char* pStrInputFilename,
                          INPUT_FILE_TIMING_INFO & ifti,
                          MUX_QUALITY_REPORT & mqr,
                          EDIT_POINT** ppEditPoints,
                          uint32_t* pnEditPoints);
uint32_t editMp4File(HMP4EDIT hMp4Edit,
	                 const char* pStrOutputFilename,
	                 EDIT_RANGE* pEditRanges,
	                 uint32_t nEditRanges);
uint32_t closeMp4Editor(HMP4EDIT hMp4Edit);
void freeEditPoints(EDIT_POINT** ppEditPoints);

void printInputFileTimingInfo(const char* pStrInputFilename, const INPUT_FILE_TIMING_INFO ifti);
void printEditPointsList(EDIT_POINT* pAvailableEditPoints, uint32_t nAvailableEditPoints, const char* pStrCaption);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

