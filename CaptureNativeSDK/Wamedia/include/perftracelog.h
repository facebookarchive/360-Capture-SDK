#pragma once
#include <inttypes.h>
#include <stdbool.h>

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

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef struct
{
  int64_t m_nNanoSeconds;
  int64_t m_nSeconds;
} WA_PERF_TIME; // a.k.a. timespec in *nix world

typedef void (*LOCKACCESS)(void);
typedef void (*UNLOCKACCESS)(void);
typedef WA_PERF_TIME (*GETCURRENTTIME)(void);
typedef bool (*FILEWRITE)(char* pBuffer, uint32_t nBytesToWrite);

typedef struct
{
  LOCKACCESS     pLock;
  UNLOCKACCESS   pUnlock;
  GETCURRENTTIME pGetCurrentTime;
  FILEWRITE      pFileWrite;
  void*          pFileWriteToken;
} TRACING_INFO;

bool initializePerformanceTracing(TRACING_INFO ti);
#ifdef VERIFY_MESSAGE_FORMAT_AGAINST_ERRORS
#include <stdio.h>
#define PERFTRACELOG   printf
#else
void PERFTRACELOG(const char* fmt, ...);
#endif // VERIFY_MESSAGE_FORMAT_AGAINST_ERRORS
uint64_t getTracingQueueBytes();
bool serializeCurrentTracesQueue();
void terminatePerformanceTracing();

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus
}; // namespace libwamediacommon
#endif // __cplusplus
