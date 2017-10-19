#pragma once

#ifdef __cplusplus
namespace libwamediastreams
{

extern "C"
{
#endif // __cplusplus

int bitToolsTest(void);
int expGolombEncodingTest();
void debugPrintRawBuffer(const char* pStrPreamble, uint8_t* pBuffer, uint32_t nBufferBytes);
int printFrameNumber(const char* pStrInputFilename);

#ifdef __cplusplus
}

}; // namespace libwamediastreams
#endif // __cplusplus
