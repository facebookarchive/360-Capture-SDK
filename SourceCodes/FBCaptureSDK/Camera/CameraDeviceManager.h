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
#include "Common/Common.h"
#include "Common/ScopedCOMPtr.h"
#include "Common/Log.h"
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

using namespace FBCapture::Common;

namespace FBCapture {
  namespace Camera {

    class CameraDeviceManager {
    public:
      CameraDeviceManager() {}
      ~CameraDeviceManager() { shutdown(); }
      bool isInitialized() const { return initialized; }
      HRESULT initialize();
      void shutdown();
      ID3D11Device *getDevice() { return pDevice; }
      ID3D11DeviceContext *getDeviceContext() { return pContext; }
      HRESULT getSourceReaderAttributes(IMFAttributes **ppMFAttributes);

    private:
      bool initialized = {};
      ScopedCOMPtr<ID3D11Device> pDevice = {};
      ScopedCOMPtr<ID3D11DeviceContext> pContext = {};
    };
  }
}
