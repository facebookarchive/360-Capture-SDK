#include "CameraDeviceManager.h"
#include "Log.h"
#include <mfapi.h>
#include <mfreadwrite.h>

namespace FBCapture {
  namespace Camera {

    HRESULT CameraDeviceManager::initialize() {
      HRESULT hr = S_OK;
      if (isInitialized()) {
        return hr;
      }

      hr = MFStartup(MF_VERSION);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR(
          "Failed to MFStartup in camera device manager. [Error code] ", to_string(hr));
        return hr;
      }

      D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };

      DWORD createDeviceFlags = 0;
#ifdef DEBUG
      createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
      hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, levels, ARRAYSIZE(levels),
        D3D11_SDK_VERSION, &pDevice, nullptr, &pContext);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to D3D11CreateDevice in camera device manager. [Error code] ",
          hr);
        return hr;
      }

      // Yes, this is D10 Multithread versus D11. It's compatible with D11. Using this as D11Multithread requires an extra Dll:
      // D3d11_4.dll
      ScopedCOMPtr<ID3D10Multithread> pMultithread;
      hr = pDevice->QueryInterface(__uuidof(ID3D10Multithread),
        (void **)&pMultithread);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to Query ID3D10Multithread Interface in camera "
          "device manager. [Error code] ",
          hr);
        return hr;
      }

      hr = pMultithread->SetMultithreadProtected(TRUE);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to SetMultithreadProtected in camera device "
          "manager. [Error code] ",
          hr);
        return hr;
      }

      initialized = true;
      return hr;
    }

    HRESULT
      CameraDeviceManager::getSourceReaderAttributes(IMFAttributes **ppMFAttributes) {
      HRESULT hr = MFCreateAttributes(ppMFAttributes, 4);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to MFCreateAttributes in camera device manager. [Error code] ",
          hr);
        return hr;
      }

      hr = (*ppMFAttributes)
        ->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to set MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS "
          "in camera device manager. [Error code] ",
          hr);
        return hr;
      }

      hr = (*ppMFAttributes)
        ->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to set MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING in "
          "camera device manager. [Error code] ",
          hr);
        return hr;
      }

      return S_OK;
    }

    void CameraDeviceManager::shutdown() {
      initialized = false;
      pDevice = NULL;
      pContext = NULL;
      MFShutdown();
    }

  }
}