#include "bitpool.h"
#include <string.h>

namespace libwamediacommon
{

CBitPool::CBitPool(IFileReader* pReader)
  : m_nBitOffset(0)
  , m_pReader(pReader)
{
}

CBitPool::~CBitPool()
{

}

void CBitPool::setBitOffset(uint32_t dwBitOffset)
{ 
  m_nBitOffset = dwBitOffset;
}

bool CBitPool::getNextBit(char* lpbBit)
{
  if((lpbBit == NULL) || (NULL == m_pReader))
    return false;

  uint32_t nPos = m_pReader->tell();
  char chTemp;
  // getting one bit = peek the current byte value, but do not advance
  // file pointer as of yet
  uint32_t nBytesRead = 0;
  if(false == m_pReader->read(&chTemp, sizeof(char), nBytesRead))
  {
    ERROR(" CBitPool failed reading 1 byte");
    return false;
  }
  m_pReader->seek(nPos, SEEK_FROM_ORIGIN);

  char chMask = (1 << (BITS_PER_BYTE - 1 - m_nBitOffset));
  *lpbBit = (chTemp & chMask) ? 1 : 0;

  m_nBitOffset++;
  if((m_nBitOffset % BITS_PER_BYTE) == 0)
  {
    // for the next bit we need to advance file pointer 
    if(false == m_pReader->read(&chTemp, sizeof(char), nBytesRead))
    {
      ERROR(" CBitPool failed reading 1 byte");
      return false;
    }
    m_nBitOffset = 0;
  }

  return true;
}

bool CBitPool::getNextNBits(void* lpbBits, uint32_t nBits)
{
  if(lpbBits == NULL || nBits > 32)
    return false;

  char chTemp = 0;
  if(nBits <= 8)
  {
    char chRetValue = 0;
    char chMask;
    for(uint32_t i = 0; i < nBits; i++)
    {
      chMask = 1 << (nBits - i - 1);
      getNextBit(&chTemp);
      chTemp &= 1;
      if(chTemp)
        chRetValue |= chMask;
    }
    *((char*)lpbBits) = chRetValue;
  }
  else if(nBits <= 16)
  {
    uint16_t shRetValue = 0;
    uint16_t shMask;
    for(uint32_t i = 0; i < nBits; i++)
    {
      shMask = 1 << (nBits - i - 1);
      getNextBit(&chTemp);
      chTemp &= 1;
      if(chTemp)
        shRetValue |= shMask;
    }
    *((uint16_t*)lpbBits) = shRetValue;
  }
  else if(nBits <= 32)
  {
    uint32_t nRetValue = 0;
    uint32_t nMask;
    for(uint32_t i = 0; i < nBits; i++)
    {
      nMask = 1 << (nBits - i - 1);
      getNextBit(&chTemp);
      chTemp &= 1;
      if(chTemp)
        nRetValue |= nMask;
    }
    *((uint32_t*)lpbBits) = nRetValue;
  }
  return true;
}

CMemBitPool::CMemBitPool(const uint8_t* pBuffer, uint32_t nBytes)
  : m_pBuffer(pBuffer)
  , m_nBytes(nBytes)
  , m_nBitOffset(0)
{
	
}

CMemBitPool::~CMemBitPool()
{
	
}

void
CMemBitPool::setBitOffset(uint32_t nBitOffset)
{
  m_nBitOffset = nBitOffset;	
}

bool 
CMemBitPool::Read(void* pBuffer, uint32_t nBytes)
{
  uint32_t nByteOffset = m_nBitOffset/BITS_PER_BYTE;
  m_nBitOffset  = nByteOffset*BITS_PER_BYTE;
  if((nByteOffset + nBytes) > m_nBytes)
    return false;

  memcpy(pBuffer, m_pBuffer + nByteOffset, nBytes);
  m_nBitOffset  += nBytes*BITS_PER_BYTE;
  return true;
}

bool 
CMemBitPool::getNextBit(char* lpbBit)
{
  if(lpbBit == NULL)
    return false;

  uint32_t nCurrentByteOffset = m_nBitOffset/BITS_PER_BYTE;
  if(nCurrentByteOffset >= m_nBytes)
    return false;
    
  uint8_t chCurrentByte = m_pBuffer[nCurrentByteOffset];

  uint8_t nIntraByteOffset = (m_nBitOffset % BITS_PER_BYTE);
  uint32_t nMask = (1 << (BITS_PER_BYTE - 1 - nIntraByteOffset));
  *lpbBit = (chCurrentByte & nMask) ? 1 : 0;

  m_nBitOffset++;
  return true;
}

bool 
CMemBitPool::getNextNBits(void* lpbBits, uint32_t nBits)
{
  if(lpbBits == NULL || nBits > 32)
    return false;

  char chTemp = 0;
  if(nBits <= 8)
  {
    char chRetValue = 0;
    char chMask;
    for(uint32_t i = 0; i < nBits; i++)
    {
      chMask = 1 << (nBits - i - 1);
      getNextBit(&chTemp);
      chTemp &= 1;
      if(chTemp)
        chRetValue |= chMask;
    }
    *((char*)lpbBits) = chRetValue;
  }
  else if(nBits <= 16)
  {
    uint16_t shRetValue = 0;
    uint16_t shMask;
    for(uint32_t i = 0; i < nBits; i++)
    {
      shMask = 1 << (nBits - i - 1);
      getNextBit(&chTemp);
      chTemp &= 1;
      if(chTemp)
        shRetValue |= shMask;
    }
    *((uint16_t*)lpbBits) = shRetValue;
  }
  else if(nBits <= 32)
  {
    uint32_t nRetValue = 0;
    uint32_t nMask;
    for(uint32_t i = 0; i < nBits; i++)
    {
      nMask = 1 << (nBits - i - 1);
      getNextBit(&chTemp);
      chTemp &= 1;
      if(chTemp)
        nRetValue |= nMask;
    }
    *((uint32_t*)lpbBits) = nRetValue;
  }
  return true;
}

}; // namespace libwamediacommon

