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

using namespace FBCapture::Common;
using namespace FBCapture::Screenshot;
using namespace std;
using Microsoft::WRL::ComPtr;

namespace FBCapture {
	namespace Video {
		// Event types for UnitySetGraphicsDevice
		enum GfxDeviceEventType {
			kGfxDeviceEventInitialize = 0,
			kGfxDeviceEventShutdown = 1,
			kGfxDeviceEventBeforeReset = 2,
			kGfxDeviceEventAfterReset = 3,
		};

		// Graphics device identifiers in Unity
		enum GfxDeviceRenderer {
			kGfxRendererOpenGL = 0, // OpenGL
			kGfxRendererD3D9,				// Direct3D 9
			kGfxRendererD3D11				// Direct3D 11
		};

		class EncoderManager {
		public:
			virtual ~EncoderManager() = default;
			virtual FBCAPTURE_STATUS createD3D11Resources(ID3D11Texture2D* inputTexture, ComPtr<ID3D11Texture2D>& outputTexture);
			virtual FBCAPTURE_STATUS mapTexture(D3D11_MAPPED_SUBRESOURCE& resource);
			virtual void setTextureDirtyRegion();
		public:
			virtual FBCAPTURE_STATUS initEncodingSession() = 0;
			virtual FBCAPTURE_STATUS encodeProcess(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) = 0;  // Main loop function for encoding			
			virtual FBCAPTURE_STATUS flushInputTextures() = 0; // Flush queued textures in buffers to create video			
			virtual FBCAPTURE_STATUS setEncodeConfigs(const wstring& fullSavePath, uint32_t width, uint32_t height, int bitrate, int fps) = 0;

		protected:
			ID3D11Texture2D* fromTexturePtr_ = nullptr;
			ID3D11DeviceContext* context_ = nullptr;
			ID3D11Device* device_ = nullptr;
			ComPtr<ID3D11Texture2D> encodingTexure_ = nullptr;
			ComPtr<ID3D11Texture2D> screenshotTexure_ = nullptr;
			D3D11_TEXTURE2D_DESC globalTexDesc_ = {};
			D3D11_BOX dirtyRegion_ = {};

			string videoFileName_ = {};
			bool encodingInitiated_ = false;	
		};

	}
}
