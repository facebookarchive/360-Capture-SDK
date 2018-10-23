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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Common/Common.h"
#include "Core/Common/ScopedCOMPtr.h"
#include "Core/Graphics/IGraphicsDeviceCapture.h"
#include "Core/Graphics/TextureRender.h"
#include "Core/Graphics/ScreenPixelShader.h"
#include "Core/Graphics/ScreenVertexShader.h"
#include <atomic>
#include <mfidl.h>
#include <VersionHelpers.h>

namespace FBCapture {
  namespace Camera {
    // Forward Declares
    class CameraDevices;
    class CameraDeviceManager;
    class CameraReader;
  }
}

namespace FBCapture {
  namespace Microphone {
    // Forward Declares
    class MicDevices;
  }
}

using namespace FBCapture::Microphone;
using namespace FBCapture::Camera;
using namespace VrDeviceType;
using namespace StereoMode;
using namespace ProjectionType;

namespace FBCapture {
  namespace Common {

    // Forward Declares
    class EncoderMain;

    // From User
    struct LiveCaptureSettings {
      uint32_t frameRate_;
      float encodeCycle_; // derived from frameRate_
      uint32_t videoBitRate_;
      float flushCycleStart_;
      float flushCycleAfter_;
      float flushCycle_; // derived from flushCycleStart_ and flushCycleAfter_
      std::wstring streamUrl_;
      bool is360_;
      bool verticalFlip_;
      bool horizontalFlip_;
      uint32_t width_;
      uint32_t height_;
      PROJECTIONTYPE projectionType_;
      STEREO_MODE stereoMode_;
    };

    // From User
    struct VodCaptureSettings {
      uint32_t frameRate_;
      float encodeCycle_; // derived from frameRate_
      uint32_t videoBitRate_;
      std::wstring fullSavePath_;
      bool is360_;
      bool verticalFlip_;
      bool horizontalFlip_;
      uint32_t width_;
      uint32_t height_;
      PROJECTIONTYPE projectionType_;
      STEREO_MODE stereoMode_;
    };

    // From User
    struct PreviewCaptureSettings {
      uint32_t frameRate_;
      float encodeCycle_; // derived from frameRate_
      bool is360_;
      bool verticalFlip_;
      bool horizontalFlip_;
      uint32_t width_;
      uint32_t height_;
    };

    // From User
    struct ScreenshotSettings {
      std::wstring fullSavePath_;
      bool is360_;
      bool verticalFlip_;
      bool horizontalFlip_;
      uint32_t width_;
      uint32_t height_;
    };

    // From User
    struct CameraOverlaySettings {
      float widthPercentage_;
      uint32_t viewPortTopLeftX_;
      uint32_t viewPortTopLeftY_;
    };

    // From User
    struct CameraSettings {
      uint32_t cameraDeviceIndexChosen_;
      bool enabledDuringCapture_;
    };

    struct CameraOverlay {
      std::unique_ptr<CameraReader> pCameraReader_ = {};
      ScopedCOMPtr<IMFMediaSource> pCameraMediaSource_ = {};
      ScopedCOMPtr<ID3D11Texture2D> pSharedCameraTexture_ = {};
      ScopedCOMPtr<IDXGIKeyedMutex> pSharedCameraTextureKeyedMutex_ = {};
      uint32_t cameraTextureOverlayWidth_ = {};
      uint32_t cameraTextureOverlayHeight_ = {};
    };

    // From User
    struct MicSettings {
      uint32_t micDeviceIndexChosen_;
      bool enabledDuringCapture_;
      LPWSTR micDeviceIdChosen_; // derived from micDeviceIndexChosen_
    };

    /* TODO AudioRenderSettings
    // From User
    struct AudioRenderSettings {
            uint32_t audioRenderDeviceIndexChosen_;
            uint32_t bitRate_;
            uint32_t sampleRate_;
    };
    */

    enum FBCaptureType {
      kLive = 0,
      kVod = 1,
      kPreview = 2,
      kScreenShot = 3,
      kFBCaptureTypeCount
    };

    class FBCaptureSystem {
    public:
      FBCaptureSystem() {}
      ~FBCaptureSystem() { shutdown(); }
      bool isInitialized() const { return initialized_; }
      FBCAPTURE_STATUS initialize();
      FBCAPTURE_STATUS getCaptureCapability();
      ID3D11Device *getDevice() { return pDevice_; }
      ID3D11DeviceContext *getDeviceContext() { return pContext_; }

