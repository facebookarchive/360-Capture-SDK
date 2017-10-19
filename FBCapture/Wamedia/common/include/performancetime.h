#pragma once
#include "perftracelog.h"
#include <string>
using namespace std;

namespace libwamediacommon
{

class CPerformanceTime
{
public:
    CPerformanceTime();
    ~CPerformanceTime();

public:
    bool startTimekeeping(GETCURRENTTIME pGetCurrentTime);
    bool getCurrentTime(WA_PERF_TIME & now);
    string getElapsedHrMinSec(WA_PERF_TIME now);

private:
    WA_PERF_TIME timeDifference(WA_PERF_TIME now, WA_PERF_TIME start);
   
private:
    WA_PERF_TIME   m_startTime;
    GETCURRENTTIME m_pGetCurrentTime;
};

}; // namespace libwamediacommon
