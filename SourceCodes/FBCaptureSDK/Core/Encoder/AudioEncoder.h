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
#include "Core/Common/Common.h"
#include "Core/Common/Log.h"
#include "Core/Common/ScopedCOMPtr.h"
#include <chrono>
#include <codecvt>
#include <iostream>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>

using namespace std;
using namespace std::chrono;
using namespace FBCapture::Common;

namespace FBCapture {
namespace Audio {

class AudioEncoder {

public:
  AudioEncoder() = default;
  ~AudioEncoder() {}
  FBCAPTURE_STATUS continueAudioTranscoding(const string &srcFile,
                                            const string &dstFile);
  void shutdown();

private:
  FBCAPTURE_STATUS initializeAudioTranscoding(IMFSourceReader **reader);

private:
  IMFActivate **mfActivate = {0};
  ScopedCOMPtr<IMFMediaType> mftOutMediaType = {0};
  ScopedCOMPtr<IMFMediaType> mftInMediaType = {0};
  ScopedCOMPtr<IMFTransform> mfTransform = {0};
  uint32_t mftCount = 0;
  uint8_t channels = 0;
  int sampleRate = 0;

  ScopedCOMPtr<IMFMediaBuffer> outMediaBuffer = {0};
  DWORD processOutputStatus = 0;
  MFT_OUTPUT_STREAM_INFO outStreamInfo = {0};
  MFT_OUTPUT_DATA_BUFFER outputData = {0};

  wstring srcFile_ = {};
  wstring dstFile_ = {};
};
}; // namespace Audio
} // namespace FBCapture
