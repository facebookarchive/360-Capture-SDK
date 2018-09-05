#include "CameraDevices.h"
#include "Log.h"
#include "ScopedCOMPtr.h"
#include <atlstr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <stdio.h>

using namespace FBCapture::Common;

namespace FBCapture {
  namespace Camera {

    void CameraDevices::clear() {
      for (uint32_t i = 0; i < deviceCount; i++) {
        if (ppDevices[i]) {
          ppDevices[i]->Release();
        }
      }
      CoTaskMemFree(ppDevices);
      ppDevices = NULL;

      deviceCount = 0;
    }

    HRESULT CameraDevices::enumerateDevices() {
      this->clear();

      HRESULT hr = S_OK;
      ScopedCOMPtr<IMFAttributes> pAttributes;

      hr = MFCreateAttributes(&pAttributes, 1);

      if (SUCCEEDED(hr)) {
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
          MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
      }

      if (SUCCEEDED(hr)) {
        hr = MFEnumDeviceSources(pAttributes, &ppDevices, &deviceCount);
      }

      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to enumerate camera devices. [Error code] ",
          hr);
      }
      return hr;
    }

    HRESULT CameraDevices::activateDevice(uint32_t index,
      _Outptr_ IMFMediaSource **ppSource) {
      HRESULT hr = S_OK;
      if (index >= count()) {
        hr = E_INVALIDARG;
      }

      if (SUCCEEDED(hr)) {
        hr = ppDevices[index]->ActivateObject(IID_PPV_ARGS(ppSource));
      }

      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to activate camera device. [Error code] ", hr);
      }
      return hr;
    }

    HRESULT CameraDevices::deactivateDevice(uint32_t index,
      _Inout_ IMFMediaSource *pSource) {
      HRESULT hr = S_OK;
      if (index >= count()) {
        hr = E_INVALIDARG;
      }

      if (SUCCEEDED(hr)) {
        hr = ppDevices[index]->ShutdownObject();
      }

      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to deactivate camera device. [Error code] ",
          hr);
      }
      return hr;
    }

    HRESULT CameraDevices::getDeviceName(uint32_t index, std::string &name) {
      HRESULT hr = S_OK;
      if (index >= count()) {
        hr = E_INVALIDARG;
      }

      LPWSTR pwszValue = NULL;
      UINT32 cchLength = 0;

      if (SUCCEEDED(hr)) {
        hr = ppDevices[index]->GetAllocatedString(
          MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pwszValue, &cchLength);
      }

      if (SUCCEEDED(hr)) {
        name = CW2A(pwszValue);
      }

      // Required by GetAllocatedString. We've already copied to our std::string&
      // name
      CoTaskMemFree(pwszValue);

      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to get camera device name. [Error code] ", hr);
      }
      return hr;
    }
  }
}
