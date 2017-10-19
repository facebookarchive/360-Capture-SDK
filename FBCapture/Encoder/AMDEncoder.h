/****************************************************************************************************************

Filename	:	AMDEncoder.h
Content		:	AMD Encoder implementation for creating h264 format video
Created		:	Jan 26, 2017
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#pragma once
#include <stdio.h>
#include <tchar.h>
#include <codecvt>
#include <d3d11.h>
#include <wincodec.h>
#include "AMD/common/AMFFactory.h"
#include "AMD/include/components/VideoEncoderVCE.h"
#include "AMD/include/components/VideoEncoderHEVC.h"
#include "AMD/common/Thread.h"
#include "AMD/common/AMFSTL.h"
#include "ScreenGrab.h"
#include "Log.h"
#include "ErrorCodes.h"

#define START_TIME_PROPERTY L"StartTimeProperty" // custom property ID to store submission time in a frame - all custom properties are copied from input to output
#define MILLISEC_TIME     10000


//--------------------------------------------------------------------------------------------------------------
// *** SAFE_RELEASE macro
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) if (a) { a->Release(); a= NULL; }
#endif

using namespace FBCapture::Common;
using namespace Directx;
using namespace std;

namespace FBCapture {
  namespace Video {

    class PollingThread : public amf::AMFThread {
    protected:
      amf::AMFContextPtr      context_;
      amf::AMFComponentPtr    encoder_;
      FILE                    *file_;
      amf_int32				frameCount_;
    public:
      PollingThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName, amf_int32 frameCount);
      ~PollingThread();
      virtual void Run();
    };

    class AMDEncoder {
    public:
      FBCAPTURE_STATUS encodeMain(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping);
      FBCAPTURE_STATUS flushEncodedImages();
      FBCAPTURE_STATUS saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool needFlipping);
      AMDEncoder();  // Constructor

    protected:
      PollingThread* thread_;

      wchar_t *codec_;

      // initialize AMF
      amf::AMFContextPtr context_;
      amf::AMFComponentPtr encoder_;
      amf::AMFSurfacePtr surfaceIn_;

      amf::AMF_MEMORY_TYPE memoryTypeIn_;
      amf::AMF_SURFACE_FORMAT formatIn_;

      amf_int32 widthIn_;
      amf_int32 heightIn_;
      amf_int32 frameRateIn_;
      amf_int64 bitRateIn_;  // in bits, 5MBit
      amf_int32 rectSize_;
      amf_int32 frameCount_;
      bool maximumSpeed_;
      bool encodingConfigInitiated_;

      ID3D11Texture2D* tex_;
      ID3D11Texture2D* newTex_;
      ID3D11Device* deviceDX11_;


    protected:
      FBCAPTURE_STATUS initialization(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps);
      FBCAPTURE_STATUS fillSurface(amf::AMFContext *context, amf::AMFSurface *surface, bool needFlipping);
    };
  }
}
