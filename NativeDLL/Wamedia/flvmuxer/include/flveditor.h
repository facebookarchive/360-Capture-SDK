#pragma once
#include "flverrors.h"
#include "avstreamsmuxquality.h"
using namespace libwamediastreams;

#ifdef __cplusplus
namespace libflvoperations
{

extern "C"
{
#endif // __cplusplus

typedef void* HFLVEDIT;

typedef struct
{
  uint64_t     nFileOffset;
  float        fTimestamp;
  uint32_t     nVideoSampleIndex;
} FLV_EDIT_POINT;

typedef struct
{
  float    fStartTime;
  float    fDuration;
  float    fStopTime;
  
  uint32_t nStartVideoFrameIndex;
  uint32_t nVideoFramesCount;
  uint32_t nStopVideoFrameIndex;

  uint32_t nPertinentChunkIndex; // needed mostly for internal operations
} FLV_EDIT_RANGE;

typedef struct
{
  uint32_t     nNumberOfVideoFrames;
  float        fFramesPerSecond;
  float        fDuration;
} FLV_INPUT_FILE_TIMING_INFO;

HFLVEDIT openFLVEditor(void);
uint32_t examineInputFLVFile(HFLVEDIT hFLVEdit,
                             const char* pStrInputFilename,
                             FLV_INPUT_FILE_TIMING_INFO & ifti,
                             MUX_QUALITY_REPORT & mqr,
                             FLV_EDIT_POINT** ppEditPoints,
                             uint32_t* pnEditPoints);
uint32_t editFLVFile(HFLVEDIT hFLVEdit,
	                 const char* pStrOutputFilename,
	                 FLV_EDIT_RANGE* pEditRanges,
	                 uint32_t nEditRanges);
uint32_t closeFLVEditor(HFLVEDIT hFLVEdit);
void freeEditPoints(FLV_EDIT_POINT** ppEditPoints);

void printInputFileTimingInfo(const char* pStrInputFilename, const FLV_INPUT_FILE_TIMING_INFO fifti);
void printEditPointsList(FLV_EDIT_POINT* pAvailableEditPoints, uint32_t nAvailableEditPoints, const char* pStrCaption);

#ifdef __cplusplus
}

}; // namespace libflvoperations
#endif // __cplusplus

