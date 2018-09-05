#include "GraphicsDeviceCaptureD3D11.h"
#include "Log.h"

namespace FBCapture {
  namespace Common {
    FBCAPTURE_STATUS GraphicsDeviceCaptureD3D11::updateSharedHandle(int width, int height, DXGI_FORMAT format) {
      D3D11_TEXTURE2D_DESC desc = {};
      ZeroMemory(&desc, sizeof(desc));
      desc.Width = width;
      desc.Height = height;
      desc.Format = format;
      desc.SampleDesc.Count = 1;
      desc.ArraySize = 1;
      desc.MipLevels = 1;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

      // reset texture if necessary
      pTexture_ = nullptr;

      HRESULT hr = pDevice_->CreateTexture2D(&desc, nullptr,
        &pTexture_);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to create Texture2D in GraphicsDeviceCaptureD3D11. [Error code] ",
          hr);
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_INIT_FAILED;
      }

      ScopedCOMPtr<IDXGIResource> textureResource;
      hr = pTexture_->QueryInterface(IID_PPV_ARGS(&textureResource));
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to get IDXGIResource from Texture2D in GraphicsDeviceCaptureD3D11. [Error code] ",
          hr);
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_INIT_FAILED;
      }

      hr = textureResource->GetSharedHandle(&pSharedHandle_);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to get shared handle from Texture2D in GraphicsDeviceCaptureD3D11. [Error code] ",
          hr);
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_INIT_FAILED;
      }
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS GraphicsDeviceCaptureD3D11::captureTextureToSharedHandle(void* fromTexturePtr) {
      if (fromTexturePtr == nullptr) {
        DEBUG_ERROR("Invalid texture given to GraphicsDeviceCaptureD3D11");
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_INVALID_TEXTURE;
      }
      FBCAPTURE_STATUS result = FBCAPTURE_STATUS_OK;

      ID3D11Texture2D* inputTexturePtr = static_cast<ID3D11Texture2D*>(fromTexturePtr);
      D3D11_TEXTURE2D_DESC inputDesc;
      inputTexturePtr->GetDesc(&inputDesc);
			// Texture format needs to be set to DXGI_FORMAT_R8G8B8A8_UNORM here. 
			// Otherwise, it will create glitch texture on AMD driver. 
			inputDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

      // update existing shared handle if necessary
      bool needsUpdate = !pTexture_;
      if (pTexture_) {
        D3D11_TEXTURE2D_DESC currentDesc;
        pTexture_->GetDesc(&currentDesc);

        needsUpdate = (inputDesc.Format != currentDesc.Format
          || inputDesc.Width != currentDesc.Width
          || inputDesc.Height != currentDesc.Height);
      }

      if (needsUpdate) {
        result = updateSharedHandle(inputDesc.Width, inputDesc.Height, inputDesc.Format);
      }

      if (result != FBCAPTURE_STATUS_OK) return result;

      ScopedCOMPtr<IDXGIKeyedMutex> sharedTextureKeyedMutex;
      HRESULT hr = pTexture_->QueryInterface(
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

      ScopedCOMPtr<ID3D11DeviceContext> context;
      pDevice_->GetImmediateContext(&context);

      context->CopyResource(pTexture_, inputTexturePtr);
      context->Flush();

      hr = sharedTextureKeyedMutex->ReleaseSync(0);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to release sync on GraphicsDeviceCapture shared resource. [Error code] ",
          to_string(hr));
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_ACQUIRE_RELASE_SYNC_FAILED;
      }
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS GraphicsDeviceCaptureD3D11::copyTexture(void* toTexturePtr, HANDLE fromOtherDeviceSharedHandle) {
      ID3D11Texture2D* toD3D11TexturePtr = static_cast<ID3D11Texture2D*>(toTexturePtr);

      ScopedCOMPtr<ID3D11Texture2D> sharedTexture;
      HRESULT hr = pDevice_->OpenSharedResource(
        fromOtherDeviceSharedHandle,
        IID_PPV_ARGS(&sharedTexture));
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to open GraphicsDeviceCapture shared resource. [Error code] ",
          to_string(hr));
				if (hr == DXGI_ERROR_DEVICE_REMOVED) {
					hr = pDevice_->GetDeviceRemovedReason();
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

      ScopedCOMPtr<ID3D11DeviceContext> context;
      pDevice_->GetImmediateContext(&context);

      context->CopyResource(toD3D11TexturePtr, sharedTexture);

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
