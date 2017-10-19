/****************************************************************************************************************

Filename	:	Muxing.cpp
Content		:
Created		:
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#include "MP4muxer.h"

namespace FBCapture {
  namespace Mux {

    MP4Muxer::MP4Muxer() {}

    MP4Muxer::~MP4Muxer() {
      remove(videoFile_.c_str());  // Remove h264 video file
      remove(audioFile_.c_str());  // Remove aac audio file
    }

    string MP4Muxer::changeFormatString(const string& path, const string& oldFormat, const string& newFormat) {
      string newPath(path);

      newPath.erase(path.length() - oldFormat.length(), oldFormat.length());
      newPath += newFormat;

      return newPath;
    }

    FBCAPTURE_STATUS MP4Muxer::muxingMedia(const wstring& videoFile, const string& audioFile) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
      string outputfile;
      const string newformat = "mp4";
      const string oldFormat = "h264";
      uint32_t nErrorCode = 0;

      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      videoFile_ = stringTypeConversion.to_bytes(videoFile);
      audioFile_ = audioFile;

      outputfile = changeFormatString(videoFile_, oldFormat, newformat);
      nErrorCode = mp4muxAVStreams(audioFile_.c_str(),
                                   videoFile_.c_str(),
                                   outputfile.c_str(),
                                   0.0f,
                                   0.0f,
                                   NO_DURATION_SPECIFIED,
                                   UNKNOWN_FRAMES_PER_SECOND,
                                   (eVideoRotationMode)VIDEO_ROTATION_MODE_NO_ROTATION,
                                   false);

      if (0 != nErrorCode) {
        DEBUG_ERROR_VAR("Muxing failed, error: ", to_string(nErrorCode));
        status = FBCAPTURE_STATUS_WAMDEIA_MUXING_FAILED;
      }

      remove(videoFile_.c_str());  // Remove h264 video file
      remove(audioFile_.c_str());  // Remove aac audio file

      return status;
    }
  }
}
