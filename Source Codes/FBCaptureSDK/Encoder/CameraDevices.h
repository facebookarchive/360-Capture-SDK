#pragma once

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <sal.h>
#include <stdint.h>
#include <string>

namespace FBCapture {
  namespace Camera {

    class CameraDevices {
    public:
      CameraDevices() {}
      ~CameraDevices() { clear(); }

      uint32_t count() const { return deviceCount; }
      void clear();
      HRESULT enumerateDevices();
      HRESULT activateDevice(uint32_t index, _Outptr_ IMFMediaSource **pSource);
      HRESULT deactivateDevice(uint32_t index, _Inout_ IMFMediaSource *pSource);
      HRESULT getDeviceName(uint32_t index, std::string &name);

    private:
      uint32_t deviceCount = {};
      IMFActivate **ppDevices = {};
    };

  }
}