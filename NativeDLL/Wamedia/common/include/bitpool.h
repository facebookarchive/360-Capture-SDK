#pragma once
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "filewrapper.h"
#include "logssender.h"

#define BITS_PER_BYTE    (8)

namespace libwamediacommon
{

class CBitPool
{
public:
  CBitPool(IFileReader* pReader);
  virtual ~CBitPool();

public:
  // Attributes
  void setBitOffset(uint32_t nBitOffset);
  uint32_t getBitOffset(void){ return m_nBitOffset;}

public:
  // Operations
  bool getNextNBits(void* lpbBits, uint32_t nBits); // user is responsible for providing storage
  bool getNextBit(char* lpbBit);

protected:
  uint32_t m_nBitOffset; // should always be in the value range [0,7]
  IFileReader* m_pReader;
};

class CMemBitPool
{
public:
  CMemBitPool(const uint8_t* pBuffer, uint32_t nBytes);
  virtual ~CMemBitPool();

public:
  // Attributes
  void setBitOffset(uint32_t nBitOffset);
  uint32_t getBitOffset(void){ return m_nBitOffset;}

public:
  // Operations
  bool Read(void* pBuffer, uint32_t nBytes);
  bool getNextNBits(void* lpbBits, uint32_t nBits); // user is responsible for providing storage
  bool getNextBit(char* lpbBit);

protected:
  const uint8_t* m_pBuffer;
  const uint32_t m_nBytes;
  uint32_t m_nBitOffset; // should always be in the value range [0,7]
};

}; // namespace libwamediacommon

