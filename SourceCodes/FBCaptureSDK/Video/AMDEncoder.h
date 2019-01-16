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
#include <stdio.h>
#include <tchar.h>
#include <codecvt>
#include <wincodec.h>
#include "3rdParty/AMD/common/AMFFactory.h"
#include "3rdParty/AMD/include/components/VideoEncoderVCE.h"
#include "3rdParty/AMD/include/components/VideoEncoderHEVC.h"
#include "3rdParty/AMD/common/Thread.h"
#include "3rdParty/AMD/common/AMFSTL.h"
#include "Common/Log.h"
#include "Common/Common.h"
#include "Common/ScopedCOMPtr.h"
#include "Video/EncoderManager.h"

#define START_TIME_PROPERTY L"StartTimeProperty" // custom property ID to store submission time in a frame - all custom properties are copied from input to output
#define MILLISEC_TIME     10000
#define ENABLE_4K 1

//--------------------------------------------------------------------------------------------------------------
// *** SAFE_RELEASE macro
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) if (a) { a->Release(); a= NULL; }
#endif

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

		class AMDEncoder : public EncoderManager {
		public:
			FBCAPTURE_STATUS initEncodingSession() override;
			FBCAPTURE_STATUS encodeProcess(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) override;
			FBCAPTURE_STATUS flushInputTextures() override;

			AMDEncoder(ID3D11Device* device);  // Constructor
			virtual ~AMDEncoder();
		protected:
			std::unique_ptr<PollingThread> thread_ = {};

			wchar_t *codec_ = nullptr;

			// initialize AMF
			amf::AMFContextPtr amfContext_ = nullptr;
			amf::AMFComponentPtr encoder_ = nullptr;
			amf::AMFSurfacePtr surfaceIn_ = nullptr;

			amf::AMF_MEMORY_TYPE memoryTypeIn_;
			amf::AMF_SURFACE_FORMAT formatIn_;

			amf_int32 widthIn_ = 0;
			amf_int32 heightIn_ = 0;
			amf_int32 frameRateIn_ = 0;
			amf_int64 bitRateIn_ = 0;  // in bits, 5MBit
			bool maximumSpeed_ = false;

		protected:
			FBCAPTURE_STATUS setEncodeConfigs(const wstring& fullSavePath, uint32_t width, uint32_t height, int bitrate, int fps) override;
			FBCAPTURE_STATUS fillSurface(amf::AMFSurface *surface);
		};
	}
}
