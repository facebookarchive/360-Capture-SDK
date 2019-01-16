/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "FBCaptureSystem.h"
#include "Camera/CameraDevices.h"
#include "Camera/CameraDeviceManager.h"
#include "Camera/CameraReader.h"
#include "EncoderMain.h"
#include "Microphone/MicDevices.h"
#include "Common/Log.h"
#include "Graphics/GraphicsDeviceCaptureD3D11.h"
#include <mfapi.h>
#include <mfreadwrite.h>

// Graphics device identifiers in Unity
enum UnityGfxRenderer {
  kUnityGfxRendererOpenGL = 0, // Legacy OpenGL
  kUnityGfxRendererD3D9 = 1, // Direct3D 9
  kUnityGfxRendererD3D11 = 2, // Direct3D 11
  kUnityGfxRendererGCM = 3, // PlayStation 3
  kUnityGfxRendererNull = 4, // "null" device (used in batch mode)
  kUnityGfxRendererXenon = 6, // Xbox 360
  kUnityGfxRendererOpenGLES20 = 8, // OpenGL ES 2.0
  kUnityGfxRendererOpenGLES30 = 11, // OpenGL ES 3.x
  kUnityGfxRendererGXM = 12, // PlayStation Vita
  kUnityGfxRendererPS4 = 13, // PlayStation 4
  kUnityGfxRendererXboxOne = 14, // Xbox One
  kUnityGfxRendererMetal = 16, // iOS Metal
  kUnityGfxRendererOpenGLCore = 17, // OpenGL core
  kUnityGfxRendererD3D12 = 18, // Direct3D 12
  kGfxRendererCount
};

// Event types for UnitySetGraphicsDevice
enum UnityGfxDeviceEventType {
  kUnityGfxDeviceEventInitialize = 0,
  kUnityGfxDeviceEventShutdown = 1,
  kUnityGfxDeviceEventBeforeReset = 2,
  kUnityGfxDeviceEventAfterReset = 3,
};

namespace FBCapture {
  namespace Common {

    FBCAPTURE_STATUS FBCaptureSystem::initialize() {
      if (pClientDevice_ == nullptr) {
        DEBUG_ERROR(
          "DirectX device hasn't set up. Please set up DirectX 11 device with fbc_setGraphicsDeviceD3D11 function.");
        return FBCAPTURE_STATUS_DEVICE_CREATING_FAILED;
      }

      HRESULT hr = S_OK;
      if (isInitialized()) {
        return FBCAPTURE_STATUS_OK;
      }

      hr = MFStartup(MF_VERSION);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR(
          "Failed to MFStartup in fb capture system. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_SYSTEM_INITIALIZE_FAILED;
      }

      pEncoder_.reset(new EncoderMain());

      const int nvidiaVenderID = 4318;  // NVIDIA Vendor ID: 0x10DE
      const int amdVenderID1 = 4098;  // AMD Vendor ID: 0x1002
      const int amdVenderID2 = 4130;  // AMD Vendor ID: 0x1022

                                      // Get adater where DX device was created by client
      ScopedCOMPtr<IDXGIDevice1> dxgidevice = {};
      ScopedCOMPtr<IDXGIAdapter> displayAdapter = {};
      ScopedCOMPtr<IDXGIFactory1> factory = {};
      DXGI_ADAPTER_DESC adapterDescription = {};
      hr = pClientDevice_->QueryInterface(IID_PPV_ARGS(&dxgidevice));
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to get DXGI device. [Error code] ", hr);
        return FBCAPTURE_STATUS_DXGI_CREATING_FAILED;
      }

      hr = dxgidevice->GetAdapter(&displayAdapter);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to get DXGI adapter. [Error code] ", hr);
        return FBCAPTURE_STATUS_DXGI_CREATING_FAILED;
      }
      if (displayAdapter != nullptr) {
        displayAdapter->GetDesc(&adapterDescription);

        wstring wDeviceName(adapterDescription.Description);
        string sDeviceName(wDeviceName.begin(), wDeviceName.end());
        DEBUG_LOG_VAR("Default Graphics Card Info: ", sDeviceName);

				if (adapterDescription.VendorId == nvidiaVenderID) {
					pEncoder_->setGPUManufacturer(GRAPHICS_CARD::NVIDIA);
				}
				else if (adapterDescription.VendorId == amdVenderID1 || adapterDescription.VendorId == amdVenderID2) {
					pEncoder_->setGPUManufacturer(GRAPHICS_CARD::AMD);
				}
      }

