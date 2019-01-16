/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "IAudioCapture.h"

namespace FBCapture {
  namespace Audio {

    HMMIO IAudioCapture::file_ = {};

    IAudioCapture::IAudioCapture() {
      // Riff chunk
      ckRIFF_.ckid = MAKEFOURCC('R', 'I', 'F', 'F');
      ckRIFF_.fccType = MAKEFOURCC('W', 'A', 'V', 'E');
      // fmt chunk
      ckFMT_.ckid = MAKEFOURCC('f', 'm', 't', ' ');
      // data chunk
      ckData_.ckid = MAKEFOURCC('d', 'a', 't', 'a');
    }

    FBCAPTURE_STATUS IAudioCapture::closeCaptureFile() {

      if (file_) {

        auto cleanupHandler = [](HMMIO__* ptr) {
          mmioClose(ptr, 0);
          file_ = nullptr;
        };
        auto filePtr = std::unique_ptr<HMMIO__, decltype(cleanupHandler)>(file_, cleanupHandler);

        MMRESULT hr;
        hr = mmioAscend(filePtr.get(), &ckData_, 0);
        if (MMSYSERR_NOERROR != hr) {
          DEBUG_LOG_VAR("Failed to ascend out of a chunk in a RIFF file. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_RELEASING_WAV_FAILED;
        }

        hr = mmioAscend(filePtr.get(), &ckRIFF_, 0);
        if (MMSYSERR_NOERROR != hr) {
          DEBUG_LOG_VAR("Failed to ascend out of a chunk in a Data file. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_RELEASING_WAV_FAILED;
        }

        DEBUG_LOG("Released wave file resources");
      }

      needToCloseCaptureFile = false;
      needToOpenCaptureFile = true;

      return FBCAPTURE_STATUS_OK;
    }
  }
}
