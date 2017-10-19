#pragma once
#include "mp4repairdefinitions.h"

#ifdef __cplusplus
namespace libwamediacommon
{

extern "C"
{
#endif //__cplusplus

typedef void (*PROBLEM_REPORTING_CALLBACK)(CMp4ProblemInfo problemInfo, void* pCallbackToken);

typedef void* HTHRDREPAIR;
typedef HTHRDREPAIR (*IDENTIFY_REPAIR_THREAD_CALLBACK)(void* user_data);

HTHRDREPAIR attachCurrentThreadToRepairDispatch(IDENTIFY_REPAIR_THREAD_CALLBACK pCurrentThreadRepairCB,
                                                void* pCurrentThreadRepairCBToken);
void detachCurrentThreadFromRepairDispatch(HTHRDREPAIR hThrdrepair);

void setCurrentThreadCheckAndRepairCB(PROBLEM_REPORTING_CALLBACK cb, void* user_data);

#ifdef __cplusplus
} // extern "C"

}; // namespace libwamediacommon
#endif //__cplusplus

