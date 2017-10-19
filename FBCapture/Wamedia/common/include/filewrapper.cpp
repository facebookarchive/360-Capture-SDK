#include "filewrapper.h"
#include "commonDefinitions.h"
#ifdef STDIO_FILE_HANDLING
#include <errno.h>
#endif // STDIO_FILE_HANDLING
#include "logssender.h"
#include <string.h>
using namespace std;

namespace libwamediacommon
{

bool transferBytesFromInputFileOffset(IFileReader* pReader,
                                      IFileWriter* pWriter,
                                      char* pTransferBuffer,
                                      uint64_t nInputFileOffset,
                                      uint64_t nBytes)
{
  if((NULL == pReader) || (NULL == pWriter) || (NULL == pTransferBuffer))
    return false;

  uint32_t nBlocks = nBytes / BLOCK_TRANSFER_BYTES;
  uint32_t nRemainder = nBytes - nBlocks*BLOCK_TRANSFER_BYTES;

  pReader->seek(nInputFileOffset, SEEK_FROM_ORIGIN);

  bool bSuccess = false;
  do
  {
    uint32_t nBytesRead = 0;
    uint32_t nBytesWritten = 0;
    for(uint32_t i = 0; i < nBlocks; i++)
    {
      if(false == pReader->read(pTransferBuffer, BLOCK_TRANSFER_BYTES, nBytesRead))
      {
        ERROR("Failed reading %d bytes from input file @offset %" PRIu64, BLOCK_TRANSFER_BYTES, pReader->tell());
        break;
      }

      if(false == pWriter->write(pTransferBuffer, BLOCK_TRANSFER_BYTES, nBytesWritten))
      {
        ERROR("Failed writing %d bytes to output file @offset %d", BLOCK_TRANSFER_BYTES, pWriter->tell());
        break;
      }
    }

    if(false == pReader->read(pTransferBuffer, nRemainder, nBytesRead))
    {
      ERROR("Failed reading %d bytes from input file @offset %" PRIu64, BLOCK_TRANSFER_BYTES, pReader->tell());
      break;
    }

    if(false == pWriter->write(pTransferBuffer, nRemainder, nBytesWritten))
    {
      ERROR("Failed writing %d bytes to output file @offset %d", nRemainder, pWriter->tell());
      break;
    }

    bSuccess = true;

  }while(false);

  return bSuccess;
}

CInputFileWrapper::CInputFileWrapper()
  : m_pAbstractFileDescriptor(NULL)
#ifdef STDIO_FILE_HANDLING
  , m_inputFile(NULL)
#endif // STDIO_FILE_HANDLING
{
}

CInputFileWrapper::~CInputFileWrapper()
{
  cleanup();
}

bool
CInputFileWrapper::open(const char* pStrFilename, uint32_t nOpenFlags)
{
  bool bSuccess = false;
  if(areFilesHandledExternally())
  {
    bSuccess = openfile(pStrFilename, nOpenFlags, &m_pAbstractFileDescriptor);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    // Any file opening mode other than "rb" is currently not supported
    // In case we end up needing any other mode, the following needs to
    // be done:
    //  a) define corresponding constant in filewrapper.h
    //  b) insert small translation routine which would convert numeric value
    //     of input to stdio file opening string
    m_inputFile = fopen(pStrFilename, "rb");
    bSuccess = (m_inputFile != NULL);
    if(false == bSuccess)
    {
      INFO("Failed opening file for reading, error = %s", strerror(errno));
    }
#else
    m_inputFile.open(pStrFilename, (ios_base::openmode)nOpenFlags);
    bSuccess = m_inputFile.is_open();
#endif // STDIO_FILE_HANDLING
  }
  return bSuccess;
}

bool
CInputFileWrapper::isOpen(void)
{
  if(areFilesHandledExternally())
  {
    return isfileopen(m_pAbstractFileDescriptor);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    return (NULL != m_inputFile);
#else
    return m_inputFile.is_open();
#endif // STDIO_FILE_HANDLING
  }
}

void
CInputFileWrapper::seek(uint64_t nTargetFileOffset, uint32_t nOrigin)
{
  if(areFilesHandledExternally())
  {
    fileseek(m_pAbstractFileDescriptor, nTargetFileOffset, nOrigin);
  }
  else
#ifdef STDIO_FILE_HANDLING
    fseek(m_inputFile, nTargetFileOffset, nOrigin);
#else
    m_inputFile.seekg(nTargetFileOffset, (ios_base::seekdir)nOrigin);
#endif // STDIO_FILE_HANDLING
}

uint64_t
CInputFileWrapper::tell(void)
{
  if(areFilesHandledExternally())
  {
    return filetell(m_pAbstractFileDescriptor);
  }
  else
#ifdef STDIO_FILE_HANDLING
    return ftell(m_inputFile);
#else
    return (uint64_t)m_inputFile.tellg();
#endif // STDIO_FILE_HANDLING
}

bool
CInputFileWrapper::read(char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead)
{
  bool bSuccess = false;
  if(areFilesHandledExternally())
  {
    nBytesRead = 0;
    bSuccess = fileread(m_pAbstractFileDescriptor, pBuffer, nBytesToRead, nBytesRead);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    nBytesRead = fread(pBuffer, 1, nBytesToRead, m_inputFile);
    bSuccess = (nBytesRead == nBytesToRead);
#else
    m_inputFile.read(pBuffer, nBytesToRead);
    bSuccess = m_inputFile.good();
#endif // STDIO_FILE_HANDLING
    if(false == bSuccess)
    {
#ifdef STDIO_FILE_HANDLING
#else
      nBytesRead = m_inputFile.gcount();
#endif // STDIO_FILE_HANDLING
      return false;
    }
  }
  return bSuccess;
}

void
CInputFileWrapper::close(void)
{
  if(areFilesHandledExternally())
  {
    fileclose(m_pAbstractFileDescriptor);
    m_pAbstractFileDescriptor = NULL;
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    SAFE_FILE_CLOSE(m_inputFile);
#else
    if(m_inputFile.is_open())
      m_inputFile.close();
#endif // STDIO_FILE_HANDLING
  }
}

void
CInputFileWrapper::cleanup()
{
  close();
}

// OUTPUT FILE WRAPPER
COutputFileWrapper::COutputFileWrapper()
  : m_pAbstractFileDescriptor(NULL)
#ifdef STDIO_FILE_HANDLING
  , m_outputFile(NULL)
#endif // STDIO_FILE_HANDLING
{
}

COutputFileWrapper::~COutputFileWrapper()
{
  cleanup();
}

bool
COutputFileWrapper::open(const char* pStrFilename, uint32_t nOpenFlags)
{
  bool bSuccess = false;
  if(areFilesHandledExternally())
  {
    bSuccess = openfile(pStrFilename, nOpenFlags, &m_pAbstractFileDescriptor);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    // Any file opening mode other than "wb" is currently not supported
    // In case we end up needing any other mode, the following needs to
    // be done:
    //  a) define coresponding constant in filewrapper.h
    //  b) insert small translation routine which would convert numeric value
    //     of input to stdio file opening string
    m_outputFile = fopen(pStrFilename, "wb");
    bSuccess = (m_outputFile != NULL);
    if(false == bSuccess)
    {
      INFO("Failed opening file for writing, error = %s", strerror(errno));
    }
#else
    m_outputFile.open(pStrFilename, (ios_base::openmode)nOpenFlags);
    bSuccess = m_outputFile.is_open();
#endif // STDIO_FILE_HANDLING
  }
  return bSuccess;
}

bool
COutputFileWrapper::isOpen(void)
{
  if(areFilesHandledExternally())
  {
    return isfileopen(m_pAbstractFileDescriptor);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    return (m_outputFile != NULL);
#else
    return m_outputFile.is_open();
#endif // STDIO_FILE_HANDLING
  }
}

void
COutputFileWrapper::seek(uint64_t nTargetFileOffset, uint32_t nOrigin)
{
  if(areFilesHandledExternally())
  {
    fileseek(m_pAbstractFileDescriptor, nTargetFileOffset, nOrigin);
  }
  else
#ifdef STDIO_FILE_HANDLING
    fseek(m_outputFile, nTargetFileOffset, nOrigin);
#else
    m_outputFile.seekp(nTargetFileOffset, (ios_base::seekdir)nOrigin);
#endif // STDIO_FILE_HANDLING
}

uint64_t
COutputFileWrapper::tell(void)
{
  if(areFilesHandledExternally())
  {
    return filetell(m_pAbstractFileDescriptor);
  }
  else
#ifdef STDIO_FILE_HANDLING
    return ftell(m_outputFile);
#else
    return (uint64_t)m_outputFile.tellp();
#endif // STDIO_FILE_HANDLING
}

bool
COutputFileWrapper::write(char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten)
{
  bool bSuccess = false;
  if(areFilesHandledExternally())
  {
    nBytesWritten = 0;
    bSuccess = filewrite(m_pAbstractFileDescriptor, pBuffer, nBytesToWrite, nBytesWritten);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    nBytesWritten = fwrite(pBuffer, 1, nBytesToWrite, m_outputFile);
    bSuccess = (nBytesWritten == nBytesToWrite);
#else
    m_outputFile.write(pBuffer, nBytesToWrite);
    bSuccess = m_outputFile.good();
#endif // STDIO_FILE_HANDLING
    if(false == bSuccess)
    {
      nBytesWritten = 0; // white lie, ofstream has no equivalent of gcount();
      return false;
    }
  }
  return bSuccess;
}

void
COutputFileWrapper::close(void)
{
  if(areFilesHandledExternally())
  {
    fileclose(m_pAbstractFileDescriptor);
    m_pAbstractFileDescriptor = NULL;
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    SAFE_FILE_CLOSE(m_outputFile);
#else
    if(m_outputFile.is_open())
      m_outputFile.close();
#endif // STDIO_FILE_HANDLING
  }
}

void
COutputFileWrapper::cleanup()
{
  close();
}

// IN/OUT FILE WRAPPER

CInOutFileWrapper::CInOutFileWrapper()
  : m_pAbstractFileDescriptor(NULL)
#ifdef STDIO_FILE_HANDLING
  , m_inoutFile(NULL)
#endif // STDIO_FILE_HANDLING
{
}

CInOutFileWrapper::~CInOutFileWrapper()
{
  cleanup();
}

bool
CInOutFileWrapper::open(const char* pStrFilename, uint32_t nOpenFlags)
{
  bool bSuccess = false;
  if(areFilesHandledExternally())
  {
    bSuccess = openfile(pStrFilename, nOpenFlags, &m_pAbstractFileDescriptor);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    // Any file opening mode other than "r+b" is currently not supported
    // In case we end up needing any other mode, the following needs to
    // be done:
    //  a) define corresponding constant in filewrapper.h
    //  b) insert small translation routine which would convert numeric value
    //     of input to stdio file opening string
    m_inoutFile = fopen(pStrFilename, "r+b");
    bSuccess = (m_inoutFile != NULL);
    if(false == bSuccess)
    {
      INFO("Failed opening file for modifying, error = %s", strerror(errno));
    }
#else
    m_inoutFile.open(pStrFilename, (ios_base::openmode)nOpenFlags);
    bSuccess = m_inoutFile.is_open();
#endif // STDIO_FILE_HANDLING
  }
  return bSuccess;
}

bool
CInOutFileWrapper::isOpen(void)
{
  if(areFilesHandledExternally())
  {
    return isfileopen(m_pAbstractFileDescriptor);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    return (m_inoutFile != NULL);
#else
    return m_inoutFile.is_open();
#endif // STDIO_FILE_HANDLING
  }
}

void
CInOutFileWrapper::seekg(uint64_t nTargetFileOffset, uint32_t nOrigin)
{
  if(areFilesHandledExternally())
  {
    fileseek(m_pAbstractFileDescriptor, nTargetFileOffset, nOrigin);
  }
  else
#ifdef STDIO_FILE_HANDLING
    fseek(m_inoutFile, nTargetFileOffset, nOrigin);
#else
    m_inoutFile.seekg(nTargetFileOffset, (ios_base::seekdir)nOrigin);
#endif // STDIO_FILE_HANDLING
}

void
CInOutFileWrapper::seekp(uint64_t nTargetFileOffset, uint32_t nOrigin)
{
  if(areFilesHandledExternally())
  {
    fileseek(m_pAbstractFileDescriptor, nTargetFileOffset, nOrigin);
  }
  else
#ifdef STDIO_FILE_HANDLING
    fseek(m_inoutFile, nTargetFileOffset, nOrigin);
#else
    m_inoutFile.seekp(nTargetFileOffset, (ios_base::seekdir)nOrigin);
#endif // STDIO_FILE_HANDLING
}

uint64_t
CInOutFileWrapper::tellg(void)
{
  if(areFilesHandledExternally())
  {
    return filetell(m_pAbstractFileDescriptor);
  }
  else
#ifdef STDIO_FILE_HANDLING
    return ftell(m_inoutFile);
#else
    return (uint64_t)m_inoutFile.tellg();
#endif // STDIO_FILE_HANDLING
}

uint64_t
CInOutFileWrapper::tellp(void)
{
  if(areFilesHandledExternally())
  {
    return filetell(m_pAbstractFileDescriptor);
  }
  else
#ifdef STDIO_FILE_HANDLING
    return ftell(m_inoutFile);
#else
    return (uint64_t)m_inoutFile.tellp();
#endif // STDIO_FILE_HANDLING
}

bool
CInOutFileWrapper::read(char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead)
{
  bool bSuccess = false;
  if(areFilesHandledExternally())
  {
    nBytesRead = 0;
    bSuccess = fileread(m_pAbstractFileDescriptor, pBuffer, nBytesToRead, nBytesRead);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    nBytesRead = fread(pBuffer, 1, nBytesToRead, m_inoutFile);
    bSuccess = (nBytesRead == nBytesToRead);
    if(bSuccess)
      fflush(m_inoutFile);
#else
    m_inoutFile.read(pBuffer, nBytesToRead);
    bSuccess = m_inoutFile.good();
#endif // STDIO_FILE_HANDLING
    if(false == bSuccess)
    {
#ifdef STDIO_FILE_HANDLING
#else
      nBytesRead = m_inoutFile.gcount();
#endif // STDIO_FILE_HANDLING
      return false;
    }
  }
  return bSuccess;
}

bool
CInOutFileWrapper::write(char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten)
{
  bool bSuccess = false;
  if(areFilesHandledExternally())
  {
    nBytesWritten = 0;
    bSuccess = filewrite(m_pAbstractFileDescriptor, pBuffer, nBytesToWrite, nBytesWritten);
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    nBytesWritten = fwrite(pBuffer, 1, nBytesToWrite, m_inoutFile);
    bSuccess = (nBytesWritten == nBytesToWrite);
    if(bSuccess)
      fflush(m_inoutFile);
#else
    m_inoutFile.write(pBuffer, nBytesToWrite);
    bSuccess = m_inoutFile.good();
#endif // STDIO_FILE_HANDLING  
    if(false == bSuccess)
    {
      nBytesWritten = 0; // white lie, ofstream has no equivalent of gcount();
      return false;
    }
  }
  return bSuccess;
}

void
CInOutFileWrapper::close(void)
{
  if(areFilesHandledExternally())
  {
    fileclose(m_pAbstractFileDescriptor);
    m_pAbstractFileDescriptor = NULL;
  }
  else
  {
#ifdef STDIO_FILE_HANDLING
    SAFE_FILE_CLOSE(m_inoutFile);
#else
    if(m_inoutFile.is_open())
      m_inoutFile.close();
#endif // STDIO_FILE_HANDLING
  }
}

void
CInOutFileWrapper::cleanup()
{
  close();
}

}; // namespace libwamediacommon
