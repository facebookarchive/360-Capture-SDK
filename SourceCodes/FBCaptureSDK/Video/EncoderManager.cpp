/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "EncoderManager.h"

namespace FBCapture {
	namespace Video {

		FBCAPTURE_STATUS EncoderManager::createD3D11Resources(ID3D11Texture2D* inputTexture, ComPtr<ID3D11Texture2D>& outputTexture) {
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
			HRESULT hr = S_OK;

			if (device_ == nullptr) {
				DEBUG_ERROR("DX device wasn't created. Please create DX device");
				return FBCAPTURE_STATUS_DEVICE_CREATING_FAILED;
			}

			// Create new texture based on RenderTexture in Unity			
			fromTexturePtr_ = inputTexture;
			fromTexturePtr_->GetDesc(&globalTexDesc_);
			globalTexDesc_.BindFlags = 0;
			globalTexDesc_.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
			globalTexDesc_.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			globalTexDesc_.Usage = D3D11_USAGE_STAGING;
			globalTexDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			hr = device_->CreateTexture2D(&globalTexDesc_, nullptr, outputTexture.ReleaseAndGetAddressOf());
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating new texture");
				return FBCAPTURE_STATUS_TEXTURE_CREATION_FAILED;
			}

			return status;
		}

		void EncoderManager::setTextureDirtyRegion() {
			// Set texure dirty region
			dirtyRegion_.left = 0;
			dirtyRegion_.right = globalTexDesc_.Width;
			dirtyRegion_.top = 0;
			dirtyRegion_.bottom = globalTexDesc_.Height;
			dirtyRegion_.front = 0;
			dirtyRegion_.back = 1;
		}

		FBCAPTURE_STATUS EncoderManager::mapTexture(D3D11_MAPPED_SUBRESOURCE& resource) {
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

			device_->GetImmediateContext(&context_);
			context_->CopySubresourceRegion(encodingTexure_.Get(), 0, 0, 0, 0, fromTexturePtr_, 0, &dirtyRegion_);
			HRESULT hr = context_->Map(encodingTexure_.Get(), 0, D3D11_MAP_READ_WRITE, 0, &resource);
			if (FAILED(hr)) {
				DEBUG_ERROR_VAR("Failed on context mapping. [Error Code] ", to_string(hr));
				return FBCAPTURE_STATUS_TEXTURE_RESOURCES_COPY_FAILED;
			}

			return status;
		}		
	}
}