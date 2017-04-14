#pragma once
#include "mp4errors.h"
#include "mp4synthesisdefinitions.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

uint32_t insertMP4BoxToFile(const char* pStrInputFilename,
                            const char* pStrOutputFilename,
                            uint32_t nBoxDepthLevel, // 0 = top level box, residing alongside with ftyp, moov, mdat,...
                            uint32_t nBoxType,
                            uint64_t nBoxSize,
                            uint8_t* pBoxContents, // carrying complete, verbatim MP4 box to be written
                            eMP4_BOX_LOCATION nPreferredBoxLocation,
                            bool bReplaceExisting,
                            bool bIgnoreExistingSyntaxImperfections,
                            int64_t* pnChunkOffsetAdjustment);

uint32_t isMP4BoxFoundInFile(const char* pStrInputFilename,
                            uint32_t nBoxType,
                            bool* pbFileAlreadyCarriesBox,
                            uint32_t nExpectedBoxDepthLevel);

uint32_t extractMP4BoxContents(const char* pStrInputFilename,
                            uint32_t nBoxType,
                            uint32_t nBoxDepthLevel,
                            uint8_t** ppBoxContentsBuffer,
                            uint32_t* pnBoxContentsBufferBytes,
                            bool bIgnoreExistingFileSyntaxImperfections);

void deallocateMP4BoxRetrievalBuffer(uint8_t** ppBoxContentsBuffer);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

