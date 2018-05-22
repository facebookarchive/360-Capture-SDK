#pragma once

#include "Common.h"
#include "ScopedCOMPtr.h"

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