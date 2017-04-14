#include "SPSdefinitions.h"
#include <vector>
using namespace std;
#include "utilities.h"
using namespace libwamediacommon;

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

void checkForAndPrintEmulationPrevention(uint32_t* pListOfEmulationPreventionBitOffsets,
                                         uint32_t nListOfEmulationPreventionBitOffsetsElements,
                                         uint32_t* pnIndexOfNextEmPrev,
                                         uint32_t nCurrentBitOffset);
void debugPrintSPS(SPS_LAYOUT* pSpsLayout,
	               uint32_t* pListOfEmulationPreventionBitOffsets,
	               uint32_t nListOfEmulationPreventionBitOffsetElements);

#ifdef __cplusplus
}; // namespace libwamediastreams
#endif // __cplusplus

