#pragma once
#include "externalfilehandler.h"
#include "externalfilehandling.h"

namespace libwamediacommon
{

class CFileHandlingCallCenter
{
public:
  ~CFileHandlingCallCenter();
  static CFileHandlingCallCenter & GetInstance();

public:
  void setFileHandlingCallback(CALLBACKS_INFO cbi);
  bool areFilesHandledExternally(void);

  bool openfile(const char* pStrFilename, uint32_t nFlags, void** ppAbstractFileDescriptor);
  bool isfileopen(void* pAbstractFileDescriptor);
  void fileseek(void* pAbstractFileDescriptor, uint64_t nTargetFileOffset, uint32_t nOrigin);
  uint64_t filetell(void* pAbstractFileDescriptor);
  bool fileread(void* pAbstractFileDescriptor, char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead);
  bool filewrite(void* pAbstractFileDescriptor, char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten);
  void fileclose(void* pAbstractFileDescriptor);
  
private:
  CFileHandlingCallCenter();
  CFileHandlingCallCenter(CFileHandlingCallCenter const &){};
  CFileHandlingCallCenter operator=(CFileHandlingCallCenter const &);
  
private:
  static CFileHandlingCallCenter s_fileHandler;
  CALLBACKS_INFO m_cbi;
};

}; // namespace libwamediacommon
