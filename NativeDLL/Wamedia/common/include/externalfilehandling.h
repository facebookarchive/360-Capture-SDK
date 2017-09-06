#pragma once
#include "filehandlingdefinitions.h"

#ifdef __cplusplus
namespace libwamediacommon
{

extern "C"
{
#endif // __cplusplus

bool areFilesHandledExternally();

bool openfile(const char* pStrFilename, uint32_t nFlags, void** ppFileDescriptor);
bool isfileopen(void* pFileDescriptor);
void fileseek(void* pFileDescriptor, uint64_t nTargetFileOffset, uint32_t nOrigin);
uint64_t filetell(void* pFileDescriptor);
bool fileread(void* pFileDescriptor, char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead);
bool filewrite(void* pFileDescriptor, char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten);
void fileclose(void* pFileDescriptor);

#ifdef __cplusplus
}

}; // namespace libwamediacommon
#endif // __cplusplus

