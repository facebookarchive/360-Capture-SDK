#include "utilities.h"
#include <math.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string.h>
using namespace std;

#include "logssender.h"

#ifdef DESKTOP_BUILD_ID

#include "desktopbuildinfo.h"

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)
#define WAMEDIA_BUILD_ID STRINGIZE(DESKTOP_BUILD_ID)
#endif // DESKTOP_BUILD_ID

namespace libwamediacommon
{

uint16_t swapBytes16(uint16_t nInput)
{
  int16_t b0 =  (nInput & BIT_MASK_8_BITS);
  int16_t b1 = ((nInput >> 8) & BIT_MASK_8_BITS);
  return ((b0 << 8) | b1) ;
}

uint32_t swapBytes32(uint32_t nInput)
{
  int32_t b0 = ( nInput        & BIT_MASK_8_BITS);
  int32_t b1 = ((nInput >>  8) & BIT_MASK_8_BITS);
  int32_t b2 = ((nInput >> 16) & BIT_MASK_8_BITS);
  int32_t b3 = ((nInput >> 24) & BIT_MASK_8_BITS);
  return ((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
}

uint64_t swapBytes64(uint64_t nInput)
{
  uint32_t nLower = nInput & BIT_MASK_32_BITS;
  nInput >>= 32;
  uint32_t nUpper = nInput & BIT_MASK_32_BITS;
  
  nLower = swapBytes32(nLower);
  nUpper = swapBytes32(nUpper);
   
  uint64_t nRetValue = (nLower & BIT_MASK_32_BITS);
  nRetValue <<= 32;
  nRetValue |= (nUpper & BIT_MASK_32_BITS);

  return nRetValue;
}

string fourCharsToUTF8String(uint32_t nBoxType)
{
  uint8_t chType[4];
  memcpy(chType, &nBoxType, sizeof(uint32_t));

#ifndef DO_NOT_USE_OSTRINGSTREAM
  ostringstream os;
  for(uint32_t i = 0; i < sizeof(chType)/sizeof(uint8_t); i++)
  {
    if((chType[i] < 0x20) || (chType[i] > 0x7F))
      os << "<" << setw(2) << setfill('0') << uppercase << hex << (int)(chType[i] & 0xFF) << ">";
    else
      os << chType[i];
  }
  string strRetValue = os.str();
  return strRetValue;
#else
  string strRetValue;
  char chStrTmp[SNPRINTF_BUFFER_BYTES];
  for(uint32_t i = 0; i < sizeof(chType)/sizeof(uint8_t); i++)
  {
    memset(chStrTmp, 0, SNPRINTF_BUFFER_BYTES*sizeof(char));
    if((chType[i] < 0x20) || (chType[i] > 0x7F))
      snprintf(chStrTmp, SNPRINTF_BUFFER_BYTES, "<%02X>", (int)(chType[i] & 0xFF));
    else
      snprintf(chStrTmp, SNPRINTF_BUFFER_BYTES, "%c", chType[i]);

    strRetValue += string(chStrTmp);
  }
  return strRetValue;
#endif // DO_NOT_USE_OSTRINGSTREAM
}

float convertFixed32BitToFloat(int32_t nInput, uint32_t nPrecision)
{
  if(nPrecision > 31)
    return (float)(0xFFFFFFFF);

  int32_t nWholeNumber = nInput;
  nWholeNumber       >>= nPrecision;

  float fFractionSupremum = (float)pow(2.0f, (int32_t)nPrecision);
  int nFractionBitMask    = (uint32_t)fFractionSupremum - 1;
  int nFraction           = nInput & nFractionBitMask;

  float fRetValue = nWholeNumber + nFraction/fFractionSupremum;
  return fRetValue;
}

uint32_t byteOffset(uint32_t nOffset)
{
  return nOffset/BITS_PER_BYTE;
}

uint32_t extraBitsOffset(uint32_t nOffset)
{
  return nOffset % BITS_PER_BYTE;
}

#ifdef DESKTOP_BUILD_ID
const char* getDesktopBuildInfo()
{
  return WAMEDIA_BUILD_ID;
}
#endif // DESKTOP_BUILD_ID

}; // namespace libwamediacommon
