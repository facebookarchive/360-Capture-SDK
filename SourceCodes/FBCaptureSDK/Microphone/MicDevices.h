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
#include <mmdeviceapi.h>
#include <stdint.h>
#include <string>
#include "Common/ScopedCOMPtr.h"

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
