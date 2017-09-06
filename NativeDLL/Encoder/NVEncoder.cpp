/****************************************************************************************************************

Filename	:	NVEncoder.cpp
Content		:	NVidia Encoder implementation for creating h264 format video
Created		:	December 13, 2016
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#include "NVEncoder.h"
#include "NVidia/common/inc/nvFileIO.h"
#include "NVidia/common/inc/nvUtils.h"
#include "Log.h"

// To print out error string got from NV HW encoder API
const char* nvidiaStatus[] = { "NV_ENC_SUCCESS", "NV_ENC_ERR_NO_ENCODE_DEVICE", "NV_ENC_ERR_UNSUPPORTED_DEVICE", "NV_ENC_ERR_INVALID_ENCODERDEVICE",
"NV_ENC_ERR_INVALID_DEVICE", "NV_ENC_ERR_DEVICE_NOT_EXIST", "NV_ENC_ERR_INVALID_PTR", "NV_ENC_ERR_INVALID_EVENT",
"NV_ENC_ERR_INVALID_PARAM", "NV_ENC_ERR_INVALID_CALL", "NV_ENC_ERR_OUT_OF_MEMORY", "NV_ENC_ERR_ENCODER_NOT_INITIALIZED",
"NV_ENC_ERR_UNSUPPORTED_PARAM", "NV_ENC_ERR_LOCK_BUSY", "NV_ENC_ERR_NOT_ENOUGH_BUFFER", "NV_ENC_ERR_INVALID_VERSION",
"NV_ENC_ERR_MAP_FAILED", "NV_ENC_ERR_NEED_MORE_INPUT", "NV_ENC_ERR_ENCODER_BUSY", "NV_ENC_ERR_EVENT_NOT_REGISTERD",
"NV_ENC_ERR_GENERIC", "NV_ENC_ERR_INCOMPATIBLE_CLIENT_KEY", "NV_ENC_ERR_UNIMPLEMENTED", "NV_ENC_ERR_RESOURCE_REGISTER_FAILED",
"NV_ENC_ERR_RESOURCE_NOT_REGISTERED", "NV_ENC_ERR_RESOURCE_NOT_MAPPED" };

namespace FBCapture {
  namespace Video {

    NVEncoder::NVEncoder() {
      nvHWEncoder_ = new CNvHWEncoder;
      device_ = NULL;
      context_ = NULL;
      tex_ = NULL;
      newTex_ = NULL;
      encodingInitiated_ = false;

      encodeBufferCount_ = 1;

      memset(&eosOutputBfr_, 0, sizeof(eosOutputBfr_));
      memset(&encodeBuffer_, 0, sizeof(encodeBuffer_));
    }

    NVEncoder::~NVEncoder() {
      deinitialize();
      if (nvHWEncoder_) {
        delete nvHWEncoder_;
        nvHWEncoder_ = NULL;
      }
    }

    FBCAPTURE_STATUS NVEncoder::setEncodeConfigures(const wstring& fullSavePath, uint32_t width, uint32_t height, int bitrate, int fps) {
      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      videoFileName_ = stringTypeConversion.to_bytes(fullSavePath);

      // Set encoding configuration
      memset(&encodeConfig_, 0, sizeof(EncodeConfig));
      encodeConfig_.endFrameIdx = INT_MAX;
      encodeConfig_.bitrate = bitrate;
      encodeConfig_.rcMode = NV_ENC_PARAMS_RC_CONSTQP;
      encodeConfig_.gopLength = NVENC_INFINITE_GOPLENGTH;
      encodeConfig_.deviceType = NV_ENC_DX11;
      encodeConfig_.codec = NV_ENC_H264;
      encodeConfig_.fps = fps;
      encodeConfig_.qp = 28;
      encodeConfig_.i_quant_factor = DEFAULT_I_QFACTOR;
      encodeConfig_.b_quant_factor = DEFAULT_B_QFACTOR;
      encodeConfig_.i_quant_offset = DEFAULT_I_QOFFSET;
      encodeConfig_.b_quant_offset = DEFAULT_B_QOFFSET;
      encodeConfig_.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
      encodeConfig_.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
      encodeConfig_.inputFormat = NV_ENC_BUFFER_FORMAT_ABGR;
      encodeConfig_.width = width;
      encodeConfig_.height = height;
      encodeConfig_.outputFileName = videoFileName_.c_str();
      encodeConfig_.fOutput = fopen(encodeConfig_.outputFileName, "wb");
      if (encodeConfig_.fOutput == NULL) {
        DEBUG_ERROR_VAR("Failed to create ", videoFileName_.c_str());
        return FBCAPTURE_STATUS_OUTPUT_FILE_OPEN_FAILED;
      }

      DEBUG_LOG("Video Codec info: NV_ENC_H264");
      DEBUG_LOG_VAR("Bitrate:", std::to_string(encodeConfig_.bitrate));
      DEBUG_LOG("Device type: DirectX 11");

      return FBCAPTURE_STATUS_OK;
    }

    void NVEncoder::setGraphicsDeviceD3D11(ID3D11Device *device) {
      if (device) {
        device_ = device;
        createResources();
      }
    }

    void NVEncoder::releaseD3D11Resources() {
      SAFE_RELEASE(tex_);
      SAFE_RELEASE(newTex_);
      SAFE_RELEASE(context_);
      SAFE_RELEASE(vb_);
      SAFE_RELEASE(cb_);
      SAFE_RELEASE(vertexShader_);
      SAFE_RELEASE(pixelShader_);
      SAFE_RELEASE(inputLayout_);
      SAFE_RELEASE(rasterState_);
      SAFE_RELEASE(blendState_);
      SAFE_RELEASE(depthState_);
    }

    NVENCSTATUS NVEncoder::deinitialize() {
      NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

      releaseD3D11Resources();
      releaseIOBuffers();

      if (encodeConfig_.fOutput) {
        fclose(encodeConfig_.fOutput);
      }

      if (nvHWEncoder_) {
        nvStatus = nvHWEncoder_->NvEncDestroyEncoder();
        if (nvStatus != NV_ENC_SUCCESS) {
          DEBUG_ERROR_VAR("Failed on NvEncDestroyEncoder: ", nvidiaStatus[nvStatus]);
          return nvStatus;
        }
      }

      return nvStatus;
    }

    void NVEncoder::releaseIOBuffers() {
      for (uint32_t i = 0; i < encodeBufferCount_; i++) {
        if (encodeBuffer_[i].stInputBfr.pNVSurface) {
          encodeBuffer_[i].stInputBfr.pNVSurface->Release();
        }

        if (encodeBuffer_[i].stInputBfr.hInputSurface) {
          nvHWEncoder_->NvEncDestroyInputBuffer(encodeBuffer_[i].stInputBfr.hInputSurface);
          encodeBuffer_->stInputBfr.hInputSurface = NULL;
        }

        if (encodeBuffer_[i].stOutputBfr.hBitstreamBuffer) {
          nvHWEncoder_->NvEncDestroyBitstreamBuffer(encodeBuffer_[i].stOutputBfr.hBitstreamBuffer);
          encodeBuffer_->stOutputBfr.hBitstreamBuffer = NULL;
        }

        if (encodeBuffer_[i].stOutputBfr.hOutputEvent) {
          nvHWEncoder_->NvEncUnregisterAsyncEvent(encodeBuffer_[i].stOutputBfr.hOutputEvent);
          CloseHandle(encodeBuffer_[i].stOutputBfr.hOutputEvent);
          encodeBuffer_[i].stOutputBfr.hOutputEvent = NULL;
        }
      }

      if (eosOutputBfr_.hOutputEvent) {
        nvHWEncoder_->NvEncUnregisterAsyncEvent(eosOutputBfr_.hOutputEvent);
        CloseHandle(eosOutputBfr_.hOutputEvent);
        eosOutputBfr_.hOutputEvent = NULL;
      }
    }

    NVENCSTATUS NVEncoder::flushEncoder() {
      NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

      if (eosOutputBfr_.hOutputEvent == NULL)
        return NV_ENC_ERR_INVALID_CALL;

      nvStatus = nvHWEncoder_->NvEncFlushEncoderQueue(eosOutputBfr_.hOutputEvent);
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR_VAR("Failed on flush. Error code: ", nvidiaStatus[nvStatus]);
        assert(0);
        return nvStatus;
      }

      EncodeBuffer *encodeBuffer = encodeBufferQueue_.getPending();
      while (encodeBuffer) {
        nvHWEncoder_->ProcessOutput(encodeBuffer);
        encodeBuffer = encodeBufferQueue_.getPending();
        // UnMap the input buffer after frame is done
        if (encodeBuffer && encodeBuffer->stInputBfr.hInputSurface) {
          nvStatus = nvHWEncoder_->NvEncUnmapInputResource(encodeBuffer->stInputBfr.hInputSurface);
          encodeBuffer->stInputBfr.hInputSurface = NULL;
        }
      }

      if (WaitForSingleObject(eosOutputBfr_.hOutputEvent, 500) != WAIT_OBJECT_0) {
        assert(0);
        nvStatus = NV_ENC_ERR_GENERIC;
      }

      return nvStatus;
    }

    NVENCSTATUS NVEncoder::allocateIOBuffers(uint32_t width, uint32_t height, NV_ENC_BUFFER_FORMAT inputFormat) {
      NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

      if (!encodingInitiated_) {
        // Initialize encode buffer
        encodeBufferQueue_.initialize(encodeBuffer_, encodeBufferCount_);
        for (uint32_t i = 0; i < encodeBufferCount_; i++) {

          // Create input buffer
          nvStatus = nvHWEncoder_->NvEncCreateInputBuffer(width, height, &encodeBuffer_[i].stInputBfr.hInputSurface, inputFormat);
          if (nvStatus != NV_ENC_SUCCESS) {
            DEBUG_ERROR_VAR("Creating input buffer has failed. Error code: ", nvidiaStatus[nvStatus]);
            return nvStatus;
          }

          encodeBuffer_[i].stInputBfr.bufferFmt = inputFormat;
          encodeBuffer_[i].stInputBfr.dwWidth = width;
          encodeBuffer_[i].stInputBfr.dwHeight = height;
          // Bit stream buffer
          nvStatus = nvHWEncoder_->NvEncCreateBitstreamBuffer(BITSTREAM_BUFFER_SIZE, &encodeBuffer_[i].stOutputBfr.hBitstreamBuffer);
          if (nvStatus != NV_ENC_SUCCESS) {
            DEBUG_ERROR_VAR("Creating bit stream buffer has failed. Error code: ", nvidiaStatus[nvStatus]);
            return nvStatus;
          }

          encodeBuffer_[i].stOutputBfr.dwBitstreamBufferSize = BITSTREAM_BUFFER_SIZE;
          // hOutputEvent
          nvStatus = nvHWEncoder_->NvEncRegisterAsyncEvent(&encodeBuffer_[i].stOutputBfr.hOutputEvent);
          if (nvStatus != NV_ENC_SUCCESS) {
            DEBUG_ERROR_VAR("Registering async event has failed. Error code: ", nvidiaStatus[nvStatus]);
            return nvStatus;
          }
        }
        eosOutputBfr_.bEOSFlag = TRUE;
      }

      encodingInitiated_ = true;

      nvStatus = nvHWEncoder_->NvEncRegisterAsyncEvent(&eosOutputBfr_.hOutputEvent);
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR_VAR("Registering asyn event has failed. Error code: ", nvidiaStatus[nvStatus]);
        return nvStatus;
      }

      return nvStatus;
    }

    HRESULT NVEncoder::createResources() {
      HRESULT hr = S_OK;

      D3D11_BUFFER_DESC desc;
      memset(&desc, 0, sizeof(desc));

      // vertex buffer
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.ByteWidth = 1024;
      desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      hr = device_->CreateBuffer(&desc, NULL, &vb_);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create buffer for vertex");
        return hr;
      }

      // constant buffer
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.ByteWidth = 64; // hold 1 matrix
      desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      desc.CPUAccessFlags = 0;
      hr = device_->CreateBuffer(&desc, NULL, &cb_);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create buffer for constant");
        return hr;
      }

      // shaders
      hr = device_->CreateVertexShader(kVertexShaderCode, sizeof(kVertexShaderCode), nullptr, &vertexShader_);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create vertex shader");
        return hr;
      }
      hr = device_->CreatePixelShader(kPixelShaderCode, sizeof(kPixelShaderCode), nullptr, &pixelShader_);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create pixel shader");
        return hr;
      }

      // input layout
      if (vertexShader_) {
        D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
        {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = device_->CreateInputLayout(s_DX11InputElementDesc, 2, kVertexShaderCode, sizeof(kVertexShaderCode), &inputLayout_);
        if (FAILED(hr)) {
          DEBUG_ERROR("Failed to create input layout");
          return hr;
        }
      }

      // render states
      D3D11_RASTERIZER_DESC rsdesc;
      memset(&rsdesc, 0, sizeof(rsdesc));
      rsdesc.FillMode = D3D11_FILL_SOLID;
      rsdesc.CullMode = D3D11_CULL_NONE;
      rsdesc.DepthClipEnable = TRUE;
      hr = device_->CreateRasterizerState(&rsdesc, &rasterState_);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create rasterizer state");
        return hr;
      }

      D3D11_DEPTH_STENCIL_DESC dsdesc;
      memset(&dsdesc, 0, sizeof(dsdesc));
      dsdesc.DepthEnable = TRUE;
      dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
      dsdesc.DepthFunc = getUsesReverseZ() ? D3D11_COMPARISON_GREATER_EQUAL : D3D11_COMPARISON_LESS_EQUAL;
      hr = device_->CreateDepthStencilState(&dsdesc, &depthState_);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create depth stencil state");
        return hr;
      }

      D3D11_BLEND_DESC bdesc;
      memset(&bdesc, 0, sizeof(bdesc));
      bdesc.RenderTarget[0].BlendEnable = FALSE;
      bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
      hr = device_->CreateBlendState(&bdesc, &blendState_);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create blend state");
        return hr;
      }

      return hr;
    }

    NVENCSTATUS NVEncoder::copyReources(uint32_t width, uint32_t height, bool needFlipping) {
      NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

      D3D11_MAPPED_SUBRESOURCE resource;
      device_->GetImmediateContext(&context_);
      context_->CopyResource(newTex_, tex_);
      unsigned int subresource = D3D11CalcSubresource(0, 0, 0);

      HRESULT hr = context_->Map(newTex_, subresource, D3D11_MAP_READ_WRITE, 0, &resource);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed on context mapping");
        return NV_ENC_ERR_GENERIC;
      }

      // Flip pixels
      if (needFlipping) {
        unsigned char* pixel = (unsigned char*)resource.pData;
        const unsigned int rows = height / 2;
        const unsigned int rowStride = width * 4;
        unsigned char* tmpRow = (unsigned char*)malloc(rowStride);

        int source_offset, target_offset;

        for (uint32_t rowIndex = 0; rowIndex < rows; rowIndex++) {
          source_offset = rowIndex * rowStride;
          target_offset = (height - rowIndex - 1) * rowStride;

          memcpy(tmpRow, pixel + source_offset, rowStride);
          memcpy(pixel + source_offset, pixel + target_offset, rowStride);
          memcpy(pixel + target_offset, tmpRow, rowStride);
        }

        free(tmpRow);
        tmpRow = NULL;
      }

      // Unmap buffer
      context_->Unmap(newTex_, 0);

      // lock input buffer
      NV_ENC_LOCK_INPUT_BUFFER lockInputBufferParams = {};

      uint32_t pitch = 0;
      lockInputBufferParams.bufferDataPtr = NULL;

      nvStatus = nvHWEncoder_->NvEncLockInputBuffer(encodeBuffer_->stInputBfr.hInputSurface, &lockInputBufferParams.bufferDataPtr, &pitch);
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR_VAR("Creating nVidia input buffer failed. [Error code] ", nvidiaStatus[nvStatus]);
        return nvStatus;
      }

      // Write into Encode buffer
      memcpy(lockInputBufferParams.bufferDataPtr, resource.pData, height * resource.RowPitch);

      // Unlock input buffer
      nvStatus = nvHWEncoder_->NvEncUnlockInputBuffer(encodeBuffer_->stInputBfr.hInputSurface);
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR_VAR("Failed on nVidia unlock input buffer. [Error code] ", nvidiaStatus[nvStatus]);
        return nvStatus;
      }

      return nvStatus;
    }


    FBCAPTURE_STATUS NVEncoder::encodeMain(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) {
      int numFramesEncoded = 0;
      NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      // Create new texture based on RenderTexture in Unity
      if (!encodingInitiated_ && texturePtr) {
        D3D11_TEXTURE2D_DESC desc;
        tex_ = (ID3D11Texture2D*)texturePtr;
        tex_->GetDesc(&desc);
        desc.BindFlags = 0;
        desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        HRESULT hr = device_->CreateTexture2D(&desc, NULL, &newTex_);
        if (FAILED(hr)) {
          DEBUG_ERROR("Failed on creating new texture");
          return FBCAPTURE_STATUS_TEXTURE_CREATION_FAILED;
        }
        // Set Encode Configs
        if (setEncodeConfigures(fullSavePath, desc.Width, desc.Height, bitrate, fps) != FBCAPTURE_STATUS_OK) {
          return status;
        } else {
          DEBUG_LOG("Set encode configurations successfully");
        }
      }

      if (!encodeConfig_.outputFileName || encodeConfig_.width == 0 || encodeConfig_.height == 0) {
        DEBUG_ERROR("Invalid texture file");
        return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
      }

      // Initialize Encoder
      if (!encodingInitiated_) {
        nvStatus = nvHWEncoder_->Initialize(device_, NV_ENC_DEVICE_TYPE_DIRECTX);
        if (nvStatus == NV_ENC_ERR_INVALID_VERSION) {
          DEBUG_ERROR("Not supported NVidia graphics driver version. Driver version should be 379.95 or newer");
          fclose(encodeConfig_.fOutput);
          remove(videoFileName_.c_str());
          return FBCAPTURE_STATUS_UNSUPPORTED_GRAPHICS_CARD_DRIVER_VERSION;
        } else if (nvStatus != NV_ENC_SUCCESS) {
          DEBUG_ERROR_VAR("Failed on initializing encoder. [Error code]", nvidiaStatus[nvStatus]);
          fclose(encodeConfig_.fOutput);
          remove(videoFileName_.c_str());
          return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
        }

        encodeConfig_.presetGUID = nvHWEncoder_->GetPresetGUID(encodeConfig_.encoderPreset, encodeConfig_.codec);

        nvStatus = nvHWEncoder_->CreateEncoder(&encodeConfig_);
        if (nvStatus != NV_ENC_SUCCESS) {
          DEBUG_ERROR_VAR("Failed on creating encoder. [Error code]", nvidiaStatus[nvStatus]);
          return FBCAPTURE_STATUS_ENCODER_CREATION_FAILED;
        }
      }

      nvStatus = allocateIOBuffers(encodeConfig_.width, encodeConfig_.height, encodeConfig_.inputFormat);
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR_VAR("Failed on allocating IO buffers. [Error code]", nvidiaStatus[nvStatus]);
        return FBCAPTURE_STATUS_IO_BUFFER_ALLOCATION_FAILED;
      }

      //Copy framebuffer to encoding buffers
      nvStatus = copyReources(encodeConfig_.width, encodeConfig_.height, needFlipping);
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR("Failed on copying framebuffers to encode  input buffers");
        return FBCAPTURE_STATUS_TEXTURE_RESOURCES_COPY_FAILED;
      }

      // Encode
      nvStatus = encodeFrame(encodeConfig_.width, encodeConfig_.height, encodeConfig_.inputFormat);
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR("Failed on copying framebuffers to encode  input buffers");
        return FBCAPTURE_STATUS_ENCODE_PICTURE_FAILED;
      }

      return status;
    }

    FBCAPTURE_STATUS NVEncoder::flushEncodedImages() {
      NVENCSTATUS nvStatus;

      encodingInitiated_ = false;

      nvStatus = flushEncoder();
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR_VAR("Failed to flush inputs from buffer. [Error code] ", nvidiaStatus[nvStatus]);
        return FBCAPTURE_STATUS_ENCODE_FLUSH_FAILED;
      }

      nvStatus = deinitialize();
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR_VAR("Failed to release resources. [Error code] ", nvidiaStatus[nvStatus]);
        return FBCAPTURE_STATUS_ENCODE_FLUSH_FAILED;
      }

      return FBCAPTURE_STATUS_OK;
    }

    NVENCSTATUS NVEncoder::encodeFrame(uint32_t width, uint32_t height, NV_ENC_BUFFER_FORMAT inputformat) {
      NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

      uint32_t lockedPitch = 0;
      EncodeBuffer *pEncodeBuffer = NULL;

      int8_t* qpDeltaMapArray = NULL;
      unsigned int qpDeltaMapArraySize = 0;

      pEncodeBuffer = encodeBufferQueue_.getAvailable();
      if (!pEncodeBuffer) {
        nvHWEncoder_->ProcessOutput(encodeBufferQueue_.getPending());
        pEncodeBuffer = encodeBufferQueue_.getAvailable();
      }

      nvStatus = nvHWEncoder_->NvEncEncodeFrame(pEncodeBuffer, NULL, width, height, NV_ENC_PIC_STRUCT_FRAME, qpDeltaMapArray, qpDeltaMapArraySize);
      if (nvStatus != NV_ENC_SUCCESS) {
        DEBUG_ERROR_VAR("Failed on encoding frames. Error code:", nvidiaStatus[nvStatus]);
        return nvStatus;
      }

      return nvStatus;
    }

    FBCAPTURE_STATUS NVEncoder::saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool needFlipping) {
      HRESULT hr = E_FAIL;
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      ID3D11Texture2D* newScreenShotTex;
      ID3D11Texture2D* screenShotTex;
      D3D11_TEXTURE2D_DESC desc;
      newScreenShotTex = (ID3D11Texture2D*)texturePtr;
      newScreenShotTex->GetDesc(&desc);
      desc.BindFlags = 0;
      desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
      desc.Usage = D3D11_USAGE_STAGING;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

      hr = device_->CreateTexture2D(&desc, NULL, &screenShotTex);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on creating new texture for ScreenShot. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_TEXTURE_CREATION_FAILED;
      }

      D3D11_MAPPED_SUBRESOURCE resource;

      device_->GetImmediateContext(&context_);
      context_->CopyResource(screenShotTex, newScreenShotTex);
      unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
      hr = context_->Map(screenShotTex, subresource, D3D11_MAP_READ_WRITE, 0, &resource);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on context mapping. [Error code] ", to_string(hr));
        deinitialize();
        return FBCAPTURE_STATUS_TEXTURE_RESOURCES_COPY_FAILED;
      }

      // Flip pixels
      if (needFlipping) {
        unsigned char* pixel = (unsigned char*)resource.pData;
        const unsigned int rows = desc.Height / 2;
        const unsigned int rowStride = desc.Width * 4;
        unsigned char* tmpRow = (unsigned char*)malloc(rowStride);

        int source_offset, target_offset;

        for (uint32_t rowIndex = 0; rowIndex < rows; rowIndex++) {
          source_offset = rowIndex * rowStride;
          target_offset = (desc.Height - rowIndex - 1) * rowStride;

          memcpy(tmpRow, pixel + source_offset, rowStride);
          memcpy(pixel + source_offset, pixel + target_offset, rowStride);
          memcpy(pixel + target_offset, tmpRow, rowStride);
        }

        free(tmpRow);
        tmpRow = NULL;
      }

      // Unmap buffer
      context_->Unmap(screenShotTex, 0);

      // Screen Capture: Save texture to image file
      hr = SaveWICTextureToFile(context_, screenShotTex, GUID_ContainerFormatJpeg, fullSavePath.c_str());
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed on creating image file. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_WIC_SAVE_IMAGE_FAILED;
      }

      SAFE_RELEASE(screenShotTex);
      SAFE_RELEASE(newScreenShotTex);

      DEBUG_LOG("Succeeded screen capture");

      return status;
    }
  }
}