      FBCAPTURE_STATUS setGraphicsDeviceD3D11(ID3D11Device* device);

      FBCAPTURE_STATUS setLiveCaptureSettings(int width, int height, int frameRate, int bitRate, float flushCycleStart, float flushCycleAfter, const TCHAR* streamUrl, bool is360, bool verticalFlip, bool horizontalFlip, PROJECTIONTYPE projectionType, STEREO_MODE stereoMode);
      FBCAPTURE_STATUS setVodCaptureSettings(int width, int height, int frameRate, int bitRate, const TCHAR* fullSavePath, bool is360, bool verticalFlip, bool horizontalFlip, PROJECTIONTYPE projectionType, STEREO_MODE stereoMode);
      FBCAPTURE_STATUS setPreviewCaptureSettings(int width, int height, int frameRate, bool is360, bool verticalFlip, bool horizontalFlip);
      FBCAPTURE_STATUS setScreenshotSettings(int width, int height, const TCHAR* fullsavePath, bool is360, bool verticalFlip, bool horizontalFlip);

      FBCAPTURE_STATUS setCameraOverlaySettings(float widthPercentage, uint32_t viewPortTopLeftX, uint32_t viewPortTopLeftY);

      FBCAPTURE_STATUS enumerateMicDevices();
      size_t getMicDevicesCount();
      const char *getMicDeviceName(uint32_t index);
      FBCAPTURE_STATUS setMicDevice(uint32_t index);
      FBCAPTURE_STATUS unsetMicDevice();
      FBCAPTURE_STATUS setMicEnabledDuringCapture(bool enabled);
      FBCAPTURE_STATUS setAudioEnabledDuringCapture(bool enabled);

      FBCAPTURE_STATUS enumerateCameraDevices();
      size_t getCameraDevicesCount();
      const char *getCameraDeviceName(uint32_t index);
      FBCAPTURE_STATUS setCameraDevice(uint32_t index);
      FBCAPTURE_STATUS unsetCameraDevice();
      FBCAPTURE_STATUS setCameraEnabledDuringCapture(bool enabled);

      FBCAPTURE_STATUS setMicAndAudioRenderDeviceByVRDeviceType(VRDeviceType vrDevice);

      FBCAPTURE_STATUS startCapture(FBCaptureType type);
      FBCAPTURE_STATUS startScreenshot(FBCaptureType type);
      FBCAPTURE_STATUS captureTexture(void* texturePtr);
      FBCAPTURE_STATUS previewCapture(void* texturePtr);
      FBCAPTURE_STATUS previewCamera(void* texturePtr);
      FBCAPTURE_STATUS saveScreenShot(void* texturePtr);
      FBCAPTURE_STATUS getCaptureStatus();
      FBCAPTURE_STATUS getScreenshotStatus();
      GRAPHICS_CARD checkGPUManufacturer();

      void stopCapture();
      void shutdown();

    private:
      bool isCameraDevicesInitialized();
      FBCAPTURE_STATUS initializeCameraDevices();

      bool isMicDevicesInitialized();
      FBCAPTURE_STATUS initializeMicDevices();

      bool initialized_ = {};
      ScopedCOMPtr<ID3D11Device> pDevice_ = {};
      ID3D11Device* pClientDevice_ = {};
      ScopedCOMPtr<ID3D11DeviceContext> pContext_ = {};
      
      std::unique_ptr<TextureRender> pTextureFormatConversion_ = {};
      std::unique_ptr<TextureRender> pPreviewTextureFormatConversion_ = {};
      std::unique_ptr<TextureRender> pScreenShotTextureFormatConversion_ = {};
      std::unique_ptr<IGraphicsDeviceCapture> pGraphicsDeviceCapture_ = {};
      std::unique_ptr<IGraphicsDeviceCapture> pGraphicsDeviceScreenshot_ = {};
      ScopedCOMPtr<ID3D11Texture2D> pEncodingTexture_ = {};
      ScopedCOMPtr<ID3D11Texture2D> pPreviewTexture_ = {};
      ScopedCOMPtr<ID3D11Texture2D> pScreenshotTexture_ = {};

