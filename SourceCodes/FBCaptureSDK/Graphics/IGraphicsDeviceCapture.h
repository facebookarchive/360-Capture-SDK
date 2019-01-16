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

#include "Common/Common.h"
#include "Common/Log.h"
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
