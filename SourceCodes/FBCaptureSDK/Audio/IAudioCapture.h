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
#include <mmdeviceapi.h>
#include <mmsystem.h>
#include <audioclient.h>
#include <initguid.h>
#include <stdio.h>
#include <avrt.h>
#include <regex>
#include <atomic>
#include <functiondiscoverykeys_devpkey.h>
#include "AudioEncoder.h"
#include "AudioBuffer.h"
#include "Common/log.h"
#include "Common/Common.h"

using namespace FBCapture::Common;

namespace FBCapture {
  namespace Audio {

    class IAudioCapture {
    public:
      IAudioCapture();  
			virtual ~IAudioCapture() = default;
      virtual FBCAPTURE_STATUS closeCaptureFile();    
      virtual void releaseCaptureResources() = 0;

      atomic<bool> needToInitializeResources = {};
      atomic<bool> needToCloseCaptureFile = {};
      atomic<bool> needToOpenCaptureFile = {};

    protected:
      static HMMIO file_;
      IAudioCaptureClient* outputAudioCaptureClient_ = {};
      IAudioClient* outputAudioClient_ = {};
      IMMDeviceEnumerator *mmDeviceEnumerator_ = {};
      IMMDevice *mmOutputDevice_ = {};
      IAudioCaptureClient* inputAudioCaptureClient_ = {};
      IAudioClient* inputAudioClient_ = {};
      IMMDevice* mmInputDevice_ = {};
      MMCKINFO ckRIFF_ = {};
      MMCKINFO ckData_ = {};
      MMCKINFO ckFMT_ = {};
      LPCWSTR fileName_ = {};
      wstring wavFileName_ = {};
      string aacFileName_ = {};
      UINT32 outputBlockAlign_ = {};
      UINT32 inputBlockAlign_ = {};
      LONG outputBytesToWrite_ = {};
      LONG inputBytesToWrite_ = {};
      BYTE* outputData_ = {};
      BYTE* inputData_ = {};
      WAVEFORMATEX* outputPWFX_ = {};
      WAVEFORMATEX* inputPWFX_ = {};
      size_t captureIndex_ = {};
      AudioBuffer* buffer_ = {};
    };
  }
}
