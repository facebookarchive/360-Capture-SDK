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

#include "Core/Common/Common.h"
#include "Core/Common/ScopedCOMPtr.h"

using namespace FBCapture::Common;

namespace FBCapture {
  namespace Render {

    class TextureRender {
    public:
      bool initialize(ID3D11Device *device, ID3D11DeviceContext *context,
        ID3D11Texture2D *texture, const void *pVertShaderBytecode,
        SIZE_T pVertShaderBytecodeLen,
        const void *pPixelShaderBytecode,
        SIZE_T pPixelShaderBytecodeLen, 
				bool verticalFlip,
				bool horizontalFlip);

      void setViewport(int topLeftX, int topLeftY, int width, int height);
      void renderFrame(ID3D11Texture2D *inputTexture);

    private:
      ID3D11Device *device;
      ID3D11DeviceContext *deviceContext;
      ID3D11Texture2D *outputTexture;
      ScopedCOMPtr<ID3D11RenderTargetView> renderTarget;
      ScopedCOMPtr<ID3D11VertexShader> vertexShader;
      ScopedCOMPtr<ID3D11PixelShader> pixelShader;

      ScopedCOMPtr<ID3D11Buffer> triangleVertexBuffer;
      ScopedCOMPtr<ID3D11InputLayout> triangleVertexBufferInputLayout;
      ScopedCOMPtr<ID3D11ShaderResourceView> sampleTextureView;
      ScopedCOMPtr<ID3D11SamplerState> samplerState;

      bool createTriangleVertexBuffer(bool verticalFlip, bool horizontalFlip);

      struct Vertex {
        FLOAT x, y, z;
        FLOAT s, t;
      };
    };
  }
}