      // We want to enumerate more GPUs if DX device created by client is on unsupported GPU
      if (pEncoder_->checkGPUManufacturer() == GRAPHICS_CARD::UNSUPPORTED_DEVICE) {
        if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory))) {
          DEBUG_ERROR("Failed to create DXGI factory object");
          return FBCAPTURE_STATUS_DXGI_CREATING_FAILED;
        }

        for (UINT i = 0; factory->EnumAdapters(i, &displayAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
          displayAdapter->GetDesc(&adapterDescription);

          // Getting graphics card information in use
          wstring wDeviceName(adapterDescription.Description);
          string sDeviceName(wDeviceName.begin(), wDeviceName.end());
          DEBUG_LOG_VAR("Graphics Card Info: ", sDeviceName);

          if (adapterDescription.VendorId == nvidiaVenderID) {
						pEncoder_->setGPUManufacturer(GRAPHICS_CARD::NVIDIA);
            break;
          } else if (adapterDescription.VendorId == amdVenderID1 || adapterDescription.VendorId == amdVenderID2) {
						pEncoder_->setGPUManufacturer(GRAPHICS_CARD::AMD);
            break;
          }
        }				
      }

      D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };

      DWORD createDeviceFlags = 0;

      //Commenting this line by default since only few test cases require this flag
      //#ifdef DEBUG
      //            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
      //#endif
      hr = D3D11CreateDevice(displayAdapter, displayAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, nullptr,
                             createDeviceFlags, levels, ARRAYSIZE(levels),
                             D3D11_SDK_VERSION, &pDevice_, nullptr, &pContext_);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to D3D11CreateDevice in fb capture system. [Error code] ",
          hr);
        return FBCAPTURE_STATUS_SYSTEM_INITIALIZE_FAILED;
      }
      // Yes, this is D10 Multithread versus D11. It's compatible with D11. Using this as D11Multithread requires an extra Dll:
      // D3d11_4.dll
      ScopedCOMPtr<ID3D10Multithread> pMultithread;
      hr = pDevice_->QueryInterface(__uuidof(ID3D10Multithread),
        (void **)&pMultithread);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to Query ID3D10Multithread Interface in fb capture system. [Error code] ",
                            hr);
        return FBCAPTURE_STATUS_SYSTEM_INITIALIZE_FAILED;
      }

      hr = pMultithread->SetMultithreadProtected(TRUE);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to SetMultithreadProtected fb capture system. [Error code] ",
                            hr);
        return FBCAPTURE_STATUS_SYSTEM_INITIALIZE_FAILED;
      }

      pEncoder_->setGraphicsDeviceD3D11(pDevice_);

      initialized_ = true;
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::getCaptureCapability() {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      // Check OS version capbility
      status = checkOSCapability();
      if (status != FBCAPTURE_STATUS_OK) {
        return status;
      }

      status = initializeSystemIfNeeded();
      if (status != FBCAPTURE_STATUS_OK) {
        return status;
      }      
	
      status = pEncoder_->initEncoderComponents();
      if (status != FBCAPTURE_STATUS_OK) {
        return status;
      }

			// Check graphics card capability
			status = pEncoder_->checkGraphicsCardCapability();
			if (status != FBCAPTURE_STATUS_OK) {	
				pEncoder_->releaseEncodeResources();
				DEBUG_ERROR_VAR("Unsupported graphics card. Software encoder will be enabled.", to_string(status));
				pEncoder_->setSoftwareEncoder();
				return FBCAPTURE_STATUS_SW_ENCODER_ENABLED;
			}

      // Check graphics driver version capability
      status = pEncoder_->initSessionANDcheckDriverCapability();
      if (status   != FBCAPTURE_STATUS_OK) {
        pEncoder_->releaseEncodeResources();
				DEBUG_ERROR_VAR("Failed on initializing GPU encoder. Software encoder will be enabled.", to_string(status));
				pEncoder_->setSoftwareEncoder();
        return FBCAPTURE_STATUS_SW_ENCODER_ENABLED;
      }

      // Need dummy encoding session to clearly clean up nvidia encoder activated to check gpu driver capability
      status = pEncoder_->dummyEncodingSession();
      if (status != FBCAPTURE_STATUS_OK) {
				DEBUG_ERROR_VAR("Failed on GPU encoding of dummy encoding session. Software encoder will be enabled.", to_string(status));
				pEncoder_->setSoftwareEncoder();
        return FBCAPTURE_STATUS_SW_ENCODER_ENABLED;
      }

      DEBUG_LOG("Passed capture capability tests. Ready to start encoding");

      return status;
    }

    FBCAPTURE_STATUS FBCaptureSystem::checkOSCapability() {
      if (!IsWindows7SP1OrGreater()) {
        DEBUG_ERROR("We're supporting Windows 7 SP1 or greater only");
        return FBCAPTURE_STATUS_UNSUPPORTED_OS_VERSION;
      }
      /// Commenting out codes checking os bit for now
      /// But since we haven't tested 32bit enough, we might need to get back them once we see issues on 32 bit
      /*
      BOOL is64bit = false;

      fnIsWow64Process
      = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),
      "IsWow64Process");

      if (nullptr != fnIsWow64Process)	{
      if (!fnIsWow64Process(GetCurrentProcess(), &is64bit)) {
      // TODO ADD handle error
      }
      }

      if (is64bit == false) {
      DEBUG_ERROR("We're supporting 64bit OS only");
      return FBCAPTURE_STATUS_UNSUPPORTED_OS_PROCESSOR;
      }
      */

      return FBCAPTURE_STATUS_OK;
    }


    bool FBCaptureSystem::isCameraDevicesInitialized() {
      return pCameraDevices_ && pCameraDeviceManager_ &&
        pCameraDeviceManager_->isInitialized();
    }

    FBCAPTURE_STATUS FBCaptureSystem::initializeCameraDevices() {
      pCameraDevices_.reset(new CameraDevices());
      pCameraDeviceManager_.reset(new CameraDeviceManager());
      HRESULT hr = pCameraDeviceManager_->initialize();
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to initialize CameraDeviceManager. [Error code] ", hr);
        return FBCAPTURE_STATUS_CAMERA_ENUMERATION_FAILED;
      }
      return FBCAPTURE_STATUS_OK;
    }

    bool FBCaptureSystem::isMicDevicesInitialized() {
      return pMicDevices_ != nullptr;
    }

    FBCAPTURE_STATUS FBCaptureSystem::initializeMicDevices() {
      pMicDevices_.reset(new MicDevices());
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setGraphicsDeviceD3D11(ID3D11Device* device) {
      if (pClientDevice_ == nullptr) {
        pClientDevice_ = device;
      }
      pGraphicsDeviceCapture_.reset(new GraphicsDeviceCaptureD3D11(device));
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setLiveCaptureSettings(int width, int height, int frameRate, int bitRate,
                                                             float flushCycleStart, float flushCycleAfter,
                                                             const TCHAR* streamUrl, bool is360, bool verticalFlip, bool horizontalFlip,
                                                             PROJECTIONTYPE projectionType, STEREO_MODE stereoMode) {
      pLiveCaptureSettings_.reset(new LiveCaptureSettings);
      pLiveCaptureSettings_->width_ = width;
      pLiveCaptureSettings_->height_ = height;
      pLiveCaptureSettings_->frameRate_ = frameRate;
      pLiveCaptureSettings_->encodeCycle_ = 1.0f / frameRate;
      pLiveCaptureSettings_->videoBitRate_ = bitRate;
      pLiveCaptureSettings_->flushCycleStart_ = flushCycleStart;
      pLiveCaptureSettings_->flushCycleAfter_ = flushCycleAfter;
      pLiveCaptureSettings_->streamUrl_ = std::wstring(streamUrl);
      pLiveCaptureSettings_->is360_ = is360;
      pLiveCaptureSettings_->verticalFlip_ = verticalFlip;
      pLiveCaptureSettings_->horizontalFlip_ = horizontalFlip;
      pLiveCaptureSettings_->flushCycle_ = pLiveCaptureSettings_->flushCycleStart_;
      pLiveCaptureSettings_->projectionType_ = projectionType;
      pLiveCaptureSettings_->stereoMode_ = stereoMode;
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setVodCaptureSettings(int width, int height, int frameRate, int bitRate,
                                                            const TCHAR* fullSavePath, bool is360, bool verticalFlip, bool horizontalFlip,
                                                            PROJECTIONTYPE projectionType, STEREO_MODE stereoMode) {
      pVodCaptureSettings_.reset(new VodCaptureSettings);
      pVodCaptureSettings_->width_ = width;
      pVodCaptureSettings_->height_ = height;
      pVodCaptureSettings_->frameRate_ = frameRate;
      pVodCaptureSettings_->encodeCycle_ = 1.0f / frameRate;
      pVodCaptureSettings_->videoBitRate_ = bitRate;
      pVodCaptureSettings_->fullSavePath_ = std::wstring(fullSavePath);
      pVodCaptureSettings_->is360_ = is360;
      pVodCaptureSettings_->verticalFlip_ = verticalFlip;
      pVodCaptureSettings_->horizontalFlip_ = horizontalFlip;
      pVodCaptureSettings_->projectionType_ = projectionType;
      pVodCaptureSettings_->stereoMode_ = stereoMode;
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setPreviewCaptureSettings(int width, int height, int frameRate, bool is360,
                                                                bool verticalFlip, bool horizontalFlip) {
      pPreviewCaptureSettings_.reset(new PreviewCaptureSettings);
      pPreviewCaptureSettings_->width_ = width;
      pPreviewCaptureSettings_->height_ = height;
      pPreviewCaptureSettings_->frameRate_ = frameRate;
      pPreviewCaptureSettings_->encodeCycle_ = 1.0f / frameRate;
      pPreviewCaptureSettings_->is360_ = is360;
      pPreviewCaptureSettings_->verticalFlip_ = verticalFlip;
      pPreviewCaptureSettings_->horizontalFlip_ = horizontalFlip;
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setScreenshotSettings(int width, int height, const TCHAR* fullsavePath, bool is360,
                                                            bool verticalFlip, bool horizontalFlip) {
      pScreenshotSettings_.reset(new ScreenshotSettings);
      pScreenshotSettings_->width_ = width;
      pScreenshotSettings_->height_ = height;
      pScreenshotSettings_->fullSavePath_ = fullsavePath;
      pScreenshotSettings_->is360_ = is360;
      pScreenshotSettings_->verticalFlip_ = verticalFlip;
      pScreenshotSettings_->horizontalFlip_ = horizontalFlip;
      return FBCAPTURE_STATUS_OK;
    }


    FBCAPTURE_STATUS FBCaptureSystem::setCameraOverlaySettings(float widthPercentage, uint32_t viewPortTopLeftX, uint32_t viewPortTopLeftY) {
      std::lock_guard<std::mutex> lock(cameraModificationMutex_);
      pCameraOverlaySettings_.reset(new CameraOverlaySettings);
      pCameraOverlaySettings_->widthPercentage_ = widthPercentage;
      pCameraOverlaySettings_->viewPortTopLeftX_ = viewPortTopLeftX;
      pCameraOverlaySettings_->viewPortTopLeftY_ = viewPortTopLeftY;
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::enumerateMicDevices() {
      if (!isMicDevicesInitialized()) {
        FBCAPTURE_STATUS status = initializeMicDevices();
        if (status != FBCAPTURE_STATUS_OK) {
          return status;
        }
      }
      HRESULT hr = pMicDevices_->enumerateDevices();
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to enumerate mic devices. [Error code] ", hr);
        return FBCAPTURE_STATUS_MIC_ENUMERATION_FAILED;
      }
      return FBCAPTURE_STATUS_OK;
    }

    size_t FBCaptureSystem::getMicDevicesCount() {
      if (!isMicDevicesInitialized()) {
        DEBUG_ERROR("enumerateMicDevices must be called before getMicDevicesCount. ");
        return FBCAPTURE_STATUS_MIC_REQUIRES_ENUMERATION;
      }
      return pMicDevices_->count();
    }

    const char * FBCaptureSystem::getMicDeviceName(uint32_t index) {
      if (!isMicDevicesInitialized()) {
        DEBUG_ERROR("enumerateMicDevices must be called before getMicDeviceName. ");
        return nullptr;
      }
      if (index >= pMicDevices_->count()) {
        DEBUG_ERROR_VAR("index out of bounds for getMicDeviceName. [Count] ", to_string(pMicDevices_->count()));
        return nullptr;
      }
      std::string name;
      pMicDevices_->getDeviceName(index, name);

      size_t allocSize = name.length() + sizeof(char);
      char *result = (char*)::CoTaskMemAlloc(allocSize);
      strcpy_s(result, allocSize, name.c_str());
      return result;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setMicDevice(uint32_t index) {
      if (!isMicDevicesInitialized()) {
        DEBUG_ERROR("enumerateMicDevices must be called before setMicDevice. ");
        return FBCAPTURE_STATUS_MIC_REQUIRES_ENUMERATION;
      }
      if (index >= pMicDevices_->count()) {
        DEBUG_ERROR_VAR("index out of bounds for setMicDevice. [Count] ", to_string(pMicDevices_->count()));
        return FBCAPTURE_STATUS_MIC_INDEX_INVALID;
      }

      FBCAPTURE_STATUS status = unsetMicDevice();
      if (status != FBCAPTURE_STATUS_OK) {
        return status;
      }

      // Support mic switching during capture through lock_guard
      std::lock_guard<std::mutex> lock(micModificationMutex_);
      pMicSettings_.reset(new MicSettings);
      pMicSettings_->micDeviceIndexChosen_ = index;
      pMicSettings_->enabledDuringCapture_ = true;
      HRESULT hr = pMicDevices_->getDeviceId(index, &pMicSettings_->micDeviceIdChosen_);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to get device id for mic. [Error code] ", hr);
        return FBCAPTURE_STATUS_MIC_ENUMERATION_FAILED;
      }
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::unsetMicDevice() {
      // Support mic switching during capture through lock_guard
      std::lock_guard<std::mutex> lock(micModificationMutex_);

      if (!pMicSettings_) {
        return FBCAPTURE_STATUS_OK;
      }
      if (pMicSettings_ && pMicSettings_->micDeviceIdChosen_) {
        pMicDevices_->freeDeviceId(pMicSettings_->micDeviceIdChosen_);
      }
      if (pMicSettings_) {
        pMicSettings_.reset(nullptr);
      }

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setMicEnabledDuringCapture(bool enabled) {
      if (!pMicSettings_) {
        pMicSettings_.reset(new MicSettings);
        pMicSettings_->micDeviceIdChosen_ = nullptr;
      }
      pMicSettings_->enabledDuringCapture_ = enabled;
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setAudioEnabledDuringCapture(bool enabled) {
      audioEnabledDuringCapture_ = enabled;

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::enumerateCameraDevices() {
      if (!isCameraDevicesInitialized()) {
        FBCAPTURE_STATUS status = initializeCameraDevices();
        if (status != FBCAPTURE_STATUS_OK) {
          return status;
        }
      }
      HRESULT hr = pCameraDevices_->enumerateDevices();
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to enumerate camera devices. [Error code] ", hr);
        return FBCAPTURE_STATUS_CAMERA_ENUMERATION_FAILED;
      }
      return FBCAPTURE_STATUS_OK;
    }

    size_t FBCaptureSystem::getCameraDevicesCount() {
      if (!isCameraDevicesInitialized()) {
        DEBUG_ERROR("enumerateCameraDevices must be called before getCameraDevicesCount. ");
        return FBCAPTURE_STATUS_CAMERA_REQUIRES_ENUMERATION;
      }
      return pCameraDevices_->count();
    }

    const char * FBCaptureSystem::getCameraDeviceName(uint32_t index) {
      if (!isCameraDevicesInitialized()) {
        DEBUG_ERROR("enumerateCameraDevices must be called before getCameraDeviceName. ");
        return nullptr;
      }
      if (index >= pCameraDevices_->count()) {
        DEBUG_ERROR_VAR("index out of bounds for getCameraDeviceName. [Count] ", to_string(pCameraDevices_->count()));
        return nullptr;
      }
      std::string name;
      pCameraDevices_->getDeviceName(index, name);

      size_t allocSize = name.length() + sizeof(char);
      char *result = (char*)::CoTaskMemAlloc(allocSize);
      strcpy_s(result, allocSize, name.c_str());
      return result;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setCameraDevice(uint32_t index) {
      if (!isCameraDevicesInitialized()) {
        DEBUG_ERROR("enumerateCameraDevices must be called before setCameraDevice. ");
        return FBCAPTURE_STATUS_CAMERA_REQUIRES_ENUMERATION;
      }
      if (index >= pCameraDevices_->count()) {
        DEBUG_ERROR_VAR("index out of bounds for setCameraDevice. [Count] ", to_string(pCameraDevices_->count()));
        return FBCAPTURE_STATUS_CAMERA_INDEX_INVALID;
      }

      FBCAPTURE_STATUS result = unsetCameraDevice();
      if (result != FBCAPTURE_STATUS_OK) {
        return result;
      }

      // Support camera switching during capture through lock_guard
      std::lock_guard<std::mutex> lock(cameraModificationMutex_);

      pCameraSettings_.reset(new CameraSettings);
      pCameraSettings_->cameraDeviceIndexChosen_ = index;
      pCameraSettings_->enabledDuringCapture_ = true;

      pCameraOverlay_.reset(new CameraOverlay);

      if (!activateCameraDevice(lock)) {
        return FBCAPTURE_STATUS_CAMERA_SET_FAILED;
      }

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::unsetCameraDevice() {
      FBCAPTURE_STATUS result = FBCAPTURE_STATUS_OK;

      // Support camera switching during capture through lock_guard
      std::lock_guard<std::mutex> lock(cameraModificationMutex_);
      if (pCameraSettings_ && pCameraOverlay_ && !deactivateCameraDevice(lock)) {
        return FBCAPTURE_STATUS_CAMERA_UNSET_FAILED;
      }

      pCameraOverlay_.reset(nullptr);
      pCameraSettings_.reset(nullptr);

      return result;
    }

    FBCAPTURE_STATUS FBCaptureSystem::setCameraEnabledDuringCapture(bool enabled) {
      if (!pCameraSettings_) {
        DEBUG_ERROR("setCameraDevice must be called before setCameraEnabledDuringCapture. ");
        return FBCAPTURE_STATUS_CAMERA_DEVICE_NOT_SET;
      }
      pCameraSettings_->enabledDuringCapture_ = enabled;
      return FBCAPTURE_STATUS_OK;
    }

    // TODO AudioRenderSettings
    FBCAPTURE_STATUS FBCaptureSystem::setMicAndAudioRenderDeviceByVRDeviceType(VRDeviceType vrDevice) {
      // TODO setMicAndAudioRenderDeviceByVRDeviceType
      vrDevice_ = vrDevice;
      vrDeviceRequested_ = true;
      // TODO iterate mics and find that which matches the vr device type and call setMicDevice to that index
      // TODO when AudioRenderSettings exist, find that audio render device which matches the vr device type and call setAudioRenderDevice to that index
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::initializeSystemIfNeeded() {
      // Initialize the system device and context if necessary
      if (!isInitialized()) {
        return initialize();
      }
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::startCapture(FBCaptureType type) {     
			FBCAPTURE_STATUS status = getCaptureStatus();
			if (status != FBCAPTURE_STATUS_OK) {				
				return status;
			}

      if (captureInProgress_) {
        DEBUG_ERROR("capture is already in progress. Call stopCapture to stop existing capture. ");
        return FBCAPTURE_STATUS_SYSTEM_CAPTURE_IN_PROGRESS;
      }

      if (type == FBCaptureType::kLive && !pLiveCaptureSettings_) {
        DEBUG_ERROR("setLiveCaptureSettings must be called before starting a live capture. ");
        return FBCAPTURE_STATUS_LIVE_CAPTURE_SETTINGS_NOT_CONFIGURED;
      } else if (type == FBCaptureType::kVod && !pVodCaptureSettings_) {
        DEBUG_ERROR("setVodCaptureSettings must be called before starting a vod capture. ");
        return FBCAPTURE_STATUS_VOD_CAPTURE_SETTINGS_NOT_CONFIGURED;
      } else if (type == FBCaptureType::kPreview && !pPreviewCaptureSettings_) {
        DEBUG_ERROR("setPreviewCaptureSettings must be called before starting a preview capture. ");
				return FBCAPTURE_STATUS_PREVIEW_CAPTURE_SETTINGS_NOT_CONFIGURED;
			}

			// Prepare the system shared texture for passing to the encoder
			const uint32_t width = getCaptureWidth(type);
			const uint32_t height = getCaptureHeight(type);

			D3D11_TEXTURE2D_DESC desc = {};
			HRESULT hr = S_OK;

			pEncodingTexture_ = nullptr;
			ZeroMemory(&desc, sizeof(desc));
			desc.Width = width;
			desc.Height = height;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.ArraySize = 1;
      desc.MipLevels = 1;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      hr = pDevice_->CreateTexture2D(&desc, nullptr,
                                     &pEncodingTexture_);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to create encoding Texture2D FBCaptureSystem. [Error code] ",
          hr);
        return FBCAPTURE_STATUS_SYSTEM_ENCODING_TEXTURE_CREATION_FAILED;
      }

      pPreviewTexture_ = nullptr;
      ZeroMemory(&desc, sizeof(desc));
      desc.Width = width;
      desc.Height = height;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.ArraySize = 1;
      desc.MipLevels = 1;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
      hr = pDevice_->CreateTexture2D(&desc, nullptr,
                                     &pPreviewTexture_);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to create preview Texture2D in FBCaptureSystem. [Error code] ",
          hr);
        return FBCAPTURE_STATUS_SYSTEM_PREVIEW_TEXTURE_CREATION_FAILED;
      }

      pTextureFormatConversion_.reset(new TextureRender());
      bool success = pTextureFormatConversion_->initialize(
																														pDevice_, pContext_,
																														pEncodingTexture_, 
																														screenVertexShaderCode, sizeof(screenVertexShaderCode), 
																														pEncoder_->isSoftwareEncoderEnabled() ? yuvPixelShaderCode : screenPixelShaderCode,
																														pEncoder_->isSoftwareEncoderEnabled() ? sizeof(yuvPixelShaderCode) : sizeof(screenPixelShaderCode),
																														getCaptureNeedsVerticalFlip(type),
																														getCaptureNeedsHorizontalFlip(type));
      if (!success) {
        hr = S_FALSE;
        DEBUG_ERROR(
          "Failed to initialize TextureFormatConversion in FBCaptureSystem.");
        return FBCAPTURE_STATUS_SYSTEM_ENCODING_TEXTURE_FORMAT_CREATION_FAILED;
      }
      pTextureFormatConversion_->setViewport(0, 0, width,
                                             height);

      pPreviewTextureFormatConversion_.reset(new TextureRender());
      success = pPreviewTextureFormatConversion_->initialize(
																															pDevice_, pContext_,
																															pPreviewTexture_, screenVertexShaderCode,
																															sizeof(screenVertexShaderCode), screenPixelShaderCode,
																															sizeof(screenPixelShaderCode),
																															getCaptureNeedsVerticalFlip(type),
																															getCaptureNeedsHorizontalFlip(type));
      if (!success) {
        hr = S_FALSE;
        DEBUG_ERROR(
          "Failed to initialize TextureFormatConversion in FBCaptureSystem.");
        return FBCAPTURE_STATUS_SYSTEM_ENCODING_TEXTURE_FORMAT_CREATION_FAILED;
      }
      pPreviewTextureFormatConversion_->setViewport(0, 0, width, height);

      if (isCameraReaderInitialized()) {
        pCameraOverlay_->cameraTextureOverlayWidth_ = (uint32_t)(
          width * (pCameraOverlaySettings_->widthPercentage_ / 100.0f));
        // Set camera overlay to 4:3 as that's what CameraReader filters to
        pCameraOverlay_->cameraTextureOverlayHeight_ = (uint32_t)(pCameraOverlay_->cameraTextureOverlayWidth_ * 3 / 4);
      }

      if (muxThread_ || audioThread_ || encodeThread_ || cameraThread_) {
        stopCapture();
      }

      continueCapture_ = true;
      captureFailed_ = false;
      captureFailureReason_ = FBCAPTURE_STATUS_OK;
      encoding_ = false;
      flush_ = false;
      captureInProgress_ = true;
      stopRequested_ = false;
      captureInProgressType_ = type;

      if (!pEncoder_->isClientPcmAudioInputEnabled()) {
        audioThread_ = new std::thread(&FBCaptureSystem::audioThreadRun, this);
      }
      muxThread_ = new std::thread(&FBCaptureSystem::muxThreadRun, this);
      encodeThread_ = new std::thread(&FBCaptureSystem::encodeThreadRun, this);
      cameraThread_ = new std::thread(&FBCaptureSystem::cameraThreadRun, this);
      stopRoutineThread_ = new std::thread(&FBCaptureSystem::stopRoutineThreadRun, this);

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::startScreenshot(FBCaptureType type) {

			FBCAPTURE_STATUS status = initializeSystemIfNeeded();
			if (status != FBCAPTURE_STATUS_OK) {
				return status;
			}

      status = getScreenshotStatus();
      if (status == FBCAPTURE_STATUS_SYSTEM_CAPTURE_IN_PROGRESS) {
        DEBUG_LOG("Screenshot is in progress.");
        return status;
      }

      if (type == FBCaptureType::kScreenShot && !pScreenshotSettings_) {
        DEBUG_ERROR("setScreenshotCaptureSettings must be called before starting a screenshot capture. ");
        return FBCAPTURE_STATUS_SCREENSHOT_CAPTURE_SETTINGS_NOT_CONFIGURED;
      }

      // Prepare the system shared texture for passing to the encoder
      const uint32_t width = getCaptureWidth(type);
      const uint32_t height = getCaptureHeight(type);
      D3D11_TEXTURE2D_DESC desc = {};
      HRESULT hr = S_OK;

      pScreenshotTexture_ = nullptr;
      ZeroMemory(&desc, sizeof(desc));
      desc.Width = width;
      desc.Height = height;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.ArraySize = 1;
      desc.MipLevels = 1;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      hr = pDevice_->CreateTexture2D(&desc, nullptr,
                                     &pScreenshotTexture_);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to create screenshot Texture2D FBCaptureSystem. [Error code] ", hr);
        status = FBCAPTURE_STATUS_SYSTEM_ENCODING_TEXTURE_FORMAT_CREATION_FAILED;
        screenshotFailed(status);
        return status;
      }

      pScreenShotTextureFormatConversion_.reset(new TextureRender());
      bool success = pScreenShotTextureFormatConversion_->initialize(
																																			pDevice_, pContext_,
																																			pScreenshotTexture_, screenVertexShaderCode,
																																			sizeof(screenVertexShaderCode), screenPixelShaderCode,
																																			sizeof(screenPixelShaderCode),
																																			getCaptureNeedsVerticalFlip(type),
																																			getCaptureNeedsHorizontalFlip(type));
      if (!success) {
        hr = S_FALSE;
        DEBUG_ERROR("Failed to initialize ScreenShotTextureFormatConversion in FBCaptureSystem.");
        status = FBCAPTURE_STATUS_SYSTEM_ENCODING_TEXTURE_FORMAT_CREATION_FAILED;
        screenshotFailed(status);
        return status;
      }
      pScreenShotTextureFormatConversion_->setViewport(0, 0, width, height);
      pGraphicsDeviceScreenshot_.reset(new GraphicsDeviceCaptureD3D11(pDevice_));

      terminateScreenshotThread_ = false;
      screenshotTextureReceieved_ = false;
      screenshotInProgress_ = true;

      screenshotThread_ = new std::thread(&FBCaptureSystem::screenshotThreadRun, this);
      screenshotThreadManager_ = new std::thread(&FBCaptureSystem::screenshotThreadManagerRun, this);

      return FBCAPTURE_STATUS_OK;
    }


    FBCAPTURE_STATUS FBCaptureSystem::captureTexture(void* texturePtr) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      // Sometimes Unity is failed on invoking texture pointer on init and it causes encoding failure with null texture
      // So I want to skip invaild texture pointer here
      if (texturePtr == nullptr) {
        DEBUG_LOG("Null texture was received.");
        return status;
      }

      status = getCaptureStatus();
      if (status != FBCAPTURE_STATUS_OK && status != FBCAPTURE_STATUS_SYSTEM_CAPTURE_IN_PROGRESS) {
        if (!captureInProgress_) {
          DEBUG_ERROR("capture is not in progress. Call startCapture to start. ");
          return FBCAPTURE_STATUS_SYSTEM_CAPTURE_NOT_IN_PROGRESS;
        } else {
          return status;
        }
      }

      // Copy the user graphics device texturePtr to a separate shared texture on the user graphics device
      FBCAPTURE_STATUS result = pGraphicsDeviceCapture_->captureTextureToSharedHandle(texturePtr);
      if (result == FBCAPTURE_STATUS_OK) {
        captureTextureReceieved_ = true;
      } else {
        captureFailed(result);
        stopCapture();
      }
      return result;
    }

    FBCAPTURE_STATUS FBCaptureSystem::previewCapture(void* texturePtr) {
      FBCAPTURE_STATUS status = getCaptureStatus();
      if (status != FBCAPTURE_STATUS_OK && status != FBCAPTURE_STATUS_SYSTEM_CAPTURE_IN_PROGRESS) {
        return status;
      }
      return doPreview(texturePtr, false);
    }

    FBCAPTURE_STATUS FBCaptureSystem::previewCamera(void* texturePtr) {
      FBCAPTURE_STATUS status = getCaptureStatus();
      if (status != FBCAPTURE_STATUS_OK && status != FBCAPTURE_STATUS_SYSTEM_CAPTURE_IN_PROGRESS) {
        return status;
      }
      return doPreview(texturePtr, true);
    }

    FBCAPTURE_STATUS FBCaptureSystem::getCaptureStatus() {
      // getCaptureStatus is a polling mechanism to officially stop a failed capture or check the end of encoding process
      if (captureInProgress_ && captureFailed_) {
        FBCAPTURE_STATUS status = captureFailureReason_;
        stopCapture();
        return status;
      } else if (captureInProgress_) {
        return FBCAPTURE_STATUS_SYSTEM_CAPTURE_IN_PROGRESS;
      }
      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS FBCaptureSystem::getScreenshotStatus() {
      if (screenshotFailed_) {
        FBCAPTURE_STATUS status = captureFailureReason_;
        screenshotInProgress_ = false;
        return status;
      } else if (screenshotInProgress_) {
        return FBCAPTURE_STATUS_SYSTEM_CAPTURE_IN_PROGRESS;
      }
      return FBCAPTURE_STATUS_OK;
    }


    FBCAPTURE_STATUS FBCaptureSystem::doPreview(void* texturePtr, bool onlyCamera) {
      if (!captureInProgress_ || captureInProgressType_ != FBCaptureType::kPreview) {
        DEBUG_ERROR("preview capture is not in progress. Call startPreviewCapture to start. ");
        return FBCAPTURE_STATUS_SYSTEM_CAPTURE_PREVIEW_NOT_IN_PROGRESS;
      }

      if (!onlyCamera && !captureTextureReceieved_) {
        DEBUG_ERROR("capture texture has not been received. Call captureTexture first. ");
        return FBCAPTURE_STATUS_SYSTEM_CAPTURE_TEXTURE_NOT_RECEIVED;
      }

      // Fill pEncodingTexture_ on system device
      doEncodeTextureRender(onlyCamera);
      if (captureFailed_) {
        return captureFailureReason_;
      }

      // Copy pEncodingTexture_ to pPreviewTexture_ on system device
      HANDLE sharedHandle;
      FBCAPTURE_STATUS result = doPreviewTextureRender(&sharedHandle);
      if (result != FBCAPTURE_STATUS_OK) {
        captureFailed(result);
        return result;
      }

      // Have the user graphics device copy pPreviewTexture_ to texturePtr
      result = pGraphicsDeviceCapture_->copyTexture(texturePtr, sharedHandle);
      if (result != FBCAPTURE_STATUS_OK) {
        captureFailed(result);
      }

      return result;
    }

    void FBCaptureSystem::stopCapture() {
      stopRequested_ = true;
    }

    void FBCaptureSystem::doEncoderStopRoutine() {
      if (captureInProgressType_ == FBCaptureType::kPreview) {
        return;
      }

      FBCAPTURE_STATUS status = pEncoder_->stopEncoding(true);
      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR_VAR("Final stop encoding failed", std::to_string(status));
        captureFailed(status);
      }
      if (status == FBCAPTURE_STATUS_OK && captureInProgressType_ == FBCaptureType::kVod) {
        status = pEncoder_->muxingData(pVodCaptureSettings_->projectionType_, pVodCaptureSettings_->stereoMode_, pVodCaptureSettings_->frameRate_, pVodCaptureSettings_->is360_);
        if (status != FBCAPTURE_STATUS_OK) {
          DEBUG_ERROR_VAR("Final MP4 muxing failed", std::to_string(status));
          captureFailed(status);
        }
      } else if (status == FBCAPTURE_STATUS_OK && captureInProgressType_ == FBCaptureType::kLive) {
        status = pEncoder_->muxingData(pLiveCaptureSettings_->projectionType_, pLiveCaptureSettings_->stereoMode_, pLiveCaptureSettings_->frameRate_, pLiveCaptureSettings_->is360_);
        if (status != FBCAPTURE_STATUS_OK) {
          DEBUG_ERROR_VAR("Final FLV muxing failed", std::to_string(status));
          captureFailed(status);
        } else {
          status = pEncoder_->startLiveStream(pLiveCaptureSettings_->streamUrl_.c_str());
          if (status != FBCAPTURE_STATUS_OK) {
            DEBUG_ERROR_VAR("Start livestream failed", std::to_string(status));
            captureFailed(status);
          }
        }
      }

      pEncoder_->stopLiveStream();
      pEncoder_->resetResources();			
    }

    void FBCaptureSystem::captureFailed(FBCAPTURE_STATUS reason) {
      std::lock_guard<std::mutex> lock(captureFailedMutex_);
      continueCapture_ = false;
      captureFailed_ = true;
      captureFailureReason_ = reason;
      if (pEncoder_) pEncoder_->releaseEncodeResources();
    }

    void FBCaptureSystem::screenshotFailed(FBCAPTURE_STATUS reason) {
      std::lock_guard<std::mutex> lock(screenshotFailedMutex_);
      screenshotFailed_ = true;
      captureFailureReason_ = reason;
      screenshotInProgress_ = false;
    }

    uint32_t FBCaptureSystem::getCaptureWidth(FBCaptureType type) {
			// Needs to be aligned to 16
      if (type == FBCaptureType::kLive) {
        return ((pLiveCaptureSettings_->width_ + kResolutionAlignment - 1) / kResolutionAlignment) * kResolutionAlignment;
      } else if (type == FBCaptureType::kVod) {
				return ((pVodCaptureSettings_->width_ + kResolutionAlignment - 1) / kResolutionAlignment) * kResolutionAlignment;
      } else if (type == FBCaptureType::kScreenShot) {
        return pScreenshotSettings_->width_;
      } else {
        return ((pPreviewCaptureSettings_->width_ + kResolutionAlignment - 1) / kResolutionAlignment) * kResolutionAlignment;
      }
    }

    uint32_t FBCaptureSystem::getCaptureHeight(FBCaptureType type) {
			// Needs to be aligned to 16
      if (type == FBCaptureType::kLive) {
        return ((pLiveCaptureSettings_->height_ + kResolutionAlignment - 1) / kResolutionAlignment) * kResolutionAlignment;
      } else if (type == FBCaptureType::kVod) {
        return ((pVodCaptureSettings_->height_ + kResolutionAlignment - 1) / kResolutionAlignment) * kResolutionAlignment;
      } else if (type == FBCaptureType::kScreenShot) {
        return pScreenshotSettings_->height_;
      } else {
        return ((pPreviewCaptureSettings_->height_ + kResolutionAlignment - 1) / kResolutionAlignment) * kResolutionAlignment;
      }
    }

    float FBCaptureSystem::getCaptureEncodeCycle(FBCaptureType type) {
      if (type == FBCaptureType::kLive) {
        return pLiveCaptureSettings_->encodeCycle_;
      } else if (type == FBCaptureType::kVod) {
        return pVodCaptureSettings_->encodeCycle_;
      } else {
        return pPreviewCaptureSettings_->encodeCycle_;
      }
    }

    bool FBCaptureSystem::getCaptureNeedsVerticalFlip(FBCaptureType type) {
      if (type == FBCaptureType::kLive) {
        return pLiveCaptureSettings_->verticalFlip_;
      } else if (type == FBCaptureType::kVod) {
        return pVodCaptureSettings_->verticalFlip_;
      } else if (type == FBCaptureType::kScreenShot) {
        return pScreenshotSettings_->verticalFlip_;
      } else {
        return pPreviewCaptureSettings_->verticalFlip_;
      }
    }

    bool FBCaptureSystem::getCaptureNeedsHorizontalFlip(FBCaptureType type) {
      if (type == FBCaptureType::kLive) {
        return pLiveCaptureSettings_->horizontalFlip_;
      } else if (type == FBCaptureType::kVod) {
        return pVodCaptureSettings_->horizontalFlip_;
      } else if (type == FBCaptureType::kScreenShot) {
        return pScreenshotSettings_->horizontalFlip_;
      } else {
        return pPreviewCaptureSettings_->horizontalFlip_;
      }
    }

    bool FBCaptureSystem::getCaptureIs360(FBCaptureType type) {
      if (type == FBCaptureType::kLive) {
        return pLiveCaptureSettings_->is360_;
      } else if (type == FBCaptureType::kVod) {
        return pVodCaptureSettings_->is360_;
      } else if (type == FBCaptureType::kScreenShot) {
        return pScreenshotSettings_->is360_;
      } else {
        return pPreviewCaptureSettings_->is360_;
      }
    }

    FBCAPTURE_STATUS FBCaptureSystem::saveScreenShot(void* texturePtr) {

      // Copy the user graphics device texturePtr to a separate shared texture on the user graphics device
      FBCAPTURE_STATUS status = pGraphicsDeviceScreenshot_->captureTextureToSharedHandle(texturePtr);
      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR_VAR("Failed on copying device texture pointer to shared texture", std::to_string(status));
        captureFailed(status);
        screenshotFailed(status);
        return status;
      }

      screenshotTextureReceieved_ = true;

      return FBCAPTURE_STATUS_OK;
    }

    bool FBCaptureSystem::activateCameraDevice(std::lock_guard<std::mutex> &lock) {
      HRESULT hr = pCameraDevices_->activateDevice(pCameraSettings_->cameraDeviceIndexChosen_,
                                                   &pCameraOverlay_->pCameraMediaSource_);
      if (FAILED(hr))
        return false;
      pCameraOverlay_->pCameraReader_.reset(
        new CameraReader(pCameraOverlay_->pCameraMediaSource_, *pCameraDeviceManager_));
      hr = pCameraOverlay_->pCameraReader_->initialize();
      return SUCCEEDED(hr);
    }

    bool FBCaptureSystem::deactivateCameraDevice(std::lock_guard<std::mutex> &lock) {
      if (isCameraReaderInitialized() && pCameraOverlay_->pCameraReader_->terminate()) {
        while (pCameraOverlay_->pCameraReader_ && !pCameraOverlay_->pCameraReader_->isFlushed()) {
          Sleep(1);
        }
      }
      pCameraOverlay_->pCameraReader_.reset(nullptr);
      HRESULT hr = pCameraDevices_->deactivateDevice(pCameraSettings_->cameraDeviceIndexChosen_,
                                                     pCameraOverlay_->pCameraMediaSource_);
      pCameraOverlay_.reset(nullptr);
      return SUCCEEDED(hr);
    }

    bool FBCaptureSystem::doesCameraReaderExist() {
      return pCameraOverlay_ && pCameraOverlay_->pCameraReader_;
    }

    bool FBCaptureSystem::isCameraReaderInitialized() {
      return doesCameraReaderExist() && pCameraOverlay_->pCameraReader_->isInitialized() && pCameraOverlaySettings_;
    }

    bool FBCaptureSystem::isCameraReaderTextureAvailable() {
      return isCameraReaderInitialized() && !pCameraOverlay_->pCameraReader_->isTerminated() && pCameraSettings_ && pCameraSettings_->enabledDuringCapture_
        && pCameraOverlay_->pCameraReader_->isCameraTextureFilled();
    }

    void FBCaptureSystem::doCameraOverlay(bool fullQuad) {
      // We don't currently support camera overlays on 360 captures
      if (getCaptureIs360(captureInProgressType_)) return;

      if (isCameraReaderTextureAvailable()) {
        std::lock_guard<std::mutex> lock(cameraModificationMutex_);
        if (isCameraReaderTextureAvailable()) {
          if (!pCameraOverlay_->pSharedCameraTexture_) {
            HRESULT hr = pDevice_->OpenSharedResource(
              pCameraOverlay_->pCameraReader_->getCameraTextureShareableHandle(),
              IID_PPV_ARGS(&pCameraOverlay_->pSharedCameraTexture_));
            if (FAILED(hr)) {
              DEBUG_ERROR_VAR("Failed to open camera shared resource in "
                              "encode. [Error code] ",
                              to_string(hr));
              captureFailed(FBCAPTURE_STATUS_SYSTEM_CAMERA_OVERLAY_FAILED);
              return;
            }
          }

          if (!pCameraOverlay_->pSharedCameraTextureKeyedMutex_) {
            HRESULT hr = pCameraOverlay_->pSharedCameraTexture_->QueryInterface(
              IID_PPV_ARGS(&pCameraOverlay_->pSharedCameraTextureKeyedMutex_));
            if (FAILED(hr)) {
              DEBUG_ERROR_VAR("Failed to open camera shared resource in "
                              "encode. [Error code] ",
                              to_string(hr));
              captureFailed(FBCAPTURE_STATUS_SYSTEM_CAMERA_OVERLAY_FAILED);
              return;
            }
          }

          HRESULT hr = pCameraOverlay_->pSharedCameraTextureKeyedMutex_->AcquireSync(0, INFINITE);
          if (FAILED(hr)) {
            DEBUG_ERROR_VAR("Failed to open camera shared resource in encode. "
                            "[Error code] ",
                            to_string(hr));
            captureFailed(FBCAPTURE_STATUS_SYSTEM_CAMERA_OVERLAY_FAILED);
            return;
          }

          if (!fullQuad) {
            pTextureFormatConversion_->setViewport(
              pCameraOverlaySettings_->viewPortTopLeftX_, pCameraOverlaySettings_->viewPortTopLeftY_,
              pCameraOverlay_->cameraTextureOverlayWidth_, pCameraOverlay_->cameraTextureOverlayHeight_);
          }

          pTextureFormatConversion_->renderFrame(
            pCameraOverlay_->pSharedCameraTexture_);

          if (!fullQuad) {
            const uint32_t width = getCaptureWidth(captureInProgressType_);
            const uint32_t height = getCaptureHeight(captureInProgressType_);

            pTextureFormatConversion_->setViewport(
              0, 0, width,
              height);
          }

          hr = pCameraOverlay_->pSharedCameraTextureKeyedMutex_->ReleaseSync(0);
          if (FAILED(hr)) {
            DEBUG_ERROR_VAR("Failed to release camera shared resource in "
                            "encode. [Error code] ",
                            to_string(hr));
            captureFailed(FBCAPTURE_STATUS_SYSTEM_CAMERA_OVERLAY_FAILED);
            return;
          }
        }
      }
    }

    void FBCaptureSystem::setAudioRawDataByClient(bool enabled) {
      if (pEncoder_) {
        pEncoder_->setClientPcmAudioInput(enabled);
      }
    }

    FBCAPTURE_STATUS FBCaptureSystem::sendAudioSample(float* audioRawData, uint32_t channel, uint32_t sampleRate, uint32_t bufferSize) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      if (pEncoder_ && pEncoder_->isClientPcmAudioInputEnabled() && continueCapture_) {
        std::lock_guard<std::mutex> audioLock(audioCaptureMutex_);

        status = pEncoder_->audioRawDataEncoding(audioRawData, channel, sampleRate, bufferSize);
        if (status != FBCAPTURE_STATUS_OK) {
          DEBUG_ERROR_VAR("Audio encoding failed", std::to_string(status));
          captureFailed(status);
          return status;
        }
      }

      return status;
    }

    FBCAPTURE_STATUS FBCaptureSystem::doAudioCapture() {
      std::lock_guard<std::mutex> micLock(micModificationMutex_);
      std::lock_guard<std::mutex> audioLock(audioCaptureMutex_);

      FBCAPTURE_STATUS status = pEncoder_->audioDeviceCaptureEncoding(vrDeviceRequested_, audioEnabledDuringCapture_,
                                                                      pMicSettings_ && pMicSettings_->enabledDuringCapture_,
                                                                      vrDevice_, pMicSettings_ ? pMicSettings_->micDeviceIdChosen_ : nullptr);

      if (status == FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED) {
        return status;
      } else if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR_VAR("Audio encoding failed", std::to_string(status));
        captureFailed(status);
        return status;
      }

      return status;
    }

    void FBCaptureSystem::muxThreadRun() {
      CoInitialize(nullptr);
      FBCAPTURE_STATUS status;

      while (continueCapture_ && captureInProgressType_ != FBCaptureType::kPreview) {
        if (flush_) {
          std::lock_guard<std::mutex> lock(flushMutex_);

          flush_ = false;
          if (captureInProgressType_ == FBCaptureType::kVod) {
            status = pEncoder_->muxingData(pVodCaptureSettings_->projectionType_, pVodCaptureSettings_->stereoMode_, pVodCaptureSettings_->frameRate_, pVodCaptureSettings_->is360_);
            if (status != FBCAPTURE_STATUS_OK) {
              DEBUG_ERROR_VAR("MP4 Muxing failed", std::to_string(status));
              captureFailed(status);
              return;
            }
          }

          if (captureInProgressType_ == FBCaptureType::kLive) {
            status = pEncoder_->muxingData(pLiveCaptureSettings_->projectionType_, pLiveCaptureSettings_->stereoMode_, pLiveCaptureSettings_->frameRate_, pLiveCaptureSettings_->is360_);
            if (status != FBCAPTURE_STATUS_OK) {
              DEBUG_ERROR_VAR("FLV Muxing failed", std::to_string(status));
              captureFailed(status);
              return;
            }

            status = pEncoder_->startLiveStream(pLiveCaptureSettings_->streamUrl_.c_str());
            if (status != FBCAPTURE_STATUS_OK) {
              DEBUG_ERROR_VAR("Start livestream failed", std::to_string(status));
              captureFailed(status);
              return;
            }
          }
        }
        Sleep(10);
      }
    }

    void FBCaptureSystem::audioThreadRun() {
      CoInitialize(nullptr);

      while (continueCapture_ && captureInProgressType_ != FBCaptureType::kPreview) {
        FBCAPTURE_STATUS status = doAudioCapture();
        if (status == FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED)
          break;
        Sleep(5);
      }
    }

    void FBCaptureSystem::stopRoutineThreadRun() {
      CoInitialize(nullptr);

      while (captureInProgress_) {
        if (stopRequested_) {
          std::lock_guard<std::mutex> lock(stopRoutineMutex_);
          stopRequested_ = false;

          continueCapture_ = false;
          if (encodeThread_ && encodeThread_->joinable()) {
            try {
              encodeThread_->join();
            } catch (std::exception& ex) {
              DEBUG_ERROR_VAR("Exception Caught in encodeThread_", ex.what());
            }
          }
          if (audioThread_ && audioThread_->joinable()) {
            try {
              audioThread_->join();
            } catch (std::exception& ex) {
              DEBUG_ERROR_VAR("Exception Caught in audioThread_", ex.what());
            }
          }
          if (muxThread_ && muxThread_->joinable()) {
            try {
              muxThread_->join();
            } catch (std::exception& ex) {
              DEBUG_ERROR_VAR("Exception Caught in muxThread_", ex.what());
            }
          }
          if (doesCameraReaderExist()) {
            std::lock_guard<std::mutex> lock(cameraModificationMutex_);
            deactivateCameraDevice(lock);
          }
          if (cameraThread_ && cameraThread_->joinable()) {
            try {
              cameraThread_->join();
            } catch (std::exception& ex) {
              DEBUG_ERROR_VAR("Exception Caught in cameraThread_", ex.what());
            }
          }

          delete muxThread_;
          delete audioThread_;
          delete encodeThread_;
          delete cameraThread_;

          muxThread_ = nullptr;
          audioThread_ = nullptr;
          encodeThread_ = nullptr;
          cameraThread_ = nullptr;

          doEncoderStopRoutine();

          pPreviewCaptureSettings_.reset(nullptr);
          pTextureFormatConversion_.reset(nullptr);
          pEncodingTexture_ = nullptr;
          pPreviewTexture_ = nullptr;
          encoding_ = false;
          flush_ = false;
          captureTextureReceieved_ = false;
          break;
        }
        Sleep(10);
      }

      // cleanup thread at last
      if (stopRoutineThread_ && stopRoutineThread_->joinable()) {
        try {
          stopRoutineThread_->detach();
        } catch (std::exception& ex) {
          DEBUG_ERROR_VAR("Exception Caught in stopRoutineThread_", ex.what());
        }
        delete stopRoutineThread_;
        stopRoutineThread_ = nullptr;
        captureInProgress_ = false;
      }
    }

    void FBCaptureSystem::screenshotThreadRun() {
      CoInitialize(nullptr);
      FBCAPTURE_STATUS status;

      while (screenshotInProgress_) {
        if (screenshotTextureReceieved_) {
          screenshotTextureReceieved_ = false;

          status = pGraphicsDeviceScreenshot_->renderCapturedTexture(pDevice_, pScreenShotTextureFormatConversion_.get());
          if (status != FBCAPTURE_STATUS_OK) {
            screenshotFailed(status);
            DEBUG_ERROR_VAR("Failed on rendering captured texture", std::to_string(status));
            break;
          }

          status = pEncoder_->saveScreenShot(pScreenshotTexture_, pScreenshotSettings_->fullSavePath_.c_str(), pScreenshotSettings_->is360_);
          if (status != FBCAPTURE_STATUS_OK) {
            screenshotFailed(status);
            DEBUG_ERROR_VAR("Failed on screenshot", std::to_string(status));
            break;
          }

          pScreenshotSettings_.reset(nullptr);
          pScreenShotTextureFormatConversion_.reset(nullptr);
          pGraphicsDeviceScreenshot_.reset(nullptr);
          pScreenshotTexture_ = nullptr;

          break;
        }
        Sleep(10);
      }

      terminateScreenshotThread_ = true;
    }

    void FBCaptureSystem::screenshotThreadManagerRun() {
      CoInitialize(nullptr);
      FBCAPTURE_STATUS status;

      while (screenshotInProgress_) {
        if (terminateScreenshotThread_ && screenshotThread_ && screenshotThread_->joinable()) {
          try {
            screenshotThread_->join();
          } catch (std::exception& ex) {
            DEBUG_ERROR_VAR("Exception Caught in screenshotThread_", ex.what());
          }
          delete screenshotThread_;
          screenshotThread_ = nullptr;

          break;
        }
        Sleep(10);
      }

      // cleanup thread at last
      if (screenshotThreadManager_ && screenshotThreadManager_->joinable()) {
        try {
          screenshotThreadManager_->detach();
        } catch (std::exception& ex) {
          DEBUG_ERROR_VAR("Exception Caught in screenshotThreadManager_", ex.what());
        }
        delete screenshotThreadManager_;
        screenshotThreadManager_ = nullptr;

        screenshotInProgress_ = false;
        terminateScreenshotThread_ = false;
        screenshotFailed_ = false;				
      }
    }


    void FBCaptureSystem::doEncodeTextureRender(bool onlyCamera) {
      if (!onlyCamera) {
        FBCAPTURE_STATUS status;

        status = pGraphicsDeviceCapture_->renderCapturedTexture(pDevice_, pTextureFormatConversion_.get());
        if (status != FBCAPTURE_STATUS_OK) {
          captureFailed(status);
          return;
        }
      }
      // Overlay camera system graphics device onto system graphics device encoding texture
      doCameraOverlay(onlyCamera);
    }

    FBCAPTURE_STATUS FBCaptureSystem::doPreviewTextureRender(HANDLE* sharedHandle) {
      ScopedCOMPtr<IDXGIResource> textureResource;
      HRESULT hr = pPreviewTexture_->QueryInterface(IID_PPV_ARGS(&textureResource));
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to get IDXGIResource from Texture2D for preview. [Error code] ",
          hr);
        return FBCAPTURE_STATUS_SYSTEM_CAPTURE_PREVIEW_FAILED;
      }

      hr = textureResource->GetSharedHandle(sharedHandle);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to get shared handle from Texture2D for preview. [Error code] ",
                            hr);
        return FBCAPTURE_STATUS_SYSTEM_CAPTURE_PREVIEW_FAILED;
      }

      ScopedCOMPtr<IDXGIKeyedMutex> sharedTextureKeyedMutex;
      hr = pPreviewTexture_->QueryInterface(
        IID_PPV_ARGS(&sharedTextureKeyedMutex));
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to open GraphicsDeviceCapture shared keyed mutex. [Error code] ",
                        to_string(hr));
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_MUTEX_ACQUIRE_FAILED;
      }

      hr = sharedTextureKeyedMutex->AcquireSync(0, INFINITE);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to acquire sync on GraphicsDeviceCapture shared resource. [Error code] ",
                        to_string(hr));
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_ACQUIRE_ACQUIRE_SYNC_FAILED;
      }

      pPreviewTextureFormatConversion_->renderFrame(pEncodingTexture_);

      hr = sharedTextureKeyedMutex->ReleaseSync(0);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to release sync on GraphicsDeviceCapture shared resource. [Error code] ",
                        to_string(hr));
        return FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_ACQUIRE_RELASE_SYNC_FAILED;
      }
      return FBCAPTURE_STATUS_OK;
    }

    void FBCaptureSystem::encodeThreadRun() {
      CoInitialize(nullptr);
      FBCAPTURE_STATUS status;

      float encodeTime = 0;
      auto prevEncodeTime = std::chrono::steady_clock::now();
      auto prevFlushTime = prevEncodeTime;

      while (continueCapture_ && captureInProgressType_ != FBCaptureType::kPreview) {

        auto now = std::chrono::steady_clock::now();
        auto encodeDiffSeconds =
          std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                                prevEncodeTime)
          .count() /
          1000.0f; // millis per second
        const float encodeCycle = getCaptureEncodeCycle(captureInProgressType_);

        if (captureTextureReceieved_ && (encodeTime + encodeDiffSeconds) >= encodeCycle) {
          encodeTime += (encodeDiffSeconds - encodeCycle);
          prevEncodeTime = now;

          doEncodeTextureRender(false);
          if (!continueCapture_) {
            return;
          }

          const bool isLive = captureInProgressType_ == FBCaptureType::kLive;
          const int videoBitRate = isLive ? pLiveCaptureSettings_->videoBitRate_ : pVodCaptureSettings_->videoBitRate_;
          const int videoFrameRate = isLive ? pLiveCaptureSettings_->frameRate_ : pVodCaptureSettings_->frameRate_;
          const TCHAR* fullSavePath = isLive ? L"" : pVodCaptureSettings_->fullSavePath_.c_str();
          status = pEncoder_->startEncoding(pEncodingTexture_, fullSavePath, isLive, videoBitRate, videoFrameRate, false);
          if (status != FBCAPTURE_STATUS_OK) {
            DEBUG_ERROR_VAR("Start encoding failed", std::to_string(status));
            captureFailed(status);
            return;
          }

          encoding_ = true;

          if (isLive) {
            auto flushDiffSeconds =
              std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                                    prevFlushTime)
              .count() /
              1000.0f; // millis per second

            if (flushDiffSeconds >= pLiveCaptureSettings_->flushCycle_) {
              std::lock_guard<std::mutex> lock(flushMutex_);
              std::lock_guard<std::mutex> audioLock(audioCaptureMutex_);
              pLiveCaptureSettings_->flushCycle_ = pLiveCaptureSettings_->flushCycleAfter_;
              prevFlushTime = now;
              encoding_ = false;
              status = pEncoder_->stopEncoding(false);
              if (status != FBCAPTURE_STATUS_OK) {
                DEBUG_ERROR_VAR("Stop encoding failed", std::to_string(status));
                captureFailed(status);
                return;
              }
              flush_ = true;
            }
          }
        }

        if (continueCapture_ && (encodeTime + encodeDiffSeconds) < encodeCycle) {
          DWORD targetSleepMS = (encodeCycle - (encodeTime + encodeDiffSeconds)) * 1000; // millis per second
          if (targetSleepMS > 0) targetSleepMS -= 1; // on the safe side
          if (targetSleepMS > 0) Sleep(targetSleepMS);
        }
      }
    }

    void FBCaptureSystem::cameraThreadRun() {
      CoInitialize(nullptr);
      bool cameraReaderAvailable = pCameraOverlay_ && pCameraOverlay_->pCameraReader_;

      if (cameraReaderAvailable && !pCameraOverlaySettings_) {
        DEBUG_ERROR("No camera overlay settings set. Not performing camera capture. ");
        return;
      }

      if (cameraReaderAvailable) {
        if (!pCameraOverlay_->pCameraReader_->beginReadLoop()) {
          DEBUG_ERROR("Camera failed to start");
          return;
        }
      }

      while (continueCapture_) {
        // If the camera is enabled, and is looping, and it fails, then deactivate
        if (pCameraOverlay_ && pCameraOverlay_->pCameraReader_ && pCameraOverlay_->pCameraReader_->isInReadLoop() &&
            pCameraOverlay_->pCameraReader_->isTerminated()) {
          std::lock_guard<std::mutex> lock(cameraModificationMutex_);
          deactivateCameraDevice(lock);
        }
        Sleep(10);
      }
    }

    GRAPHICS_CARD FBCaptureSystem::checkGPUManufacturer() {
      if (pEncoder_ == nullptr) {
        DEBUG_LOG("You need to call fbc_getCaptureCapability function first to get GPU device info");
        return GRAPHICS_CARD::UNSUPPORTED_DEVICE;
      }

      return pEncoder_->checkGPUManufacturer();
    }


    void FBCaptureSystem::shutdown() {
      stopCapture();
      unsetMicDevice();

      vrDevice_ = VRDeviceType::NONE; // TODO remove/replace with AudioRenderSettings
      pEncoder_.reset(nullptr);
      pCameraOverlay_.reset(nullptr);
      pCameraDeviceManager_.reset(nullptr);
      pCameraDevices_.reset(nullptr);
      pMicDevices_.reset(nullptr);
      pMicSettings_.reset(nullptr);
      pCameraSettings_.reset(nullptr);
      pCameraOverlaySettings_.reset(nullptr);
      pPreviewCaptureSettings_.reset(nullptr);
      pVodCaptureSettings_.reset(nullptr);
      pLiveCaptureSettings_.reset(nullptr);
      pGraphicsDeviceCapture_.reset(nullptr);
      pScreenshotSettings_.reset(nullptr);
      pScreenShotTextureFormatConversion_.reset(nullptr);
      pGraphicsDeviceScreenshot_.reset(nullptr);
      pScreenshotTexture_ = nullptr;			

      initialized_ = false;
      MFShutdown();
    }
  }
}

