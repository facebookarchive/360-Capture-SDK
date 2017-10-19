#pragma once

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string>
using namespace std;

#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR '/'
#endif // PATH_SEPARATOR

#ifdef DO_NOT_USE_OSTRINGSTREAM
#define SNPRINTF_BUFFER_BYTES (80)
#endif // DO_NOT_USE_OSTRINGSTREAM

#define UNUSED(x)             (void)(x)

#define INVALID_INDEX                   ((uint32_t)-1)
#define BITS_PER_BYTE                   (8)

#define SMALL_FLOATING_POINT_EPSILON    (0.00001f)

#define MILLISECONDS_PER_SECOND         (1000)
#define MICROSECONDS_PER_SECOND         (1000000)
#define HUNDRED_NANOSECONDS_PER_SECOND  (10*MICROSECONDS_PER_SECOND)
#define SECONDS_PER_MINUTE              (60)
#define MILLISECONDS_PER_MINUTE         (SECONDS_PER_MINUTE*MILLISECONDS_PER_SECOND)
#define MINUTES_PER_HOUR                (60)
#define SECONDS_PER_HOUR                (SECONDS_PER_MINUTE*MINUTES_PER_HOUR)
#define MILLISECONDS_PER_HOUR           (SECONDS_PER_HOUR*MILLISECONDS_PER_SECOND)
                        
#define SECONDS_PER_DAY                 (HOURS_PER_DAY*MINUTES_PER_HOUR*SECONDS_PER_MINUTE)  
#define HOURS_PER_DAY                   (24)
#define MONTHS_PER_YEAR                 (12)
#define DAYS_PER_REGULAR_YEAR           (365)
#define DAYS_PER_LEAP_YEAR              (366)
#define SECONDS_PER_REGULAR_YEAR        (DAYS_PER_REGULAR_YEAR*HOURS_PER_DAY*MINUTES_PER_HOUR*SECONDS_PER_MINUTE)
#define SECONDS_PER_LEAP_YEAR           (DAYS_PER_LEAP_YEAR*HOURS_PER_DAY*MINUTES_PER_HOUR*SECONDS_PER_MINUTE)
#define SECONDS_PER_FOUR_YEARS          (3*(SECONDS_PER_REGULAR_YEAR) + SECONDS_PER_LEAP_YEAR)

#define LEFT_CHANNEL_INDEX              (0)
#define RIGHT_CHANNEL_INDEX             (1)
#define MONO_CHANNEL_INDEX              (LEFT_CHANNEL_INDEX)

#define BLOCK_TRANSFER_BYTES            (1024)

#define CHARS_TO_INT32(a, b, c, d) ((d << 24) | (c << 16) | (b << 8) | a)

#define BIT_MASK_1_BIT                  (0x0001)
#define BIT_MASK_2_BITS                 (0x0003)
#define BIT_MASK_3_BITS                 (0x0007)
#define BIT_MASK_4_BITS                 (0x000F)
#define BIT_MASK_5_BITS                 (0x001F)
#define BIT_MASK_6_BITS                 (0x003F)
#define BIT_MASK_7_BITS                 (0x007F)
#define BIT_MASK_8_BITS                 (0x00FF)
#define BIT_MASK_9_BITS                 (0x01FF)
#define BIT_MASK_10_BITS                (0x03FF)
#define BIT_MASK_11_BITS                (0x07FF)
#define BIT_MASK_12_BITS                (0x0FFF)
#define BIT_MASK_13_BITS                (0x1FFF)
#define BIT_MASK_14_BITS                (0x3FFF)
#define BIT_MASK_15_BITS                (0x7FFF)
#define BIT_MASK_16_BITS                (0x0000FFFF)
#define BIT_MASK_17_BITS                (0x0001FFFF)
#define BIT_MASK_18_BITS                (0x0003FFFF)
#define BIT_MASK_19_BITS                (0x0007FFFF)
#define BIT_MASK_20_BITS                (0x000FFFFF)
#define BIT_MASK_21_BITS                (0x001FFFFF)
#define BIT_MASK_22_BITS                (0x003FFFFF)
#define BIT_MASK_23_BITS                (0x007FFFFF)
#define BIT_MASK_24_BITS                (0x00FFFFFF)
#define BIT_MASK_25_BITS                (0x01FFFFFF)
#define BIT_MASK_26_BITS                (0x03FFFFFF)
#define BIT_MASK_27_BITS                (0x07FFFFFF)
#define BIT_MASK_28_BITS                (0x0FFFFFFF)
#define BIT_MASK_29_BITS                (0x1FFFFFFF)
#define BIT_MASK_30_BITS                (0x3FFFFFFF)
#define BIT_MASK_31_BITS                (0x7FFFFFFF)
#define BIT_MASK_32_BITS                (0xFFFFFFFF)

#define SAFE_FILE_CLOSE(x)               \
{                                        \
  if((x))                                \
  {                                      \
    fclose((x));                         \
    (x) = NULL;                          \
  }                                      \
}

#define SAFE_DELETE(x)                   \
{                                        \
  if((x))                                \
  {                                      \
    delete (x);                          \
    (x) = NULL;                          \
  }                                      \
}

#define SAFE_DELETE_ARRAY(x)             \
{                                        \
  if((x))                                \
  {                                      \
    delete [] (x);                       \
    (x) = NULL;                          \
  }                                      \
}

typedef enum
{
   ONE_BIT            = 1,
   TWO_BITS,
   THREE_BITS,
   FOUR_BITS,      
   FIVE_BITS,      
   SIX_BITS,       
   SEVEN_BITS,
   EIGHT_BITS,
   NINE_BITS,
   TEN_BITS,
   ELEVEN_BITS,
   TWELVE_BITS,
   THIRTEEN_BITS,
   FOURTEEN_BITS,
   FIFTEEN_BITS,
   SIXTEEN_BITS,
   SEVENTEEN_BITS,
   EIGHTEEN_BITS,
   NINETEEN_BITS,
   TWENTY_BITS,
   TWENTY_ONE_BITS,
   TWENTY_TWO_BITS,
   TWENTY_THREE_BITS,
   TWENTY_FOUR_BITS,
   TWENTY_FIVE_BITS,
   TWENTY_SIX_BITS,
   TWENTY_SEVEN_BITS,
   TWENTY_EIGHT_BITS,
   TWENTY_NINE_BITS,
   THIRTY_BITS,
   THIRTY_ONE_BITS,
   THIRTY_TWO_BITS,
} eBitFields;
