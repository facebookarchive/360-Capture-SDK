/****************************************************************************************************************

Filename	:	AMDEncoder.cpp
Content		:	AMD Encoder implementation for creating h264 format video
Created		:	Jan 26, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#include "AMDEncoder.h"

namespace FBCapture {
  namespace Video {

    PollingThread::PollingThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName)
      : context_(context), encoder_(encoder) {
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
        if (data != nullptr) {
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
    }

    AMDEncoder::AMDEncoder(ID3D11Device* device) {
      memoryTypeIn_ = amf::AMF_MEMORY_DX11;
      formatIn_ = amf::AMF_SURFACE_RGBA;

      frameRateIn_ = 30;
      bitRateIn_ = 5000000L; // in bits, 5MBit      
      maximumSpeed_ = true;
      encodingConfigInitiated_ = false;
      device_ = device;
    }

    AMDEncoder::~AMDEncoder() {
      if (surfaceIn_) {
        surfaceIn_ = nullptr;
      }

      if (encoder_) {
        encoder_->Terminate();
        encoder_ = nullptr;
      }

      if (context_) {
        context_->Terminate();
        context_ = nullptr;
      }

      if (g_AMFFactory.GetFactory()) {
        g_AMFFactory.Terminate();
      }
    }

    FBCAPTURE_STATUS AMDEncoder::flushInputTextures() {
      AMF_RESULT res = AMF_OK;  // error checking can be added later

      encodingConfigInitiated_ = false;

      while (true) {
        res = encoder_->Drain();
        if (res != AMF_INPUT_FULL)  // handle full queue
        {
          break;
        }
        amf_sleep(1);  // input queue is full: wait and try again
      }

      thread_->WaitForStop();

      if (thread_) {
        delete thread_;
      }

      encoder_->Flush();

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AMDEncoder::initializeEncodingComponents(const wstring& fullSavePath, int bitrate, int fps) {
      AMF_RESULT hr = AMF_OK; // error checking can be added later

      codec_ = AMFVideoEncoderVCE_AVC;

      memoryTypeIn_ = amf::AMF_MEMORY_DX11;
      formatIn_ = amf::AMF_SURFACE_RGBA;

      frameRateIn_ = fps;
      bitRateIn_ = bitrate;
      maximumSpeed_ = true;
      encodingConfigInitiated_ = false;

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
        encoder_->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH);
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);
        if (hr != AMF_OK) {
          DEBUG_ERROR_VAR("Failed to set proprty: ", "AMF_VIDEO_ENCODER_PROFILE_LEVEL & 51");
          return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
        }
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
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
        encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_TIER, AMF_VIDEO_ENCODER_HEVC_TIER_HIGH);
        hr = encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_PROFILE_LEVEL, AMF_LEVEL_5_1);
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

      thread_ = new PollingThread(context_, encoder_, fullSavePath.c_str());
      thread_->Start();

      encodingConfigInitiated_ = true;

      DEBUG_LOG("Encoding configuration is initiated");

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AMDEncoder::encodeMain(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) {
      AMF_RESULT hr = AMF_OK; // error checking can be added later
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      createD3D11Resources((ID3D11Texture2D*)texturePtr);

      if (!encodingConfigInitiated_) {
        status = initializeEncodingComponents(fullSavePath, bitrate, fps);
        if (status != FBCAPTURE_STATUS_OK) {
          DEBUG_ERROR("Initial configuration setting is failed");
          return status;
        }
      }

      surfaceIn_ = nullptr;
      hr = context_->AllocSurface(memoryTypeIn_, formatIn_, widthIn_, heightIn_, &surfaceIn_);
      if (hr != AMF_OK) {
        DEBUG_ERROR("Failed to allocate surface");
        return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
      }
      status = fillSurface(surfaceIn_);
      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR("Failed to copy texture resources");
        return status;
      }

      // encode
      amf_pts start_time = amf_high_precision_clock();
      surfaceIn_->SetProperty(START_TIME_PROPERTY, start_time);

      hr = encoder_->SubmitInput(surfaceIn_);
      if (hr == AMF_INPUT_FULL) {  // handle full queue
        amf_sleep(1); // input queue is full: wait, poll and submit again
      } else {
        surfaceIn_ = nullptr;
      }

      return status;
    }

    FBCAPTURE_STATUS AMDEncoder::fillSurface(amf::AMFSurface *surface) {
      AMF_RESULT hr = AMF_OK;

      ID3D11Device* amfDevice = (ID3D11Device *)context_->GetDX11Device(); // no reference counting - do not Release()
      ID3D11Texture2D* amdEncodingTexture_ = {};
      amdEncodingTexture_ = (ID3D11Texture2D*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
      ScopedCOMPtr<ID3D11DeviceContext> amfContext = {};
      amfDevice->GetImmediateContext(&amfContext);
      amfContext->CopySubresourceRegion(amdEncodingTexture_, 0, 0, 0, 0, fromTexturePtr_, 0, &dirtyRegion_);
      amfContext->Flush();

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AMDEncoder::initAMDEncodingSession() {
      if (g_AMFFactory.GetFactory() == nullptr) {
        AMF_RESULT hr = g_AMFFactory.Init();
        if (hr != AMF_OK) {
          DEBUG_ERROR("Failed to initialize AMF Factory. Display driver should be Crimson 17.1.1 or newer");
          return FBCAPTURE_STATUS_UNSUPPORTED_GRAPHICS_CARD_DRIVER_VERSION;
        }

        // Getting AMF header and runtime version information
        char versionBuf[20];
        sprintf(versionBuf, "%I64X", AMF_FULL_VERSION);
        DEBUG_LOG_VAR("AMF version (header): ", versionBuf);
        sprintf(versionBuf, "%I64X", g_AMFFactory.AMFQueryVersion());
        DEBUG_LOG_VAR("AMF version (runtime): ", versionBuf);
      }

      return FBCAPTURE_STATUS_OK;
    }


    FBCAPTURE_STATUS AMDEncoder::createD3D11Resources(ID3D11Texture2D* texture) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
      HRESULT hr = S_OK;

      if (device_ == nullptr) {
        DEBUG_ERROR("DX device wasn't created. Please create DX device");
        return FBCAPTURE_STATUS_DEVICE_CREATING_FAILED;
      }

      ::amf_increase_timer_precision();
      // context
      if (context_ == nullptr) {
        hr = g_AMFFactory.GetFactory()->CreateContext(&context_);
        if (hr != AMF_OK) {
          DEBUG_ERROR("Failed to create AMF context");
          return FBCAPTURE_STATUS_CONTEXT_CREATION_FAILED;
        }
      }

      fromTexturePtr_ = texture;
      fromTexturePtr_->GetDesc(&globalTexDesc_);
      globalTexDesc_.BindFlags = 0;
      globalTexDesc_.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
      globalTexDesc_.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
      globalTexDesc_.Usage = D3D11_USAGE_STAGING;
      globalTexDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

      if (globalTexDesc_.Width > 4096 || globalTexDesc_.Height > 2048) {
        DEBUG_ERROR("Invalid texture resolution. Max resolution is 4096 x 2048 on AMD graphics card");
        return FBCAPTURE_STATUS_INVALID_TEXTURE_RESOLUTION;
      }

      // Set video resolution
      widthIn_ = globalTexDesc_.Width;
      heightIn_ = globalTexDesc_.Height;

      // Set texure dirty region
      dirtyRegion_.left = 0;
      dirtyRegion_.right = widthIn_;
      dirtyRegion_.top = 0;
      dirtyRegion_.bottom = heightIn_;
      dirtyRegion_.front = 0;
      dirtyRegion_.back = 1;

      return status;
    }

    FBCAPTURE_STATUS AMDEncoder::saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool is360) {
      AMF_RESULT hr = AMF_OK; // error checking can be added later
      HRESULT hres = S_OK;
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
      status = createD3D11Resources((ID3D11Texture2D*)(texturePtr));
      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR("Failed to create DirectX 11 resources");
        return status;
      }

      ScopedCOMPtr<ID3D11Texture2D> screenshotTexure_;
      hres = device_->CreateTexture2D(&globalTexDesc_, nullptr, &screenshotTexure_);
      if (FAILED(hr)) {
        DEBUG_ERROR("Failed on creating new texture");
        return FBCAPTURE_STATUS_TEXTURE_CREATION_FAILED;
      }

      D3D11_MAPPED_SUBRESOURCE resource = {};
      ScopedCOMPtr<ID3D11DeviceContext> context;
      device_->GetImmediateContext(&context);
      context->CopySubresourceRegion(screenshotTexure_, 0, 0, 0, 0, fromTexturePtr_, 0, &dirtyRegion_);

      // Screen Capture: Save texture to image file
      hres = SaveWICTextureToFile(context, screenshotTexure_, GUID_ContainerFormatJpeg, fullSavePath.c_str(), nullptr, nullptr, is360);
      if (FAILED(hres)) {
        DEBUG_ERROR("Failed to save texture to file");
        return FBCAPTURE_STATUS_WIC_SAVE_IMAGE_FAILED;
      }

      return status;
    }
  }
}




