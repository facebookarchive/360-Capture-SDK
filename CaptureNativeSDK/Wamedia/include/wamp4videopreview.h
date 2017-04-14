#pragma once
#include "mp4errors.h"
#include "wamp4videopreviewdefinitions.h"
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

uint32_t insertVideoPreview(const char* pStrInputMP4Filename,
                            const char* pStrVideoPreviewFilename,
                            const char* pStrOutputMP4Filename);
uint32_t extractVideoPreview(const char* pStrInputMP4Filename,
                             uint8_t** ppVideoPreviewBuffer,
                             uint32_t* pnVideoPreviewBufferBytes,
                             bool bIgnoreExistingFileSyntaxImperfections);
void deallocateVideoPreviewBuffer(uint8_t** ppVideoPreviewBuffer);
#ifdef __cplusplus
}

}; // namespace libwhatsappspecific
#endif // __cplusplus
