#include "filehandlingcallcenter.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace libwamediacommon
{

void setFileHandlingCallback(CALLBACKS_INFO cbi)
{
  CFileHandlingCallCenter::GetInstance().setFileHandlingCallback(cbi);
}

bool areFilesHandledExternally(void)
{
  return CFileHandlingCallCenter::GetInstance().areFilesHandledExternally();
}

bool openfile(const char* pStrFilename, uint32_t nFlags, void** ppAbstractFileDescriptor)
{
  return CFileHandlingCallCenter::GetInstance().openfile(pStrFilename, nFlags, ppAbstractFileDescriptor);
}

bool isfileopen(void* pAbstractFileDescriptor)
{
  return CFileHandlingCallCenter::GetInstance().isfileopen(pAbstractFileDescriptor);
}

void fileseek(void* pAbstractFileDescriptor, uint64_t nTargetFileOffset, uint32_t nOrigin)
{
  CFileHandlingCallCenter::GetInstance().fileseek(pAbstractFileDescriptor, nTargetFileOffset, nOrigin);
}

uint64_t filetell(void* pAbstractFileDescriptor)
{
  return CFileHandlingCallCenter::GetInstance().filetell(pAbstractFileDescriptor);
}

bool fileread(void* pAbstractFileDescriptor, char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead)
{
  return CFileHandlingCallCenter::GetInstance().fileread(pAbstractFileDescriptor, pBuffer, nBytesToRead, nBytesRead);
}

bool filewrite(void* pAbstractFileDescriptor, char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten)
{
  return CFileHandlingCallCenter::GetInstance().filewrite(pAbstractFileDescriptor, pBuffer, nBytesToWrite, nBytesWritten);
}

void fileclose(void* pAbstractFileDescriptor)
{
  CFileHandlingCallCenter::GetInstance().fileclose(pAbstractFileDescriptor);
}

CFileHandlingCallCenter CFileHandlingCallCenter::s_fileHandler;

CFileHandlingCallCenter::CFileHandlingCallCenter()
{
  memset(&m_cbi, 0, sizeof(CALLBACKS_INFO));
}

CFileHandlingCallCenter::~CFileHandlingCallCenter()
{
  memset(&m_cbi, 0, sizeof(CALLBACKS_INFO));
}

CFileHandlingCallCenter&
CFileHandlingCallCenter::GetInstance(void)
{
  return s_fileHandler;
}

void
CFileHandlingCallCenter::setFileHandlingCallback(CALLBACKS_INFO cbi)
{
  m_cbi = cbi;
}

bool
CFileHandlingCallCenter::areFilesHandledExternally(void)
{
  return (
           (NULL != m_cbi.pOpenFileCallback) &&
           (NULL != m_cbi.pIsFileOpenCallback) &&
           (NULL != m_cbi.pFileSeekCallback) &&
           (NULL != m_cbi.pFileTellCallback) &&
           (NULL != m_cbi.pFileReadCallback) &&
           (NULL != m_cbi.pFileWriteCallback) &&
           (NULL != m_cbi.pFileCloseCallback)
          ); 
}

bool
CFileHandlingCallCenter::openfile(const char* pStrFilename, uint32_t nFlags, void** ppAbstractFileDescriptor)
{
  if(m_cbi.pOpenFileCallback)
    return m_cbi.pOpenFileCallback(pStrFilename, nFlags, ppAbstractFileDescriptor, m_cbi.pCallbackToken);
  else
    return false;
}

bool
CFileHandlingCallCenter::isfileopen(void* pAbstractFileDescriptor)
{
  if(m_cbi.pIsFileOpenCallback)
    return m_cbi.pIsFileOpenCallback(pAbstractFileDescriptor, m_cbi.pCallbackToken);
  else
    return false;
}

void
CFileHandlingCallCenter::fileseek(void* pAbstractFileDescriptor, uint64_t nTargetFileOffset, uint32_t nOrigin)
{
  if(m_cbi.pFileSeekCallback)
    return m_cbi.pFileSeekCallback(pAbstractFileDescriptor, nTargetFileOffset, nOrigin, m_cbi.pCallbackToken);
}

uint64_t
CFileHandlingCallCenter::filetell(void* pAbstractFileDescriptor)
{
  if(m_cbi.pFileTellCallback)
    return m_cbi.pFileTellCallback(pAbstractFileDescriptor, m_cbi.pCallbackToken);
  else
    return false;
}

bool
CFileHandlingCallCenter::fileread(void* pAbstractFileDescriptor, char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead)
{
  if(m_cbi.pFileReadCallback)
    return m_cbi.pFileReadCallback(pAbstractFileDescriptor, pBuffer, nBytesToRead, nBytesRead, m_cbi.pCallbackToken);
  else
    return false;
}

bool
CFileHandlingCallCenter::filewrite(void* pAbstractFileDescriptor, char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten)
{
  if(m_cbi.pFileWriteCallback)
    return m_cbi.pFileWriteCallback(pAbstractFileDescriptor, pBuffer, nBytesToWrite, nBytesWritten, m_cbi.pCallbackToken);
  else
    return false;
}

void
CFileHandlingCallCenter::fileclose(void* pAbstractFileDescriptor)
{
  if(m_cbi.pFileCloseCallback)
    return m_cbi.pFileCloseCallback(pAbstractFileDescriptor, m_cbi.pCallbackToken);
}
 
}; // namespace libwamediacommon
