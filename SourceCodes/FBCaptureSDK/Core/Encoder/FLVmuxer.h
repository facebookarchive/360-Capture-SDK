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
#include "flvmuxing.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <codecvt>
#include "Core/Common/Log.h"
#include "Core/Common/Common.h"

using namespace std;
using namespace FBCapture::Common;
using namespace libflvoperations;

namespace FBCapture {
  namespace Mux {
    class FLVMuxer {
    public:
      FLVMuxer();
      virtual ~FLVMuxer();

    private:
      int frameIndex;

      string videoFile_;
      string audioFile_;

    private:
      string changeFormatString(const string& path, const string& oldFormat, const string& newFormat);

    public:
      FBCAPTURE_STATUS muxingMedia(const wstring& videoFile, const string& audioFile);
    };
  }
}
