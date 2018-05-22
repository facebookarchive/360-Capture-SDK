#pragma once

#include "Common.h"
#include "ScopedCOMPtr.h"
#include <mfidl.h>

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