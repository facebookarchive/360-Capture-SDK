/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "IGraphicsDeviceCapture.h"

namespace FBCapture {
  namespace Common {
    FBCAPTURE_STATUS IGraphicsDeviceCapture::renderCapturedTexture(ID3D11Device *otherDevice, TextureRender* otherDeviceTextureRender) {
      ScopedCOMPtr<ID3D11Texture2D> sharedTexture;
      HRESULT hr = otherDevice->OpenSharedResource(
        getSharedHandle(),
        IID_PPV_ARGS(&sharedTexture));
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to open GraphicsDeviceCapture shared resource. [Error code] ",
                        to_string(hr));
        if (hr == DXGI_ERROR_DEVICE_REMOVED) {
          hr = otherDevice->GetDeviceRemovedReason();
          DEBUG_HRESULT_ERROR("DirectX deivce has removed", hr);
        }
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_OPEN_SHARED_RESOURCE_FAILED;
      }

      ScopedCOMPtr<IDXGIKeyedMutex> sharedTextureKeyedMutex;
      hr = sharedTexture->QueryInterface(
        IID_PPV_ARGS(&sharedTextureKeyedMutex));
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to open GraphicsDeviceCapture shared keyed mutex. [Error code] ",
                        to_string(hr));
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_MUTEX_ACQUIRE_FAILED;
      }

      hr = sharedTextureKeyedMutex->AcquireSync(0, INFINITE);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to acquire sync on GraphicsDeviceCapture shared resource. [Error code] ",
                        to_string(hr));
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_ACQUIRE_ACQUIRE_SYNC_FAILED;
      }

      otherDeviceTextureRender->renderFrame(sharedTexture);

      hr = sharedTextureKeyedMutex->ReleaseSync(0);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to release sync on GraphicsDeviceCapture shared resource. [Error code] ",
                        to_string(hr));
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_ACQUIRE_RELASE_SYNC_FAILED;
      }
      return FBCAPTURE_STATUS_OK;
    }
  }
}
