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
#include <audioclient.h>
#include "IAudioCapture.h"
#include "AudioEncoder.h"
#include "Common/log.h"
#include "Common/Common.h"

using namespace FBCapture::Common;
using namespace VrDeviceType;
using namespace StereoMode;
using namespace ProjectionType;
using namespace std;

namespace FBCapture {
  namespace Audio {

    class AudioDeviceCapture : public IAudioCapture {
    public:
      AudioDeviceCapture();
      ~AudioDeviceCapture();
      FBCAPTURE_STATUS initializeCaptureResources(bool useRiftAudioEndpoint, VRDeviceType vrDevice, LPCWSTR useMicIMMDeviceId);      
      FBCAPTURE_STATUS openCaptureFile(const string& srcFile);
      void continueAudioCapture(bool enabledAudioCapture, bool enabledMicCapture);      
      void releaseCaptureResources() override;

    private:
      bool writeWaveHeader(WAVEFORMATEX* pwfx, HMMIO* phFile);
      bool openFile(LPCWSTR szFileName, HMMIO *phFile);
      bool findAudioSource(bool isMic, LPCWSTR withIMMDeviceId);
      bool findVRDeviceAudioSource(bool isMic, VRDeviceType vrDevice);
      bool startAudioclient(IAudioClient* audioClient, IAudioCaptureClient** captureClient, bool isInput);

    private:
      bool useMicrophone_;
      bool useVRAudioResources_;
      std::chrono::time_point<std::chrono::steady_clock> lastAudioCapture_;
    };
  }
}
