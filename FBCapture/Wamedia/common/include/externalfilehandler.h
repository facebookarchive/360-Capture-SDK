#pragma once
#include "filehandlingdefinitions.h"

#ifdef __cplusplus
namespace libwamediacommon
{

extern "C"
{
#endif //__cplusplus

typedef struct
{
  OPENFILEFUNC   pOpenFileCallback;
  ISFILEOPENFUNC pIsFileOpenCallback;
  FILESEEKFUNC   pFileSeekCallback;
  FILETELLFUNC   pFileTellCallback;
  FILEREADFUNC   pFileReadCallback;
  FILEWRITEFUNC  pFileWriteCallback;
  FILECLOSEFUNC  pFileCloseCallback;
  void*          pCallbackToken;
} CALLBACKS_INFO;

void setFileHandlingCallback(CALLBACKS_INFO cbi);

#ifdef __cplusplus
} // extern "C"

}; // namespace libwamediacommon
#endif //__cplusplus

