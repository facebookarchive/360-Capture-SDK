#pragma once

#include "CameraDeviceManager.h"
#include "Common.h"
#include "ScopedCOMPtr.h"
#include "TextureRender.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <mfapi.h>
#include <mferror.h>
#include <mfreadwrite.h>
#include <shlwapi.h>
#include <string>

using namespace FBCapture::Common;
using namespace FBCapture::Render;

namespace FBCapture {
  namespace Camera {

    class CameraReader : public IMFSourceReaderCallback {
    public:
      CameraReader(IMFMediaSource *pCameraMediaSource,
        CameraDeviceManager &cameraDeviceManager);
      ~CameraReader() = default;

      bool isInitialized() const { return initialized; }
      bool isTerminated() const { return terminated; }
      bool isInReadLoop() const { return readLoop; }
      bool isFlushed() const { return flushed; }
      bool isCameraTextureFilled() const { return filled; }

      LONGLONG getLastSampleTimestamp() const { return llLastSampleTimestamp; }

      ID3D11Texture2D *getShareableCameraTexture() {
        return pCameraTextureShareable;
      };
      HANDLE getCameraTextureShareableHandle() const {
        return cameraTextureShareableHandle;
      }

      uint32_t getCameraTextureWidth() const { return cameraTextureWidth; }
      uint32_t getCameraTextureHeight() const { return cameraTextureHeight; }

      HRESULT initialize();
      bool terminate();
      bool beginReadLoop();

      // IMFSourceReaderCallback
#pragma warning(push)
#pragma warning(disable : 4838) // conversion from 'DWORD' to 'int' requires a
                                // narrowing conversion
      STDMETHODIMP QueryInterface(REFIID iid, void **ppv) {
        static const QITAB qit[] = {
            QITABENT(CameraReader, IMFSourceReaderCallback),
            {0},
        };
        return QISearch(this, qit, iid, ppv);
      }
#pragma warning(pop)
      STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&refCount); }
      STDMETHODIMP_(ULONG) Release() {
        ULONG uCount = InterlockedDecrement(&refCount);
        // We own this object and will not treat it as a Com one, so we don't have
        // to worry about deleting on ref count 0
        return uCount;
      }
      STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,
        DWORD dwStreamFlags, LONGLONG llTimestamp,
        IMFSample *pSample) override;
      STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *) override;
      STDMETHODIMP OnFlush(DWORD) override {
        flushed = true;
        return S_OK;
      }

    private:
      HRESULT markCameraReaderTerminatedByHrStatus(const std::string &log,
        HRESULT hr);

      bool initialized = {};
      bool flushed = {};
      bool readLoop = {};
      bool terminated = {};
      bool filled = {};

      ScopedCOMPtr<ID3D11Texture2D> pCameraTextureTarget = {};
      ScopedCOMPtr<ID3D11Texture2D> pCameraTextureShareable = {};

      ScopedCOMPtr<IDXGIKeyedMutex> pCameraTextureTargetKeyedMutex = {};

      ScopedCOMPtr<IDXGIResource> pCameraTextureShareableResource = {};

      HANDLE cameraTextureShareableHandle = {};

      ScopedCOMPtr<IMFSourceReader> pReader = {};

      ScopedCOMPtr<IMFMediaSource> pCameraMediaSource = {};

      CameraDeviceManager &cameraDeviceManager;

      std::chrono::time_point<std::chrono::steady_clock> lastSampleStartTime = {};

      LONGLONG llLastSampleTimestamp = {};

      DWORD streamIndex = {};

      uint32_t cameraTextureWidth = {};
      uint32_t cameraTextureHeight = {};
      LONG cameraTextureStride = {};

      // IMFSourceReaderCallback
      long refCount;
      CRITICAL_SECTION critsec;
    };
  }
}