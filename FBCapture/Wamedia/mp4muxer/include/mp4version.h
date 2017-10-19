#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

typedef struct
{
  uint32_t nMajorVersion;
  uint32_t nMinorVersion;
  uint32_t nReleaseVersion;
  uint32_t nAuthoringModule;
} MP4_OPERATIONS_VERSION_INFO;

uint32_t insertVersionTag(const char* pStrInputFilename, const char* pStrOutputFilename);
void printVersionInfo(const char* pStrPreamble, MP4_OPERATIONS_VERSION_INFO ver);
bool isFileVersioned(MP4_OPERATIONS_VERSION_INFO versionInfo);
bool isVersioningUpdateRequired(MP4_OPERATIONS_VERSION_INFO fileVersionInfo);

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

