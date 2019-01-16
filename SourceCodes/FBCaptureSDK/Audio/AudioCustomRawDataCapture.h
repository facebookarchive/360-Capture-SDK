/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#pragma once

#include "Common/Common.h"
#include "Common/log.h"
#include "IAudioCapture.h"

using namespace std;
using namespace FBCapture::Common;

namespace FBCapture {
  namespace Audio {

    class AudioCustomRawDataCapture : public IAudioCapture {
    private:
      WAVEFORMATEX format = {};
    public:
      AudioCustomRawDataCapture();
      ~AudioCustomRawDataCapture();
      FBCAPTURE_STATUS openCaptureFile(const string& srcFile, uint32_t sampleRate, uint32_t channel);
      void continueAudioCapture(float* audioRawData, uint32_t bufferSize);
      void releaseCaptureResources() override;
    };
  }
}