std::unique_ptr<FBCapture::Common::FBCaptureSystem>
fbCaptureSystem(new FBCapture::Common::FBCaptureSystem);


#define DllExport __declspec (dllexport)

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_reset() {
  fbCaptureSystem.reset(new FBCapture::Common::FBCaptureSystem);
  return FBCAPTURE_STATUS_OK;
}

// If plugged into Unity, this function will be called automatically by Unity when the
// graphics device is created, destroyed, before it's being reset (i.e. resolution changed), after it's been reset
extern "C" DllExport void UnitySetGraphicsDevice(void* device, int deviceType, int eventType) {
  if (eventType == UnityGfxDeviceEventType::kUnityGfxDeviceEventShutdown) {
    fbc_reset();
    return;
  }
  if (eventType != UnityGfxDeviceEventType::kUnityGfxDeviceEventInitialize) {
    DEBUG_ERROR_VAR("Failed on UnitySetGraphicsDevice. Unsupported eventType. [Event type] ", to_string(eventType));
    return;
  }
  if (deviceType == kUnityGfxRendererD3D11) {
    fbCaptureSystem->setGraphicsDeviceD3D11((ID3D11Device*)device);
  } else {
    DEBUG_ERROR_VAR("Failed on UnitySetGraphicsDevice. Unsupported deviceType. [Device type] ", to_string(deviceType));
  }
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setGraphicsDeviceD3D11(void* device) {
  return fbCaptureSystem->setGraphicsDeviceD3D11((ID3D11Device*)device);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setLiveCaptureSettings(int width, int height, int frameRate, int bitRate, float flushCycleStart, float flushCycleAfter, const TCHAR* streamUrl, bool is360, bool verticalFlip, bool horizontalFlip, PROJECTIONTYPE projectionType, STEREO_MODE stereoMode) {
  return fbCaptureSystem->setLiveCaptureSettings(width, height, frameRate, bitRate, flushCycleStart, flushCycleAfter, streamUrl, is360, verticalFlip, horizontalFlip, projectionType, stereoMode);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setVodCaptureSettings(int width, int height, int frameRate, int bitRate, const TCHAR* fullSavePath, bool is360, bool verticalFlip, bool horizontalFlip, PROJECTIONTYPE projectionType, STEREO_MODE stereoMode) {
  return fbCaptureSystem->setVodCaptureSettings(width, height, frameRate, bitRate, fullSavePath, is360, verticalFlip, horizontalFlip, projectionType, stereoMode);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setPreviewCaptureSettings(int width, int height, int frameRate, bool is360, bool verticalFlip, bool horizontalFlip) {
  return fbCaptureSystem->setPreviewCaptureSettings(width, height, frameRate, is360, verticalFlip, horizontalFlip);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setScreenshotSettings(int width, int height, const TCHAR* fullsavePath, bool is360, bool verticalFlip, bool horizontalFlip) {
  return fbCaptureSystem->setScreenshotSettings(width, height, fullsavePath, is360, verticalFlip, horizontalFlip);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setCameraOverlaySettings(float widthPercentage, uint32_t viewPortTopLeftX, uint32_t viewPortTopLeftY) {
  return fbCaptureSystem->setCameraOverlaySettings(widthPercentage, viewPortTopLeftX, viewPortTopLeftY);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_enumerateMicDevices() {
  return fbCaptureSystem->enumerateMicDevices();
}

extern "C" DllExport size_t fbc_getMicDevicesCount() {
  return fbCaptureSystem->getMicDevicesCount();
}

extern "C" DllExport const char * fbc_getMicDeviceName(uint32_t index) {
  return fbCaptureSystem->getMicDeviceName(index);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setMicDevice(uint32_t index) {
  return fbCaptureSystem->setMicDevice(index);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_unsetMicDevice() {
  return fbCaptureSystem->unsetMicDevice();
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setMicEnabledDuringCapture(bool enabled) {
  return fbCaptureSystem->setMicEnabledDuringCapture(enabled);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setAudioEnabledDuringCapture(bool enabled) {
  return fbCaptureSystem->setAudioEnabledDuringCapture(enabled);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_enumerateCameraDevices() {
  return fbCaptureSystem->enumerateCameraDevices();
}

extern "C" DllExport size_t fbc_getCameraDevicesCount() {
  return fbCaptureSystem->getCameraDevicesCount();
}

extern "C" DllExport const char * fbc_getCameraDeviceName(uint32_t index) {
  return fbCaptureSystem->getCameraDeviceName(index);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setCameraDevice(uint32_t index) {
  return fbCaptureSystem->setCameraDevice(index);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_unsetCameraDevice() {
  return fbCaptureSystem->unsetCameraDevice();
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setCameraEnabledDuringCapture(bool enabled) {
  return fbCaptureSystem->setCameraEnabledDuringCapture(enabled);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_setMicAndAudioRenderDeviceByVRDeviceType(VRDeviceType vrDevice) {
  return fbCaptureSystem->setMicAndAudioRenderDeviceByVRDeviceType(vrDevice);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_startLiveCapture() {
  return fbCaptureSystem->startCapture(FBCaptureType::kLive);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_startVodCapture() {
  return fbCaptureSystem->startCapture(FBCaptureType::kVod);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_startPreviewCapture() {
  return fbCaptureSystem->startCapture(FBCaptureType::kPreview);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_startScreenshot() {
  return fbCaptureSystem->startScreenshot(FBCaptureType::kScreenShot);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_captureTexture(void* texturePtr) {
  return fbCaptureSystem->captureTexture(texturePtr);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_previewCapture(void* texturePtr) {
  return fbCaptureSystem->previewCapture(texturePtr);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_previewCamera(void* texturePtr) {
  return fbCaptureSystem->previewCamera(texturePtr);
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_getCaptureStatus() {
  return fbCaptureSystem->getCaptureStatus();
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_getScreenshotStatus() {
  return fbCaptureSystem->getScreenshotStatus();
}

extern "C" DllExport void fbc_stopCapture() {
  fbCaptureSystem->stopCapture();
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_getCaptureCapability() {
  return fbCaptureSystem->getCaptureCapability();
}

extern "C" DllExport FBCapture::Common::FBCAPTURE_STATUS fbc_saveScreenShot(void* texturePtr) {
  return fbCaptureSystem->saveScreenShot(texturePtr);
}

extern "C" DllExport FBCapture::Common::GRAPHICS_CARD fbc_checkGPUManufacturer() {
  return fbCaptureSystem->checkGPUManufacturer();
}

extern "C" DllExport void fbc_setAudioRawDataByClient(bool enabled) {
  fbCaptureSystem->setAudioRawDataByClient(enabled);
}

extern "C" DllExport void fbc_sendAudioSample(void* audioRawData, uint32_t channel, uint32_t sampleRate, uint32_t bufferSize) {
  fbCaptureSystem->sendAudioSample(reinterpret_cast<float*>(audioRawData), channel, sampleRate, bufferSize);
}
