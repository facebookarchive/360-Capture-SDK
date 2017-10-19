#pragma once

#ifdef DESKTOP_BUILD_ID

#ifdef __cplusplus
namespace libwamediacommon
{

extern "C"
{
#endif // __cplusplus

const char* getDesktopBuildInfo();

#ifdef __cplusplus
}

}; // namespace libwamediacommon
#endif // __cplusplus

#endif // DESKTOP_BUILD_ID

