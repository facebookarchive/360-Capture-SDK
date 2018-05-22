#pragma once

#include "ScopedCOMPtr.h"
#include <mmdeviceapi.h>
#include <stdint.h>
#include <string>

using namespace FBCapture::Common;

namespace FBCapture {
  namespace Microphone {

    class MicDevices {
    public:
      MicDevices() {}
      ~MicDevices() { clear(); }
      uint32_t count() const { return deviceCount; }
      void clear();
      HRESULT enumerateDevices();
      void freeDeviceId(LPWSTR pstrId);
      HRESULT getDeviceId(uint32_t index, _Outptr_ LPWSTR *ppstrId);
      HRESULT getDeviceName(uint32_t index, std::string &name);

    private:
      uint32_t deviceCount = {};
      ScopedCOMPtr<IMMDeviceCollection> pDevices = {};
      ScopedCOMPtr<IMMDeviceEnumerator> pDeviceEnumerator = {};
    };

  }
}