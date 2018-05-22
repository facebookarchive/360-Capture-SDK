/****************************************************************************************************************

Filename	:	AMDEncoder.h
Content		:	AMD Encoder implementation for creating h264 format video
Created		:	Jan 26, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#pragma once
#include <stdio.h>
#include <tchar.h>
#include <codecvt>
#include <wincodec.h>
#include "AMD/common/AMFFactory.h"
#include "AMD/include/components/VideoEncoderVCE.h"
#include "AMD/include/components/VideoEncoderHEVC.h"
#include "AMD/common/Thread.h"
#include "AMD/common/AMFSTL.h"
#include "ScreenGrab.h"
#include "Log.h"
#include "Common.h"
#include "ScopedCOMPtr.h"

#define START_TIME_PROPERTY L"StartTimeProperty" // custom property ID to store submission time in a frame - all custom properties are copied from input to output
#define MILLISEC_TIME     10000
#define ENABLE_4K 1

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
			amf::AMFContextPtr      context_ = nullptr;
			amf::AMFComponentPtr    encoder_ = nullptr;
			FILE                    *file_ = nullptr;
		public:
			PollingThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName);
			~PollingThread();
			virtual void Run();
		};

		class AMDEncoder {
		public:
			FBCAPTURE_STATUS initAMDEncodingSession();
			FBCAPTURE_STATUS encodeMain(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping);
			FBCAPTURE_STATUS flushInputTextures();
			FBCAPTURE_STATUS saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool is360);
			AMDEncoder(ID3D11Device* device);  // Constructor
			virtual ~AMDEncoder();
		protected:
			PollingThread* thread_ = nullptr;

			wchar_t *codec_ = nullptr;

			// initialize AMF
			amf::AMFContextPtr context_ = nullptr;
			amf::AMFComponentPtr encoder_ = nullptr;
			amf::AMFSurfacePtr surfaceIn_ = nullptr;

			amf::AMF_MEMORY_TYPE memoryTypeIn_;
			amf::AMF_SURFACE_FORMAT formatIn_;

			amf_int32 widthIn_ = 0;
			amf_int32 heightIn_ = 0;
			amf_int32 frameRateIn_ = 0;
			amf_int64 bitRateIn_ = 0;  // in bits, 5MBit
			bool maximumSpeed_ = false;
			bool encodingConfigInitiated_ = false;

			// DX 11 interfaces  
			ID3D11Texture2D* fromTexturePtr_ = nullptr;						
			ScopedCOMPtr<ID3D11Device> device_ = nullptr;
			D3D11_BOX dirtyRegion_ = {};
			D3D11_TEXTURE2D_DESC globalTexDesc_ = {};

		protected:
			FBCAPTURE_STATUS initializeEncodingComponents(const wstring& fullSavePath, int bitrate, int fps);
			FBCAPTURE_STATUS fillSurface(amf::AMFSurface *surface);
			FBCAPTURE_STATUS createD3D11Resources(ID3D11Texture2D* texture);
		};
	}
}