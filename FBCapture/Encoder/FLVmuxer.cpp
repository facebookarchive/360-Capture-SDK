/****************************************************************************************************************

Filename	:	Muxing.cpp
Content		:
Created		:
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/
#include "FLVmuxer.h"

namespace FBCapture {
  namespace Mux {

    FLVMuxer::FLVMuxer() {}

    FLVMuxer::~FLVMuxer() {
      remove(videoFile_.c_str());  // Remove h264 video file
      remove(audioFile_.c_str());  // Remove aac audio file
    }

    string FLVMuxer::changeFormatString(const string& path, const string& oldFormat, const string& newFormat) {
      string newPath(path);

      newPath.erase(path.length() - oldFormat.length(), oldFormat.length());
      newPath += newFormat;

      return newPath;
    }

    FBCAPTURE_STATUS FLVMuxer::muxingMedia(const wstring& videoFile, const string& audioFile) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
      string outputfile;
      const string newformat = "flv";
      const string oldFormat = "h264";
      uint32_t nErrorCode = 0;

      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      videoFile_ = stringTypeConversion.to_bytes(videoFile);
      audioFile_ = audioFile;

      outputfile = changeFormatString(videoFile_, oldFormat, newformat);
      nErrorCode = flvmuxAVStreams(audioFile_.c_str(),
                                   videoFile_.c_str(),
                                   outputfile.c_str(),
                                   0.0f,
                                   0.0f,
                                   NO_DURATION_SPECIFIED,
                                   DEFAULT_SUGGESTED_FRAMES_PER_SECOND);

      if (0 != nErrorCode) {
        DEBUG_ERROR_VAR("Muxing failed. [Error code] ", to_string(nErrorCode));
        status = FBCAPTURE_STATUS_WAMDEIA_MUXING_FAILED;
      }

      remove(videoFile_.c_str());  // Remove h264 video file
      remove(audioFile_.c_str());  // Remove aac audio file

      return status;
    }
  }
}
