#pragma once
#include <inttypes.h>
#include "mp4repairdefinitions.h"

#ifdef DEBUG
#undef DEBUG
#endif

namespace libwamediacommon
{

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void reportProblem(eMP4_FORMAT_PROBLEM_TYPE formatProblemType,
                   uint32_t nTrackIndex,
                   bool bRelaxedCriterion,
                   const char* fmt, ...);

void reportSimpleReplacementProblem(eMP4_FORMAT_PROBLEM_TYPE formatProblemType,
                                    uint32_t nTrackIndex,
                                    uint32_t nDesiredNewValue,
                                    bool bRelaxedCriterion,
                                    const char* fmt, ...);

#ifdef __cplusplus
}
#endif // __cplusplus
}; // namespace libwamediacommon
