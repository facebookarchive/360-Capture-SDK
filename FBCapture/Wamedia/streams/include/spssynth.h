#pragma once
#include "SPSdefinitions.h"
#include "errorcodes.h"
using namespace libwamediacommon;
#include <vector>
using namespace std;

#ifdef __cplusplus
namespace libwamediastreams
{

extern "C"
{
#endif // __cplusplus

WAMEDIA_STATUS analyzeSPS(uint8_t* pSPSBuffer,
	                           uint32_t nSPSBufferBytes,
	                           SPS_LAYOUT* pSpsLayout,
	                           uint32_t** ppListOfEmulationPreventionBitOffsets,
	                           uint32_t* pnListOfEmulationPreventionBitOffsetsElements);
WAMEDIA_STATUS synthesizeSPS(const SPS_LAYOUT* pSpsLayout, uint8_t** ppSPSBuffer, uint32_t* pnSPSBufferBytes);
void deallocateSPSLayout(SPS_LAYOUT* pSpsLayout);
void deallocateListOfEmulationPreventionBitOffsets(uint32_t** ppListOfEmulationPreventionBitOffsets);

#ifdef __cplusplus
}

}; // namespace libwamediastreams
#endif // __cplusplus