      std::unique_ptr<LiveCaptureSettings> pLiveCaptureSettings_ = {};
      std::unique_ptr<VodCaptureSettings> pVodCaptureSettings_ = {};
      std::unique_ptr<PreviewCaptureSettings> pPreviewCaptureSettings_ = {};
      std::unique_ptr<ScreenshotSettings> pScreenshotSettings_ = {};

      std::unique_ptr<MicSettings> pMicSettings_ = {};
      std::unique_ptr<MicDevices> pMicDevices_ = {};

      std::unique_ptr<CameraSettings> pCameraSettings_ = {};
      std::unique_ptr<CameraOverlaySettings> pCameraOverlaySettings_ = {};
      std::unique_ptr<CameraDevices> pCameraDevices_ = {};
      std::unique_ptr<CameraDeviceManager> pCameraDeviceManager_ = {};
      std::unique_ptr<CameraOverlay> pCameraOverlay_ = {};

      std::unique_ptr<EncoderMain> pEncoder_ = {};

      // TODO remove/replace with AudioRenderSettings
      bool vrDeviceRequested_ = {};
      bool audioEnabledDuringCapture_ = {};
      VRDeviceType vrDevice_ = VRDeviceType::NONE;

      // Runtime controls
      FBCAPTURE_STATUS initializeSystemIfNeeded();
      bool activateCameraDevice(std::lock_guard<std::mutex> &);
      bool deactivateCameraDevice(std::lock_guard<std::mutex> &);

      bool doesCameraReaderExist();
      bool isCameraReaderInitialized();
      bool isCameraReaderTextureAvailable();

      void doCameraOverlay(bool fullQuad);
      void doEncodeTextureRender(bool onlyCamera);
      FBCAPTURE_STATUS doPreview(void* texturePtr, bool onlyCamera);
      FBCAPTURE_STATUS doPreviewTextureRender(HANDLE* sharedHandle);
      FBCAPTURE_STATUS doAudioCapture();

      void doEncoderStopRoutine();

      void captureFailed(FBCAPTURE_STATUS reason);
      void screenshotFailed(FBCAPTURE_STATUS reason);

      uint32_t getCaptureWidth(FBCaptureType type);
      uint32_t getCaptureHeight(FBCaptureType type);
      float getCaptureEncodeCycle(FBCaptureType type);
      bool getCaptureNeedsVerticalFlip(FBCaptureType type);
      bool getCaptureNeedsHorizontalFlip(FBCaptureType type);
      bool getCaptureIs360(FBCaptureType type);

      FBCAPTURE_STATUS checkOSCapability();

      void muxThreadRun();
      void audioThreadRun();
      void encodeThreadRun();
      void cameraThreadRun();
      void stopRoutineThreadRun();
      void screenshotThreadRun();
      void screenshotThreadManagerRun();

      std::thread *muxThread_ = {};
      std::thread *audioThread_ = {};
      std::thread *encodeThread_ = {};
      std::thread *cameraThread_ = {};
      std::thread *stopRoutineThread_ = {};
      std::thread *screenshotThread_ = {};
      std::thread *screenshotThreadManager_ = {};

      std::mutex audioCaptureMutex_ = {};
      std::mutex flushMutex_ = {};
      std::mutex micModificationMutex_ = {};
      std::mutex cameraModificationMutex_ = {};
      std::mutex stopRoutineMutex_ = {};

      std::atomic<bool> continueCapture_ = {};
      std::atomic<bool> captureFailed_ = {};
      std::atomic<bool> screenshotFailed_ = {};
      FBCAPTURE_STATUS captureFailureReason_ = FBCAPTURE_STATUS_OK;
      std::mutex captureFailedMutex_ = {};
      std::mutex screenshotFailedMutex_ = {};

      std::atomic<bool> encoding_ = {};
      std::atomic<bool> flush_ = {};
      std::atomic<bool> captureInProgress_ = {};
      std::atomic<bool> stopRequested_ = {};
      std::atomic<bool> screenshotInProgress_ = {};
      std::atomic<bool> terminateScreenshotThread_ = {};
      FBCaptureType captureInProgressType_ = {};
      bool captureTextureReceieved_ = {};
      bool screenshotTextureReceieved_ = {};
      bool unsupportedEncodingEnv_ = { true };

      typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
      LPFN_ISWOW64PROCESS fnIsWow64Process;
    };
  }
}
