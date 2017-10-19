#pragma once
#include "logsrecipient.h"

#ifdef __cplusplus
namespace libwamediacommon
{
#endif // __cplusplus
// There is a potential pitfall which must be avoided:
//  - the logger functions are variadic in nature, but
//    ultimately designed to snugly fit the requirements
//    of printf/fprintf.
//
//  - if we don't pay strict attention that our formatting
//    messages do not satisfy strict printf requirements,
//    the code may very well compile, but it may wreak havoc
//    at run time.
//
// In order to avoid your misconstrued debug message crashing
// the app at run time, before checking in your code, please
//   a) uncomment the line below, i.e enable the preprocessor
//      definition VERIFY_MESSAGE_FORMAT_AGAINST_ERRORS
//   b) make sure that the code compiles without any warnings
//   c) comment out the line below (otherwise, the logger will
//      not work as expected.
//
//#define VERIFY_MESSAGE_FORMAT_AGAINST_ERRORS

#ifdef DEBUG
#undef DEBUG
#endif

#ifdef VERIFY_MESSAGE_FORMAT_AGAINST_ERRORS

#include <stdio.h>

#define DEBUG   printf
#define INFO    printf
#define NOTICE  printf
#define WARNING printf
#define ERROR   printf

#define DEBUG_R   printf
#define INFO_R    printf
#define NOTICE_R  printf
#define WARNING_R printf
#define ERROR_R   printf

#else

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define REPEATED_MESSAGES_START "Repeated Messages Start"
#define REPEATED_MESSAGES_END   "Repeated Messages End"

void DEBUG(const char* fmt, ...);
void INFO(const char* fmt, ...);
void NOTICE(const char* fmt, ...);
void WARNING(const char* fmt, ...);
void ERROR(const char* fmt, ...);

void DEBUG_R(uint32_t nSourceFileLineNumber, const char* fmt, ...);
void INFO_R(uint32_t nSourceFileLineNumber, const char* fmt, ...);
void NOTICE_R(uint32_t nSourceFileLineNumber, const char* fmt, ...);
void WARNING_R(uint32_t nSourceFileLineNumber, const char* fmt, ...);
void ERROR_R(uint32_t nSourceFileLineNumber, const char* fmt, ...);

void REPORT_COMPLETION_PERCENT(uint32_t nPercent);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

#ifdef __cplusplus
}; // namespace libwamediacommon
#endif // __cplusplus
