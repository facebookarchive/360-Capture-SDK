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
#include "3rdParty/Wamedia/mp4muxer/include/mp4muxing.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <codecvt>
#include "Common/Log.h"
#include "Common/Common.h"
#include "3rdParty/Spatialmedia/metadata_utils.h"

using namespace std;
using namespace libmp4operations;
using namespace FBCapture::Common;
using namespace VrDeviceType;
using namespace StereoMode;
using namespace ProjectionType;

#define APPEND_METADATA_SUFFIX 1

namespace FBCapture {
  namespace Mux {
    class MP4Muxer {
    public:
      MP4Muxer();
      virtual ~MP4Muxer();

    private:
      int frameIndex;

      string videoFile_;
      string audioFile_;

    private:
      string replaceString(const string& inputStr, const string& searchStr, const string& replaceStr);

    public:
      FBCAPTURE_STATUS muxingMedia(const wstring& videoFile, const string& audioFile, PROJECTIONTYPE projectionType, STEREO_MODE stereoMode, float fps, bool is360);
    };
  }
}
