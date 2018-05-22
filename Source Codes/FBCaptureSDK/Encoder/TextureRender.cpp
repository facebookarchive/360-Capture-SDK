#include "TextureRender.h"
#include "Log.h"
#include <D3DCompiler.h>

namespace FBCapture {
  namespace Render {

    bool TextureRender::initialize(ID3D11Device *device,
      ID3D11DeviceContext *context,
      ID3D11Texture2D *outputTexture,
      const void *pVertShaderBytecode,
      SIZE_T pVertShaderBytecodeLen,
      const void *pPixelShaderBytecode,
      SIZE_T pPixelShaderBytecodeLen, 
			bool verticalFlip, 
			bool horizontalFlip) {

      this->device = device;
      this->deviceContext = context;
      this->outputTexture = outputTexture;

      this->device->CreateVertexShader(pVertShaderBytecode, pVertShaderBytecodeLen,
        nullptr, &this->vertexShader);
      this->device->CreatePixelShader(pPixelShaderBytecode, pPixelShaderBytecodeLen,
        nullptr, &this->pixelShader);			

      static D3D11_INPUT_ELEMENT_DESC desc[] = {
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
           D3D11_INPUT_PER_VERTEX_DATA},
          {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
           D3D11_INPUT_PER_VERTEX_DATA},
      };

      int n_desc = sizeof(desc) / sizeof(desc[0]);

      HRESULT hr = device->CreateInputLayout(
        desc, n_desc, pVertShaderBytecode, pVertShaderBytecodeLen,
        &this->triangleVertexBufferInputLayout);

      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Create input layout failed", hr);
        return false;
      }

      if (!createTriangleVertexBuffer(verticalFlip, horizontalFlip)) {
        return false;
      }

      D3D11_SAMPLER_DESC samplerDesc;
      samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
      samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
      samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
      samplerDesc.MipLODBias = 0.0f;
      samplerDesc.MaxAnisotropy = 1;
      samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
      samplerDesc.MinLOD = -FLT_MAX;
      samplerDesc.MaxLOD = FLT_MAX;

