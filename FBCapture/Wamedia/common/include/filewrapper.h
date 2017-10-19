#pragma once
#ifdef STDIO_FILE_HANDLING
#include <stdio.h>
#endif // STDIO_FILE_HANDLING
#include <fstream> // still need to include it for flags etc
#include <string>
using namespace std;
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
using namespace std;

#include "externalfilehandling.h"

#ifdef STDIO_FILE_HANDLING
#define SEEK_FROM_ORIGIN   SEEK_SET
#define SEEK_FROM_END      SEEK_END
#define INPUT_BINARY_MODE  0
#define OUTPUT_BINARY_MODE 1
#define INOUT_BINARY_MODE  2
#else
#define SEEK_FROM_ORIGIN   ifstream::beg
#define SEEK_FROM_END      ifstream::end
#define INPUT_BINARY_MODE  ifstream::in | ifstream::binary
#define OUTPUT_BINARY_MODE ofstream::out | ofstream::binary
#define INOUT_BINARY_MODE  ios_base::in | ios_base::out | ios_base::binary | ios_base::ate
#endif // STDIO_FILE_HANDLING
namespace libwamediacommon
{

class IFileReader
{
public:
  virtual ~IFileReader(){}; // some compilers with some strict compiler flags cry about it missing
  virtual void seek(uint64_t nTargetFileOffset, uint32_t nOrigin) = 0;
  virtual uint64_t tell(void) = 0;
  virtual bool read(char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead) = 0;
};

class IFileWriter
{
public:
  virtual ~IFileWriter(){}; // some compilers with some strict compiler flags cry about it missing
  virtual void seek(uint64_t nTargetFileOffset, uint32_t nOrigin) = 0;
  virtual uint64_t tell(void) = 0;
  virtual bool write(char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten) = 0;
};

class IFileReaderWriter
{
public:
  virtual ~IFileReaderWriter(){}; // some compilers with some strict compiler flags cry about it missing
  virtual void seekg(uint64_t nTargetFileOffset, uint32_t nOrigin) = 0;
  virtual void seekp(uint64_t nTargetFileOffset, uint32_t nOrigin) = 0;
  virtual uint64_t tellg(void) = 0;
  virtual uint64_t tellp(void) = 0;
  virtual bool read(char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead) = 0;
  virtual bool write(char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten) = 0;
};

bool transferBytesFromInputFileOffset(IFileReader* pReader,
                                      IFileWriter* pWriter,
                                      char* pTransferBuffer,
                                      uint64_t nInputFileOffset,
                                      uint64_t nBytes);

class CInputFileWrapper : public IFileReader
{
public:
  CInputFileWrapper();
  virtual ~CInputFileWrapper();

public:
  bool open(const char* pStrFilename, uint32_t nOpenFlags);
  bool isOpen(void);
  void seek(uint64_t nTargetFileOffset, uint32_t nOrigin);
  uint64_t tell(void);
  bool read(char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead);
  void close(void);

protected:
  void*    m_pAbstractFileDescriptor;
#ifdef STDIO_FILE_HANDLING
  FILE*    m_inputFile;
#else
  ifstream m_inputFile;
#endif // STDIO_FILE_HANDLING

private:
  void cleanup(void);
};

class COutputFileWrapper : public IFileWriter
{
public:
  COutputFileWrapper();
  virtual ~COutputFileWrapper();

public:
  bool open(const char* pStrFilename, uint32_t nOpenFlags);
  bool isOpen(void);
  void seek(uint64_t nTargetFileOffset, uint32_t nOrigin);
  uint64_t tell(void);
  bool write(char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten);
  void close(void);

protected:
  void*    m_pAbstractFileDescriptor;
#ifdef STDIO_FILE_HANDLING
  FILE*    m_outputFile;
#else
  ofstream m_outputFile;
#endif // STDIO_FILE_HANDLING

private:
  void cleanup(void);
};

class CInOutFileWrapper : public IFileReaderWriter
{
public:
  CInOutFileWrapper();
  virtual ~CInOutFileWrapper();

public:
  bool open(const char* pStrFilename, uint32_t nOpenFlags);
  bool isOpen(void);
  void seekg(uint64_t nTargetFileOffset, uint32_t nOrigin);
  void seekp(uint64_t nTargetFileOffset, uint32_t nOrigin);
  uint64_t tellg(void);
  uint64_t tellp(void);
  bool read(char* pBuffer, uint32_t nBytesToRead, uint32_t & nBytesRead);
  bool write(char* pBuffer, uint32_t nBytesToWrite, uint32_t & nBytesWritten);
  void close(void);

protected:
  void*    m_pAbstractFileDescriptor;
#ifdef STDIO_FILE_HANDLING
  FILE*    m_inoutFile;
#else
  fstream  m_inoutFile;
#endif // STDIO_FILE_HANDLING

private:
  void cleanup(void);
};

}; // namespace libwamediacommon
