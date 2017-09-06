/****************************************************************************************************************

Filename	:	AMDEncoder.cpp
Content   :	AMD Encoder implementation for creating h264 format video
Created   :	Jan 26, 2017
Authors   :	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#include "AMDEncoder.h"

namespace FBCapture {
  namespace Video {

    PollingThread::PollingThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName, amf_int32 frameCount) : context_(context), encoder_(encoder), file_(NULL), frameCount_(frameCount) {
      file_ = _wfopen(pFileName, L"wb");
    }
    PollingThread::~PollingThread() {
      if (file_) {
        fclose(file_);
      }
    }
    void PollingThread::Run() {
      RequestStop();

      amf_pts latencyTime = 0;
      amf_pts writeDuration = 0;
      amf_pts encodeDuration = 0;
      amf_pts lastPollTime = 0;

      AMF_RESULT res = AMF_OK; // error checking can be added later
      while (true) {
        amf::AMFDataPtr data;
        res = encoder_->QueryOutput(&data);
        if (res == AMF_EOF) {
          break; // Drain complete
        }
        if (data != NULL) {
          amf_pts pollTime = amf_high_precision_clock();
          amf_pts startTime = 0;
          data->GetProperty(START_TIME_PROPERTY, &startTime);
          if (startTime < lastPollTime) // remove wait time if submission was faster then encode
          {
            startTime = lastPollTime;
          }
          lastPollTime = pollTime;

          encodeDuration += pollTime - startTime;

          if (latencyTime == 0) {
            latencyTime = pollTime - startTime;
          }

          amf::AMFBufferPtr buffer(data); // query for buffer interface
          fwrite(buffer->GetNative(), 1, buffer->GetSize(), file_);

          writeDuration += amf_high_precision_clock() - pollTime;
        } else {
          amf_sleep(1);
        }
      }

      encoder_ = NULL;
      context_ = NULL;
    }

    AMDEncoder::AMDEncoder() {
      memoryTypeIn_ = amf::AMF_MEMORY_DX11;
      formatIn_ = amf::AMF_SURFACE_RGBA;

      frameRateIn_ = 30;
      bitRateIn_ = 5000000L; // in bits, 5MBit
      rectSize_ = 50;
      frameCount_ = 2000;
      maximumSpeed_ = true;
      encodingConfigInitiated_ = false;
    }

    FBCAPTURE_STATUS AMDEncoder::flushEncodedImages() {
      AMF_RESULT res = AMF_OK;  // error checking can be added later

      while (true) {
        res = encoder_->Drain();
        if (res != AMF_INPUT_FULL)  // handle full queue
        {
          break;
        }
        amf_sleep(1);  // input queue is full: wait and try again
      }

      thread_->WaitForStop();

      // cleanup in this order
      surfaceIn_ = NULL;
      if (encoder_) {
        encoder_->Terminate();
        encoder_ = NULL;
      }
      if (context_) {
        context_->Terminate();
        context_ = NULL;  // context is the last
      }

      if (thread_) {
        delete thread_;
      }

      encodingConfigInitiated_ = false;

      if (g_AMFFactory.GetFactory()) {
        res = g_AMFFactory.Terminate();
        if (res != AMF_OK) {
          DEBUG_ERROR("Faield to release AMF resources");
          return FBCAPTURE_STATUS_ENCODE_FLUSH_FAILED;
        }
      }

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AMDEncoder::initialization(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps) {
      AMF_RESULT hr = AMF_OK; // error checking can be added later

      codec_ = AMFVideoEncoderVCE_AVC;

      memoryTypeIn_ = amf::AMF_MEMORY_DX11;
      formatIn_ = amf::AMF_SURFACE_RGBA;

      frameRateIn_ = fps;
      bitRateIn_ = bitrate; // in bits
      rectSize_ = 50;
      frameCount_ = 2000;
      maximumSpeed_ = true;
      encodingConfigInitiated_ = false;


      if (g_AMFFactory.GetFactory() == NULL) {
        hr = g_AMFFactory.Init();
        if (hr != AMF_OK) {
          DEBUG_ERROR("Failed to initialize AMF Factory. Display driver should be Crimson 17.1.1 or newer");
          return FBCAPTURE_STATUS_UNSUPPORTED_GRAPHICS_CARD_DRIVER_VERSION;
        }
      }

      ::amf_increase_timer_precision();
      // context
      if (context_ == NULL) {
        hr = g_AMFFactory.GetFactory()->CreateContext(&context_);
        if (hr != AMF_OK) {
          DEBUG_ERROR("Failed to create AMF context");
          return FBCAPTURE_STATUS_CONTEXT_CREATION_FAILED;
        }
        if (memoryTypeIn_ == amf::AMF_MEMORY_DX11) {
          hr = context_->InitDX11(NULL); // can be DX11 device
          if (hr != AMF_OK) {
            DEBUG_ERROR("Failed to initiate dx11 device from AMFContext");
            return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
          }
        }
      }

      deviceDX11_ = (ID3D11Device*)context_->GetDX11Device();

      D3D11_TEXTURE2D_DESC desc;
      tex_ = (ID3D11Texture2D*)texturePtr;
      tex_->GetDesc(&desc);
      desc.BindFlags = 0;
      desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
      desc.Usage = D3D11_USAGE_STAGING;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      widthIn_ = desc.Width;
      heightIn_ = desc.Height;

      // Call pure dx11 function(HRESULT) to create texture which will be used for copying RenderTexture to AMF buffer.
      HRESULT hres = deviceDX11_->CreateTexture2D(&desc, NULL, &newTex_);
      if (FAILED(hres)) {
        DEBUG_ERROR("Failed to create texture 2d");
        return FBCAPTURE_STATUS_TEXTURE_CREATION_FAILED;
      }

      // component: encoder
      hr = g_AMFFactory.GetFactory()->CreateComponent(context_, codec_, &encoder_);
      if (hr != AMF_OK) {
        DEBUG_ERROR("Failed to create encoder component");
        return FBCAPTURE_STATUS_ENCODER_CREATION_FAILED;
      }

      if (amf_wstring(codec_) == amf_wstring(AMFVideoEncoderVCE_AVC)) {
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty: ", "AMF_VIDEO_ENCODER_USAGE & AMF_VIDEO_ENCODER_USAGE_TRANSCONDING");
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
        if (maximumSpeed_) {
          hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
          if (hr != AMF_OK) {
            DEBUG_ERROR_VAR("Failed to set proprty: ", "AMF_VIDEO_ENCODER_B_PIC_PATTERN & 0");
            return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
          }
          hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
          if (hr != AMF_OK) {
            DEBUG_ERROR_VAR("Failed to set proprty: ", "AMF_VIDEO_ENCODER_QUALITY_PRESET & AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED");
            return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
          }
        }

        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitRateIn_);
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty(AMF_VIDEO_ENCODER_TARGET_BITRATE) ", "bit rate: " + to_string(bitRateIn_));
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(widthIn_, heightIn_));
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty(AMF_VIDEO_ENCODER_FRAMESIZE) ", "width: " + to_string(widthIn_) + " " + "height: " + to_string(heightIn_));
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(frameRateIn_, 1));
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty(AMF_VIDEO_ENCODER_FRAMERATE)", "frame rate: " + to_string(frameRateIn_));
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }

#if defined(ENABLE_4K)
        encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH);
        hr = encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty: ", "AMF_VIDEO_ENCODER_PROFILE_LEVEL & 51");
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
        hr = encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty: ", "AMF_VIDEO_ENCODER_B_PIC_PATTERN & 0");
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
#endif
      } else {
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_USAGE, AMF_VIDEO_ENCODER_HEVC_USAGE_TRANSCONDING);
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty: ", "AMF_VIDEO_ENCODER_HEVC_USAGE & AMF_VIDEO_ENCODER_HEVC_USAGE_TRANSCONDING");
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }

        if (maximumSpeed_) {
          hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET, AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED);
          if (hr != AMF_OK) {
            DEBUG_ERROR_VAR("Failed to set proprty: ", "AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET & AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED");
            return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
          }
        }

        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_TARGET_BITRATE, bitRateIn_);
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty(AMF_VIDEO_ENCODER_HEVC_TARGET_BITRATE) ", "bit rate: " + to_string(bitRateIn_));
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_FRAMESIZE, ::AMFConstructSize(widthIn_, heightIn_));
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty(AMF_VIDEO_ENCODER_HEVC_FRAMESIZE) ", "width: " + to_string(widthIn_) + " " + "height: " + to_string(heightIn_));
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_FRAMERATE, ::AMFConstructRate(frameRateIn_, 1));
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty(AMF_VIDEO_ENCODER_HEVC_FRAMERATE)", "frame rate: " + to_string(frameRateIn_));
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }

#if defined(ENABLE_4K)
        encoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_TIER, AMF_VIDEO_ENCODER_HEVC_TIER_HIGH);
        hr = encoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_PROFILE_LEVEL, AMF_LEVEL_5_1);
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty(AMF_VIDEO_ENCODER_HEVC_PROFILE_LEVEL)", "AMF_LEVEL_5_1");
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
#endif
      }
      hr = encoder_->Init(formatIn_, widthIn_, heightIn_);
      if (hr != AMF_OK) {
        DEBUG_ERROR("Failed on initializing AMF components");
        return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
      }


      thread_ = new PollingThread(context_, encoder_, fullSavePath.c_str(), frameCount_);
      thread_->Start();

      encodingConfigInitiated_ = true;

      // encode some frames
      amf_int32 submitted = 0;
      DEBUG_LOG("Encoding configuration is initiated");

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AMDEncoder::encodeMain(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) {
      AMF_RESULT hr = AMF_OK; // error checking can be added later
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      if (!encodingConfigInitiated_ && texturePtr) {
        status = initialization(texturePtr, fullSavePath, bitrate, fps);
        if (status != FBCAPTURE_STATUS_OK) {
          DEBUG_ERROR("Initial configuration setting is failed");
          return status;
        }
      }

      if (surfaceIn_ == NULL) {
        surfaceIn_ = NULL;
        hr = context_->AllocSurface(memoryTypeIn_, formatIn_, widthIn_, heightIn_, &surfaceIn_);
        if (hr != AMF_OK) {
          DEBUG_ERROR("Failed to allocate surface");
          return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
        }
        status = fillSurface(context_, surfaceIn_, needFlipping);
        if (status != FBCAPTURE_STATUS_OK) {
          DEBUG_ERROR("Failed to copy texture resources");
          return status;
        }
      }
      // encode
      amf_pts start_time = amf_high_precision_clock();
      surfaceIn_->SetProperty(START_TIME_PROPERTY, start_time);

      hr = encoder_->SubmitInput(surfaceIn_);
      if (hr == AMF_INPUT_FULL) {  // handle full queue
        amf_sleep(1); // input queue is full: wait, poll and submit again
      } else {
        surfaceIn_ = NULL;
      }

      return status;
    }

    FBCAPTURE_STATUS AMDEncoder::fillSurface(amf::AMFContext *context, amf::AMFSurface *surface, bool needFlipping) {
      AMF_RESULT hr = AMF_OK; // error checking can be added later
      HRESULT hres;

      // Copy frame buffer to resource
      D3D11_MAPPED_SUBRESOURCE resource;
      ID3D11Texture2D *textureDX11 = (ID3D11Texture2D*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
      ID3D11DeviceContext *contextDX11 = NULL;

      deviceDX11_->GetImmediateContext(&contextDX11);
      contextDX11->CopyResource(newTex_, tex_);
      unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
      hres = contextDX11->Map(newTex_, subresource, D3D11_MAP_READ_WRITE, 0, &resource);
      if (FAILED(hres)) {
        DEBUG_ERROR("Failed to map texture");
        return FBCAPTURE_STATUS_TEXTURE_RESOURCES_COPY_FAILED;
      }

      // Flip pixels
      if (needFlipping) {
        unsigned char* pixel = (unsigned char*)resource.pData;
        const unsigned int rows = heightIn_ / 2;
        const unsigned int rowStride = widthIn_ * 4;
        unsigned char* tmpRow = (unsigned char*)malloc(rowStride);

        int source_offset, target_offset;

        for (uint32_t rowIndex = 0; rowIndex < rows; rowIndex++) {
          source_offset = rowIndex * rowStride;
          target_offset = (heightIn_ - rowIndex - 1) * rowStride;

          memcpy(tmpRow, pixel + source_offset, rowStride);
          memcpy(pixel + source_offset, pixel + target_offset, rowStride);
          memcpy(pixel + target_offset, tmpRow, rowStride);
        }

        free(tmpRow);
        tmpRow = NULL;
      }

      contextDX11->CopyResource(textureDX11, newTex_);

      contextDX11->Unmap(newTex_, 0);
      contextDX11->Flush();
      contextDX11->Release();

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AMDEncoder::saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool needFlipping) {
      AMF_RESULT hr = AMF_OK; // error checking can be added later
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

      if (g_AMFFactory.GetFactory() == NULL) {
        hr = g_AMFFactory.Init();
        if (hr != AMF_OK) {
          DEBUG_ERROR("Failed to initiate AMF");
          return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
        }
      }

      // context
      if (context_ == NULL) {
        hr = g_AMFFactory.GetFactory()->CreateContext(&context_);
        if (hr != AMF_OK) {
          DEBUG_ERROR("Failed to create AMF context");
          return FBCAPTURE_STATUS_CONTEXT_CREATION_FAILED;
        }
        if (memoryTypeIn_ == amf::AMF_MEMORY_DX11) {
          hr = context_->InitDX11(NULL);
          if (hr != AMF_OK) {
            DEBUG_ERROR("Failed to init DX11 in AMF context");
            return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
          }
        }
      }

      ID3D11Device* deviceDX11 = (ID3D11Device*)context_->GetDX11Device();
      ID3D11DeviceContext *contextDX11 = NULL;

      HRESULT hres = deviceDX11->CreateTexture2D(&desc, NULL, &screenShotTex);
      if (FAILED(hres)) {
        DEBUG_ERROR("Failed to create texture 2d");
        return FBCAPTURE_STATUS_TEXTURE_CREATION_FAILED;
      }

      D3D11_MAPPED_SUBRESOURCE resource;

      deviceDX11->GetImmediateContext(&contextDX11);
      contextDX11->CopyResource(screenShotTex, newScreenShotTex);
      unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
      hres = contextDX11->Map(screenShotTex, subresource, D3D11_MAP_READ_WRITE, 0, &resource);
      if (FAILED(hres)) {
        DEBUG_ERROR("Failed to map texture");
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
      contextDX11->Unmap(screenShotTex, 0);

      // Screen Capture: Save texture to image file
      hres = SaveWICTextureToFile(contextDX11, screenShotTex, GUID_ContainerFormatJpeg, fullSavePath.c_str());
      if (FAILED(hres)) {
        DEBUG_ERROR("Failed to save texture to file");
        return FBCAPTURE_STATUS_WIC_SAVE_IMAGE_FAILED;
      }

      SAFE_RELEASE(screenShotTex);
      SAFE_RELEASE(newScreenShotTex);

      return status;
    }
  }
}