      hr = device->CreateSamplerState(&samplerDesc, &samplerState);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Create sampler state failed", hr);
        return false;
      }			

      D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
      ZeroMemory(&renderTargetDesc, sizeof(renderTargetDesc));
      renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

      D3D11_TEXTURE2D_DESC textureDesc;
      this->outputTexture->GetDesc(&textureDesc);
      renderTargetDesc.Format = textureDesc.Format;

      D3D11_TEX2D_RTV subresource;
      subresource.MipSlice = 0;
      renderTargetDesc.Texture2D = subresource;

      hr = this->device->CreateRenderTargetView(outputTexture, &renderTargetDesc,
        &this->renderTarget);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Create render target view failed", hr);
        return false;
      }

      return true;
    }

    void TextureRender::setViewport(int topLeftX, int topLeftY, int width,
      int height) {
      D3D11_VIEWPORT viewport;
      ZeroMemory(&viewport, sizeof(viewport));

      viewport.TopLeftX = topLeftX;
      viewport.TopLeftY = topLeftY;
      viewport.Height = (FLOAT)height;
      viewport.Width = (FLOAT)width;

      deviceContext->RSSetViewports(1, &viewport);
    }

    void TextureRender::renderFrame(ID3D11Texture2D *inputTexture) {

      D3D11_TEXTURE2D_DESC inputDesc;			
      inputTexture->GetDesc(&inputDesc);

      D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
      // All textures other than camera should be DXGI_FORMAT_R8G8B8A8_UNORM
      // Camera textures of DXGI_FORMAT_B8G8R8X8_UNORM are converted to DXGI_FORMAT_R8G8B8A8_UNORM automatically in render
      shaderResourceViewDesc.Format = inputDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM ? DXGI_FORMAT_B8G8R8X8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
      shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      shaderResourceViewDesc.Texture2D.MipLevels = -1;
      shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;			

      sampleTextureView = nullptr;
      HRESULT hr =
        device->CreateShaderResourceView(inputTexture, &shaderResourceViewDesc, &sampleTextureView);
      if (FAILED(hr)) {				
        DEBUG_HRESULT_ERROR("Create shader resource view failed", hr);
				if (hr == DXGI_ERROR_DEVICE_REMOVED) {
					hr = device->GetDeviceRemovedReason();
					DEBUG_HRESULT_ERROR("DirectX deivce has removed", hr);
				}
        return;
      }

      deviceContext->PSSetSamplers(0, 1, &samplerState);
      deviceContext->PSSetShaderResources(0, 1, &sampleTextureView);

      deviceContext->VSSetShader(vertexShader, NULL, 0);
      deviceContext->PSSetShader(pixelShader, NULL, 0);
      deviceContext->IASetInputLayout(triangleVertexBufferInputLayout);

      ID3D11Buffer *vertexBuffers[1] = { triangleVertexBuffer };
      UINT stride = sizeof(Vertex);
      UINT offset = 0;
      deviceContext->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
      deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
      deviceContext->OMSetRenderTargets(1, &this->renderTarget, NULL);
      deviceContext->Draw(4, 0);

      ID3D11RenderTargetView *const pRTV[1] = { NULL };
      deviceContext->OMSetRenderTargets(1, pRTV, NULL);

      ID3D11ShaderResourceView *const pSRV[1] = { NULL };
      deviceContext->PSSetShaderResources(0, 1, pSRV);
    }

    bool TextureRender::createTriangleVertexBuffer(bool verticalFlip, bool horizontalFlip) {

			Vertex triangle[4];
    	
			if (verticalFlip && horizontalFlip) {
				// Coordinates:  x       y      z     s     t
				triangle[0] = { -1.00f, -1.00f, 0.0f, 1.0f, 1.0f };
				triangle[1] = { -1.00f,  1.00f, 0.0f, 1.0f, 0.0f };
				triangle[2] = {  1.00f, -1.00f, 0.0f, 0.0f, 1.0f };
				triangle[3] = {  1.00f,  1.00f, 0.0f, 0.0f, 0.0f };
			} else if (verticalFlip) {
				triangle[0] = { -1.00f, -1.00f, 0.0f, 0.0f, 1.0f };
				triangle[1] = { -1.00f,  1.00f, 0.0f, 0.0f, 0.0f };
				triangle[2] = {  1.00f, -1.00f, 0.0f, 1.0f, 1.0f };
				triangle[3] = {  1.00f,  1.00f, 0.0f, 1.0f, 0.0f };
			} else if (horizontalFlip) {
				triangle[0] = { -1.00f, -1.00f, 0.0f, 1.0f, 0.0f };
				triangle[1] = { -1.00f,  1.00f, 0.0f, 1.0f, 1.0f };
				triangle[2] = { 1.00f,  -1.00f, 0.0f, 0.0f, 0.0f };
				triangle[3] = { 1.00f,   1.00f, 0.0f, 0.0f, 1.0f };
			} else {
				triangle[0] = { -1.00f, -1.00f, 0.0f, 0.0f, 0.0f };
				triangle[1] = { -1.00f,  1.00f, 0.0f, 0.0f, 1.0f };
				triangle[2] = {  1.00f, -1.00f, 0.0f, 1.0f, 0.0f };
				triangle[3] = {  1.00f,  1.00f, 0.0f, 1.0f, 1.0f };
			}

      D3D11_BUFFER_DESC desc;
      ZeroMemory(&desc, sizeof(desc));

      desc.Usage = D3D11_USAGE_DYNAMIC;
      desc.ByteWidth = sizeof(triangle);
      desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

      HRESULT hr = device->CreateBuffer(&desc, nullptr, &triangleVertexBuffer);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Create triangle vertex buffer failed", hr);
        return false;
      }

      D3D11_MAPPED_SUBRESOURCE ms;
      hr = deviceContext->Map(triangleVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0,
        &ms);
      CopyMemory(ms.pData, triangle, sizeof(triangle));
      deviceContext->Unmap(triangleVertexBuffer, 0);
      return true;
    }
  }
}