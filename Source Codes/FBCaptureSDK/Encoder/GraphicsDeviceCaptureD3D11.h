#pragma once

#include "IGraphicsDeviceCapture.h"
#include "ScopedCOMPtr.h"
#include "TextureRender.h"

using namespace FBCapture::Render;

namespace FBCapture {
  namespace Common {
    class GraphicsDeviceCaptureD3D11 : public IGraphicsDeviceCapture {
    public:
      GraphicsDeviceCaptureD3D11(ID3D11Device* device) : pDevice_(device) {};
      FBCAPTURE_STATUS captureTextureToSharedHandle(void* fromTexturePtr) override;
      FBCAPTURE_STATUS copyTexture(void* toTexturePtr, HANDLE fromOtherDeviceSharedHandle) override;
      HANDLE getSharedHandle() override { return pSharedHandle_; }
    private:
      FBCAPTURE_STATUS updateSharedHandle(int width, int height, DXGI_FORMAT format);
      ScopedCOMPtr<ID3D11Texture2D> pTexture_;
      ID3D11Device* pDevice_ = {};
      HANDLE pSharedHandle_ = {};
    };
  }
}