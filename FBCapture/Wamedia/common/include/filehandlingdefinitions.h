#pragma once
#include <inttypes.h>

namespace libwamediacommon
{

typedef bool (*OPENFILEFUNC)(const char* pStrFilename, uint32_t nFlags, void** pAbstractFileDescriptor, void* pCBToken);
typedef bool (*ISFILEOPENFUNC)(void* pAbstractFileDescriptor, void* pCBToken);
typedef void (*FILESEEKFUNC)(void* pAbstractFileDescriptor, uint64_t nTargetFileOffset, uint32_t nOrigin, void* pCBToken);
typedef uint64_t (*FILETELLFUNC)(void* pAbstractFileDescriptor, void* pCBToken);
typedef bool (*FILEREADFUNC )(void* pAbstractFileDescriptor, char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead, void* pCBToken);
typedef bool (*FILEWRITEFUNC)(void* pAbstractFileDescriptor, char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten, void* pCBToken);
typedef void (*FILECLOSEFUNC)(void* pAbstractFileDescriptor, void* pCBToken);

}; // namespace libwamediacommon

