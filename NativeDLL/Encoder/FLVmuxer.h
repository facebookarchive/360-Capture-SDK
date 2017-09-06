/****************************************************************************************************************

Filename	:	Muxing.h
Content		:
Created		:
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#pragma once
#include "flvmuxing.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <codecvt>
#include "windows.h"
#include "Log.h"
#include "ErrorCodes.h"

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
