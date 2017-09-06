#include "performancetime.h"
#include <stdio.h>
#include <string.h>

namespace libwamediacommon
{

#define TIME_SNPRINTF_BUFFER_BYTES (14)

#define SECONDS_PER_MINUTE (60)
#define MINUTES_PER_HOUR   (60)
#define SECONDS_PER_HOUR   (MINUTES_PER_HOUR*SECONDS_PER_MINUTE)
#define NANOSECONDS_PER_MILLISECOND (1000*1000)

CPerformanceTime::CPerformanceTime()
{
  memset(&m_startTime, 0, sizeof(WA_PERF_TIME));
}

CPerformanceTime::~CPerformanceTime()
{

}

bool CPerformanceTime::startTimekeeping(GETCURRENTTIME pGetCurrentTime)
{
  if(NULL == pGetCurrentTime)
    return false;

  m_pGetCurrentTime = pGetCurrentTime;

  return getCurrentTime(m_startTime);
}
 
bool CPerformanceTime::getCurrentTime(WA_PERF_TIME & now)
{ 
  if(NULL == m_pGetCurrentTime)
    return false;

  now = m_pGetCurrentTime(); // clock_gettime(CLOCK_REALTIME, &now);
  return true;
}

string CPerformanceTime::getElapsedHrMinSec(WA_PERF_TIME now)
{
  WA_PERF_TIME difference = timeDifference(now, m_startTime);

  uint32_t nSeconds = difference.m_nSeconds;
  uint32_t nNanoseconds = difference.m_nNanoSeconds;
  uint32_t nHours  = nSeconds / SECONDS_PER_HOUR;
  nSeconds -= nHours*SECONDS_PER_HOUR;
  uint32_t nMinutes = nSeconds / SECONDS_PER_MINUTE;
  nSeconds -= nMinutes*SECONDS_PER_MINUTE;
  uint32_t nMilliseconds = nNanoseconds / NANOSECONDS_PER_MILLISECOND;

  char chStrTmp[TIME_SNPRINTF_BUFFER_BYTES];
  memset(chStrTmp, 0, TIME_SNPRINTF_BUFFER_BYTES*sizeof(char));
  snprintf(chStrTmp, TIME_SNPRINTF_BUFFER_BYTES, "%02d:%02d:%02d:%03d",
           nHours, nMinutes, nSeconds, nMilliseconds);
  string strTime = string(chStrTmp);
  return strTime;
}

WA_PERF_TIME CPerformanceTime::timeDifference(WA_PERF_TIME now, WA_PERF_TIME start)
{
  WA_PERF_TIME temp;
  if ((now.m_nNanoSeconds-start.m_nNanoSeconds)<0) {
    temp.m_nSeconds = now.m_nSeconds-start.m_nSeconds-1;
    temp.m_nNanoSeconds = 1000000000+now.m_nNanoSeconds-start.m_nNanoSeconds;
  }
  else
  {
    temp.m_nSeconds = now.m_nSeconds-start.m_nSeconds;
    temp.m_nNanoSeconds = now.m_nNanoSeconds-start.m_nNanoSeconds;
  }
  return temp;
}

}; // namespace libwamediacommon
