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
#include "Video/EncoderManager.h"
#include "3rdParty/OpenH264/api/svc/codec_def.h"
#include "3rdParty/OpenH264/api/svc/codec_api.h"
#include "3rdParty/OpenH264/encoder/core/inc/extern.h"
#include "3rdParty/OpenH264/common/inc/macros.h"
#include "3rdParty/OpenH264/encoder/core/inc/wels_const.h"
#include "3rdParty/OpenH264/encoder/core/inc/mt_defs.h"
#include "3rdParty/OpenH264/common/inc/WelsThreadLib.h"

namespace FBCapture {
	namespace Video {

		class OpenH264Encoder : public EncoderManager {
		public:
			explicit OpenH264Encoder(ID3D11Device* device);
			FBCAPTURE_STATUS initEncodingSession() override;
			FBCAPTURE_STATUS encodeProcess(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) override;
			FBCAPTURE_STATUS flushInputTextures() override;

		protected:
			FBCAPTURE_STATUS setEncodeConfigs(const wstring& fullSavePath, uint32_t width, uint32_t height, int bitrate, int fps) override;
			void chromaSubSamplingFromYUV444toYUV420(byte* yuva, byte* yuv420, int width, int height);

		private:
			SFrameBSInfo frameBSInfo_ = {};
			SEncParamExt encodeConfig_ = {};
			FILE* file_ = {};
			SSourcePicture srcPic_ = {};
			ISVCEncoder* svcEncoder_ = {};
			uint8_t* pYUV420_ = nullptr;
		};
	}
}
