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
#include <iostream>
#include <codecvt>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "AudioEncoder.h"
#include "AudioBuffer.h"
#include <iostream>
#include <initguid.h>
#include <stdio.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <regex>
#include <functiondiscoverykeys_devpkey.h>
#include "Core/Common/log.h"
#include "Core/Common/Common.h"
#include <atomic>
#include "Core/Common/ScopedCOMPtr.h"

using namespace FBCapture::Common;
using namespace VrDeviceType;
using namespace StereoMode;
using namespace ProjectionType;
using namespace std;

namespace FBCapture {
  namespace Audio {

    class AudioCapture {
    public:
      AudioCapture();
      virtual ~AudioCapture();
      FBCAPTURE_STATUS initializeDevices(bool useRiftAudioEndpoint, VRDeviceType vrDevice, LPCWSTR useMicIMMDeviceId);
      void closeDevices();
      FBCAPTURE_STATUS openCaptureFile(const string& srcFile);
      FBCAPTURE_STATUS closeCaptureFile();
      void continueAudioCapture(bool enabledAudioCapture, bool enabledMicCapture);
      atomic<bool> needToInitializeDevices;
      atomic<bool> needToCloseCaptureFile;
      atomic<bool> needToOpenCaptureFile;

    private:
      bool writeWaveHeader(WAVEFORMATEX* pwfx, HMMIO* phFile);
      bool openFile(LPCWSTR szFileName, HMMIO *phFile);
      bool findAudioSource(bool isMic, LPCWSTR withIMMDeviceId);
      bool findVRDeviceAudioSource(bool isMic, VRDeviceType vrDevice);
      bool startAudioclient(IAudioClient* audioClient, IAudioCaptureClient** captureClient, bool isInput);

    private:
			static HMMIO file_;
			static IAudioCaptureClient* outputAudioCaptureClient_;
      static IAudioClient* outputAudioClient_;
      static IMMDeviceEnumerator *mmDeviceEnumerator_;
      static IMMDevice *mmOutputDevice_;
      static IAudioCaptureClient* inputAudioCaptureClient_;
      static IAudioClient* inputAudioClient_;
      static IMMDevice* mmInputDevice_;
      static MMCKINFO ckRIFF_;
      static MMCKINFO ckData_;
      static MMCKINFO ckFMT_;
      static LPCWSTR fileName_;
      static wstring wavFileName_;
      static string aacFileName_;
      static UINT32 outputBlockAlign_;
      static UINT32 inputBlockAlign_;
      static LONG outputBytesToWrite_;
      static LONG inputBytesToWrite_;
      static BYTE* outputData_;
      static BYTE* inputData_;
			static WAVEFORMATEX* outputPWFX_;
			static WAVEFORMATEX* inputPWFX_;
      static size_t captureIndex_;
      static AudioBuffer* buffer_;

      bool useMicrophone_;
      bool useVRAudioResources_;
      std::chrono::time_point<std::chrono::steady_clock> lastAudioCapture_;
    };
  }
}
