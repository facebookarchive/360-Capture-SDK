#pragma once
#include "h264picturedefinitions.h"
#include "errorcodes.h"
using namespace libwamediacommon;

#ifdef __cplusplus
namespace libwamediastreams
{

extern "C"
{
#endif // __cplusplus

WAMEDIA_STATUS parsePictureFrame(uint8_t* pFrameBuffer,
	                                  uint32_t nFrameBytes,
	                                  bool bSeparateColourPlaneFlag,
	                                  uint8_t nNumberOfFrameNumBits, // SPS: log2_max_frame_num_minus4 + 4
	                                  PICTURE_FRAME_LAYOUT* pPictureFrameLayout);
void debugPrintPictureFrame(PICTURE_FRAME_LAYOUT* pPictureFrameLayout); //, vector<uint32_t> & listOfEmulationPreventionBitOffsets);
void deallocatePictureFrameLayout(PICTURE_FRAME_LAYOUT* pPictureFrameLayout);
#ifdef __cplusplus
}

}; // namespace libwamediastreams
#endif // __cplusplus
