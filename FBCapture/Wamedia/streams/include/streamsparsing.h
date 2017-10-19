#pragma once
#include "muxingdefinitions.h"
#include "avstreamsdefinitions.h"
#include "errorcodes.h"
#include "filewrapper.h"
using namespace libwamediacommon;

#ifdef __cplusplus
namespace libwamediastreams
{

extern "C"
{
#endif //__cplusplus

bool IsID3v2TagEncountered(char chTest[TAG_EXAMINATION_BUFFER_BYTES]);
WAMEDIA_STATUS SkipID3v2Tag(char (*pchTest)[TAG_EXAMINATION_BUFFER_BYTES],
                            uint32_t nMaxBytes,
                            IFileReader *pReader);
bool IsEndTagEncountered(char chTest[TAG_EXAMINATION_BUFFER_BYTES]);

WAMEDIA_STATUS ExamineAACStream(const char* pStrAudioStreamFile,
                                float fStartTime,
                                float fTargetDuration,
                                MUX_INPUT_STREAM_TOPOLOGY* pMist,
                                bool bQuickTimeMuxFlavor);

WAMEDIA_STATUS ParseAMRStream(const char* pStrAudioStreamFile);
WAMEDIA_STATUS ExamineAMRStream(const char* pStrAudioStreamFile,
                                float fStartTime,
                                float fTargetDuration,
                                MUX_INPUT_STREAM_TOPOLOGY* pMist);
  
WAMEDIA_STATUS ExamineMp3Stream(const char* pStrAudioStreamFile, 
                                float fStartTime,
                                float fTargetDuration,
                                MUX_INPUT_STREAM_TOPOLOGY* pMist);
bool IsValidMp3Header(MP3_HEADER mp3HeaderCandidate, 
                      uint32_t* pnFrameLength, 
                      bool bStrictValidation);
bool VerifyMp3HeadersConsistency(MP3_HEADER veryFirstHeader, MP3_HEADER newHeader);
bool GetMp3FrameAudioParameters(MP3_HEADER mp3Header,
                                uint32_t* pnSamplingRate,
                                uint8_t*  pnLayer,
                                uint8_t*  pnChannelConfiguration);
bool GetMp3FrameAudioSamplesPerChannel(MP3_HEADER mp3Header,
                                uint32_t* pnSamplesPerChannel);

WAMEDIA_STATUS ParseQCELPStream(const char* pStrAudioStreamFile);
WAMEDIA_STATUS ExamineQCELPStream(const char* pStrAudioStreamFile,
                                  float fStartTime,
                                  float fTargetDuration,
                                  MUX_INPUT_STREAM_TOPOLOGY* pMist);

WAMEDIA_STATUS ParseOggStream(const char* pStrAudioStreamFile);

WAMEDIA_STATUS ParseH264Stream(const char* pStrVideoStreamFile,
                               float* pfFramesPerSecond);
WAMEDIA_STATUS ExamineH264Stream(const char* pStrVideoStreamFile,
                                 float fStartTime,
                                 float fTargetDuration,
                                 MUX_INPUT_STREAM_TOPOLOGY* pMist,
                                 float* pfFramesPerSecond,
                                 REPORT_H264_HEADER pReportH264HeaderCallback,
                                     void* pCallbackToken);
bool ParseSPSextractFramesPerSecond(uint8_t* pSPSBuffer,
                                    uint32_t nSPSBufferBytes,
                                    float* pfFramesPerSecond);
bool ExamineSPSextractVideoParameters(uint8_t* pSPSBuffer,
                                      uint32_t nSPSBufferBytes,
                                      uint8_t* pnH264Profile,
                                      uint8_t* pnH264Level,
                                      uint32_t* pnVideoWidth,
                                      uint32_t* pnVideoHeight,
                                      uint32_t* pnVideoTimescale,
                                      float* pfFramesPerSecond);

bool DetermineH264SliceType(uint8_t* pSliceBuffer, uint32_t nSliceBufferBytes, uint32_t* pnSliceType);

char* NALUnitTypeToString(uint8_t nNALUnitType);
void DeallocateWaMediaStreamsString(char** ppString);

void CleanupMuxInputStreamTopology(MUX_INPUT_STREAM_TOPOLOGY* pMuxInputStreamTopology);

#ifdef __cplusplus
} // extern "C"

}; // namespace libwamediastreams
#endif //__cplusplus
	
