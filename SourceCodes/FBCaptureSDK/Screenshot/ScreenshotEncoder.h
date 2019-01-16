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
#pragma warning(disable : 4996)
#define GetCurrentDir _getcwd
#include <d3d11.h>
#include <wincodec.h>
#include <wrl\client.h>
#include "Screenshot/ScreenGrab.h"
#include "Common/Common.h"
#include "Common/ScopedCOMPtr.h"
#include "Video/EncoderManager.h"

using namespace FBCapture::Common;
using namespace FBCapture::Video;

namespace FBCapture {
	namespace Screenshot {

		class ScreenShotEncoder : public EncoderManager {
		public:
			ScreenShotEncoder(ID3D11Device* device);
			FBCAPTURE_STATUS saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool is360);  // Take screenshot					
		private:
			FBCAPTURE_STATUS initEncodingSession() override { return FBCAPTURE_STATUS_OK; };
			FBCAPTURE_STATUS encodeProcess(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) override { return FBCAPTURE_STATUS_OK; };
			FBCAPTURE_STATUS flushInputTextures() override { return FBCAPTURE_STATUS_OK; };			
			FBCAPTURE_STATUS setEncodeConfigs(const wstring& fullSavePath, uint32_t width, uint32_t height, int bitrate, int fps) override { return FBCAPTURE_STATUS_OK; };
		};
	}
}
