#pragma once

#include "Common.h"
#include "TextureRender.h"

using namespace FBCapture::Render;

namespace FBCapture {
  namespace Common {

    class IGraphicsDeviceCapture {
    public:
      virtual FBCAPTURE_STATUS captureTextureToSharedHandle(void* fromTexturePtr) = 0;
      virtual FBCAPTURE_STATUS copyTexture(void* toTexturePtr, HANDLE fromOtherDeviceSharedHandle) = 0;
      virtual HANDLE getSharedHandle() = 0;
      virtual FBCAPTURE_STATUS renderCapturedTexture(ID3D11Device *otherDevice, TextureRender* otherDeviceTextureRender);
    };
  }
}
