#pragma once
#include "mp4errors.h"
#include "wamp4animgifdefinitions.h"
#include <stdbool.h>

#ifdef DEBUG
#undef DEBUG
#endif


#ifdef __cplusplus
namespace libwhatsappspecific
{

extern "C"
{
#endif // __cplusplus

uint32_t applyWAanimatedGIFtag(const char* pStrInputFilename,
                               const char* pStrOutputFilename);
uint32_t isMP4FileTaggedAsWAanimatedGIF(const char* pStrInputFilename,
                               bool* pbFileIsAnimatedGIF,
                               bool bIgnoreFileSyntaxImperfections);
#ifdef __cplusplus
}

}; // namespace libwhatsappspecific
#endif // __cplusplus
