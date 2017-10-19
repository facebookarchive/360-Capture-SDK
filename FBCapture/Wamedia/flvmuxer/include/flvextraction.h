#pragma once
#include <inttypes.h>
//#include "flvdemuxreport.h"

#ifdef __cplusplus
namespace libflvoperations
{

extern "C"
{
#endif // __cplusplus

//
// FLV file streams extraction support
//
uint32_t extractAVStreams(const char* pStrInputFilename, const char* pStrOutputDirectory);
uint32_t extractAVCThumbnailVideoStream(const char* pStrInputFilename, const char* pStrOutputDirectory, uint32_t nMaxThumbnails);

#if 0
//
// FLV stream demux support
//
#define INVALID_TIMESTAMP ((uint64_t)-1)

typedef bool (*DEMUXSTREAMWRITE)(void* pCallbackToken, char* pBuffer, uint32_t nBytes, bool bSeekPoint, uint64_t nTimestamp);
typedef struct
{
  uint32_t nTrackID;
  float    fFrameDuration;
  DEMUXSTREAMWRITE pWriteCallback;
  void* pWriteCallbackToken;
} DEMUX_STREAM_DUMP_INFO;

typedef void* HSTRMDMX;

HSTRMDMX openFLVStreamDemux(const char* pStrInputFilename, 
                            uint64_t nStreamedResourceContentLength);
uint32_t identifyFLVStreamAVStreams(HSTRMDMX hStrmDmx,
                            uint64_t nAvailableStreamBytes,
                            bool bLastMDATBoxHasBeenReceived,
                            MP4_FILE_QUICK_DEMUX_INFO* pDemuxQuickInfo);
uint32_t demultiplexSelectedStreams(HSTRMDMX hStrmDmx,
                            DEMUX_STREAM_DUMP_INFO* pDemuxStreamDumpInfos,
                            uint32_t nDemuxStreamDumpInfos);
void closeFLVStreamDemux(HSTRMDMX hStrmDmx);

//
// Alternatively: assuming the details of A/V streams carried by the MP4 byte stream
// are known in advance, a single API call may be sufficient to complete the demuxing:
//
uint32_t demultiplexMP4ByteStream(const char* pStrInputFilename);
#endif

#ifdef __cplusplus
}

}; // namespace libflvoperations
#endif // __cplusplus

