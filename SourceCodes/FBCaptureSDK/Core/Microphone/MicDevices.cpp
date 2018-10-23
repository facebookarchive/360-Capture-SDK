/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "MicDevices.h"
#include "Core/Common/Log.h"
#include "Core/Common/ScopedCOMPtr.h"
#include <atlstr.h>
#include <functiondiscoverykeys_devpkey.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <string>

namespace FBCapture {
  namespace Microphone {

    void MicDevices::clear() {
      deviceCount = 0;
      pDevices = NULL;
      pDeviceEnumerator = NULL;
    }

    HRESULT MicDevices::enumerateDevices() {
      this->clear();

      HRESULT hr = S_OK;

      hr = CoInitialize(nullptr);

      if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
          __uuidof(IMMDeviceEnumerator),
          (void **)&pDeviceEnumerator);
      }

      if (SUCCEEDED(hr)) {
        hr = pDeviceEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE,
          &pDevices);
      }

      if (SUCCEEDED(hr)) {
        UINT count;
        hr = pDevices->GetCount(&count);
        if (SUCCEEDED(hr)) {
          deviceCount = count;
        }
      }

      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to enumerate mic devices. [Error code] ", hr);
      }
      return hr;
    }

    void MicDevices::freeDeviceId(LPWSTR pstrId) { CoTaskMemFree(pstrId); }

    HRESULT MicDevices::getDeviceId(uint32_t index, _Outptr_ LPWSTR *ppstrId) {
      HRESULT hr = S_OK;
      if (index >= count()) {
        hr = E_INVALIDARG;
      }

      ScopedCOMPtr<IMMDevice> pDevice;
      if (SUCCEEDED(hr)) {
        hr = pDevices->Item(index, &pDevice);
      }

      if (SUCCEEDED(hr)) {
        hr = pDevice->GetId(ppstrId);
      }

      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to get mic device id. [Error code] ", hr);
      }
      return hr;
    }

    HRESULT MicDevices::getDeviceName(uint32_t index, std::string &name) {
      HRESULT hr = S_OK;
      if (index >= count()) {
        hr = E_INVALIDARG;
      }

      ScopedCOMPtr<IMMDevice> pDevice;
      if (SUCCEEDED(hr)) {
        hr = pDevices->Item(index, &pDevice);
      }

      ScopedCOMPtr<IPropertyStore> pProps = NULL;
      if (SUCCEEDED(hr)) {
        hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
      }

      PROPVARIANT varName;
      PropVariantInit(&varName);
      if (SUCCEEDED(hr)) {
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
      }

      if (SUCCEEDED(hr)) {
        LPWSTR pstrName = varName.pwszVal;
        name = CW2A(pstrName);
      }

      PropVariantClear(&varName);

      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to get mic device name. [Error code] ", hr);
      }
      return hr;
    }

  }
}
