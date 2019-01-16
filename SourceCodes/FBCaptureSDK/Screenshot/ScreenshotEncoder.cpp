/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "ScreenshotEncoder.h"

namespace FBCapture {
	namespace Screenshot {
		ScreenShotEncoder::ScreenShotEncoder(ID3D11Device* device) {
			device_ = device;
		}

		FBCAPTURE_STATUS ScreenShotEncoder::saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool is360) {
			HRESULT hr = E_FAIL;
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

			createD3D11Resources((ID3D11Texture2D*)texturePtr, screenshotTexure_);
			setTextureDirtyRegion();

			D3D11_MAPPED_SUBRESOURCE resource = {};
			ComPtr<ID3D11DeviceContext> context;
			device_->GetImmediateContext(&context);
			context->CopySubresourceRegion(screenshotTexure_.Get(), 0, 0, 0, 0, (ID3D11Texture2D*)texturePtr, 0, &dirtyRegion_);

			// Screen Capture: Save texture to image file
			hr = SaveWICTextureToFile(context.Get(), screenshotTexure_.Get(), GUID_ContainerFormatJpeg, fullSavePath.c_str(), nullptr, nullptr, is360);
			if (FAILED(hr)) {
				DEBUG_ERROR_VAR("Failed on creating image file. [Error code] ", to_string(hr));
				return FBCAPTURE_STATUS_WIC_SAVE_IMAGE_FAILED;
			}			

			return status;
		}
	}
}