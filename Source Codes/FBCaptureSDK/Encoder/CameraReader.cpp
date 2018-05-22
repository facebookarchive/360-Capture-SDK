#include "CameraReader.h"
#include "Log.h"

namespace FBCapture {
  namespace Camera {

    // For now hardcode these constants but make some tunable later
    const int MAX_CAMERA_FRAME_RATE = 60;

    CameraReader::CameraReader(IMFMediaSource *pSource,
      CameraDeviceManager &manager)
      : pCameraMediaSource(pSource), cameraDeviceManager(manager), refCount(1) {
      pCameraMediaSource->AddRef();
      InitializeCriticalSection(&critsec);
    }

    HRESULT CameraReader::initialize() {
      HRESULT hr = S_OK;
      if (isInitialized()) {
        return hr;
      }

      ScopedCOMPtr<IMFAttributes> pAttributes;
      hr = cameraDeviceManager.getSourceReaderAttributes(&pAttributes);
      if (FAILED(hr)) {
        return hr;
      }

      hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK,
        static_cast<IMFSourceReaderCallback *>(this));
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to set MF_SOURCE_READER_ASYNC_CALLBACK in "
          "camera reader. [Error code] ",
          hr);
        return hr;
      }

      hr = MFCreateSourceReaderFromMediaSource(pCameraMediaSource, pAttributes,
        &pReader);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR("Failed to MFCreateSourceReaderFromMediaSource in "
          "camera reader. [Error code] ",
          hr);
        return hr;
      }

      // Find target media type
      streamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
      DWORD bestDwMediaTypeIndex = 0;
      bool found = false;

      // Find target media type
      ScopedCOMPtr<IMFMediaType> pNativeType;
      DWORD dwMediaTypeIndex = 0;
      UINT32 maxFrameRate = 0;
      UINT32 frameRate = 0;
      UINT32 denominator = 0;
      DWORD32 maxWidth = 0;
      DWORD32 width = 0;
      DWORD32 height = 0;
      while (SUCCEEDED(hr)) {
        pNativeType = NULL;
        hr = pReader->GetNativeMediaType(streamIndex, dwMediaTypeIndex++,
          &pNativeType);
        if (hr == MF_E_NO_MORE_TYPES) {
          hr = S_OK;
          break;
        }
        if (FAILED(hr))
          break;

        GUID majorType;
        hr = pNativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
        if (FAILED(hr))
          break;
        GUID subtype;
        hr = pNativeType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (FAILED(hr))
          break;

        // Supported formats for conversion to RGB are MFVideoFormat_NV12 and
        // MFVideoFormat_YUY2
        if (IsEqualGUID(majorType, MFMediaType_Video) &&
          (IsEqualGUID(subtype, MFVideoFormat_NV12) ||
            IsEqualGUID(subtype, MFVideoFormat_YUY2))) {
          hr = MFGetAttributeRatio(pNativeType, MF_MT_FRAME_RATE, &frameRate,
            &denominator);
          if (FAILED(hr))
            break;

          // Filter to supported frame rates
          if (frameRate > MAX_CAMERA_FRAME_RATE) {
            continue;
          }

          hr = MFGetAttributeSize(pNativeType, MF_MT_FRAME_SIZE, &width, &height);
          if (FAILED(hr))
            break;

          // Filter to 4:3 only as certain cameras claim to support 16:9 but display
          // in pillarbox
          if (4 * height != 3 * width) {
            continue;
          }

          // Filter to width and height
          // Apparently this is the maximum supported size that allows
          // us to MFCopyImage successfully, otherwise the buffer is not large enough
          // to target our generated texture of the same dimensions.
          // This should be fine as most devs should limit the screen space taken by the camera
          // And the smaller the more performant for our mapping operations.
          if (width > 640 || height > 480) {
            continue;
          }

          // Maximize camera frame rate
          if (frameRate < maxFrameRate) {
            continue;
          }

          // Maximize camera capture width for equal frame rates
          if (frameRate == maxFrameRate && width < maxWidth) {
            continue;
          }

          // Filter out MFVideoArea support
          HRESULT videoAreaHR;
          MFVideoArea videoArea;
          memset(&videoArea, 0, sizeof(MFVideoArea));
          BOOL bPanScan = FALSE;
          bPanScan =
            MFGetAttributeUINT32(pNativeType, MF_MT_PAN_SCAN_ENABLED, FALSE);
          if (bPanScan) {
            videoAreaHR =
              pNativeType->GetBlob(MF_MT_PAN_SCAN_APERTURE, (UINT8 *)&videoArea,
                sizeof(MFVideoArea), NULL);
          }
          if (!bPanScan || videoAreaHR == MF_E_ATTRIBUTENOTFOUND) {
            videoAreaHR = pNativeType->GetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE,
              (UINT8 *)&videoArea,
              sizeof(MFVideoArea), NULL);
            if (videoAreaHR == MF_E_ATTRIBUTENOTFOUND) {
              videoAreaHR = pNativeType->GetBlob(MF_MT_GEOMETRIC_APERTURE,
                (UINT8 *)&videoArea,
                sizeof(MFVideoArea), NULL);
            }
          }
          if (SUCCEEDED(videoAreaHR)) {
            continue;
          }

          bestDwMediaTypeIndex = dwMediaTypeIndex - 1;
          maxWidth = width;
          maxFrameRate = frameRate;
          found = true;
        }
      }
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to filter target media in camera reader. [Error code] ", hr);
        return hr;
      }
      if (!found) {
        DEBUG_ERROR(
          "Failed to initialize camera reader. No suitable camera media type.");
        return S_FALSE;
      }

      ScopedCOMPtr<IMFMediaType> pFoundNativeType;
      hr = pReader->GetNativeMediaType(streamIndex, bestDwMediaTypeIndex,
        &pFoundNativeType);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to GetNativeMediaType in camera reader. [Error code] ", hr);
        return hr;
      }

      // Have source reader target RGB32
      ScopedCOMPtr<IMFMediaType> pTargetType;
      hr = MFCreateMediaType(&pTargetType);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to MFCreateMediaType in camera reader. [Error code] ", hr);
        return hr;
      }

      hr = pFoundNativeType->CopyAllItems(pTargetType);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to CopyAllItems in camera reader. [Error code] ", hr);
        return hr;
      }

      hr = pTargetType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to set MF_MT_SUBTYPE in camera reader. [Error code] ", hr);
        return hr;
      }

      hr = pReader->SetCurrentMediaType(streamIndex, nullptr, pTargetType);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to SetCurrentMediaType in camera reader. [Error code] ", hr);
        return hr;
      }

      // Get camera target media type width and height
      hr = MFGetAttributeSize(pTargetType, MF_MT_FRAME_SIZE, &width, &height);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to MFGetAttributeSize in camera reader. [Error code] ", hr);
        return hr;
      }

      hr = MFGetStrideForBitmapInfoHeader(MFVideoFormat_RGB32.Data1, width, &cameraTextureStride);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to get stride in camera reader. [Error code] ", hr);
        return hr;
      }

      cameraTextureWidth = (uint32_t)width;
      cameraTextureHeight = (uint32_t)height;

      // Create read sample target texture
      D3D11_TEXTURE2D_DESC desc = {};
      ZeroMemory(&desc, sizeof(desc));
      desc.Width = cameraTextureWidth;
      desc.Height = cameraTextureHeight;
      desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.ArraySize = 1;
      desc.MipLevels = 1;
      desc.Usage = D3D11_USAGE_STAGING;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

      hr = cameraDeviceManager.getDevice()->CreateTexture2D(&desc, nullptr,
        &pCameraTextureTarget);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to create target Texture2D in camera reader. [Error code] ",
          hr);
        return hr;
      }

      // Create read sample shared texture
      ZeroMemory(&desc, sizeof(desc));
      desc.Width = cameraTextureWidth;
      desc.Height = cameraTextureHeight;
      desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.ArraySize = 1;
      desc.MipLevels = 1;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

      hr = cameraDeviceManager.getDevice()->CreateTexture2D(
        &desc, nullptr, &pCameraTextureShareable);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to create shared Texture2D in camera reader. [Error code] ",
          hr);
        return hr;
      }

      // Create read sample shared texture handles
      hr = pCameraTextureShareable->QueryInterface(
        IID_PPV_ARGS(&pCameraTextureShareableResource));
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to query IDXGIResource in camera reader. [Error code] ", hr);
        return hr;
      }

      hr = pCameraTextureShareableResource->GetSharedHandle(
        &cameraTextureShareableHandle);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to get shared handle in camera reader. [Error code] ", hr);
        return hr;
      }

      hr = pCameraTextureShareable->QueryInterface(
        IID_PPV_ARGS(&pCameraTextureTargetKeyedMutex));
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to query IDXGIKeyedMutex in camera reader. [Error code] ", hr);
        return hr;
      }

      initialized = true;
      return hr;
    }

    bool CameraReader::beginReadLoop() {
      if (!initialized || readLoop) {
#ifdef DEBUG
        assert(false);
#endif
        return false;
      }

      HRESULT hr = pReader->ReadSample(streamIndex, 0, NULL, NULL, NULL, NULL);
      if (FAILED(hr)) {
        DEBUG_HRESULT_ERROR(
          "Failed to do first ReadSample in camera reader. [Error code] ", hr);
        return false;
      }
      readLoop = true;
      return true;
    }

    bool CameraReader::terminate() {
      terminated = true;
      HRESULT hr = pReader->Flush(streamIndex);
      return SUCCEEDED(hr);
    }

    HRESULT
      CameraReader::markCameraReaderTerminatedByHrStatus(const std::string &log,
        HRESULT hr) {
      terminated = true;
      DEBUG_HRESULT_ERROR(log, hr);
      LeaveCriticalSection(&critsec);
      return hr;
    }

    STDMETHODIMP CameraReader::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,
      DWORD flags, LONGLONG llTimestamp,
      IMFSample *pSample) {

      EnterCriticalSection(&critsec);

      if (terminated) {
        LeaveCriticalSection(&critsec);
        return S_FALSE;
      }

      if (FAILED(hrStatus)) {
        return markCameraReaderTerminatedByHrStatus(
          "Failed to ReadSample in camera reader. [Error code] ", hrStatus);
      }

      auto clockNow = std::chrono::high_resolution_clock::now();
      auto clockDiff = clockNow - lastSampleStartTime;
      auto clockMillisDiff =
        std::chrono::duration_cast<std::chrono::milliseconds>(clockDiff).count();

      llLastSampleTimestamp = llTimestamp;

      bool requestNewSample = false;
      bool readSample = false;
      bool flaggedError = false;

      DWORD errorFlags[] = {
          MF_SOURCE_READERF_ENDOFSTREAM, MF_SOURCE_READERF_NEWSTREAM,
          MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED,
          MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED, MF_SOURCE_READERF_ERROR };
      size_t errorFlagsSize = ARRAYSIZE(errorFlags);
      for (size_t i = 0; i < errorFlagsSize; i++) {
        if (flags & errorFlags[i]) {
          flaggedError = true;
        }
      }

      if (!flaggedError) {
        if (flags & MF_SOURCE_READERF_STREAMTICK) {
          requestNewSample = true;
        }
        else if (pSample) {
          requestNewSample = true;
          readSample = true;
        }
      }

      HRESULT hr = S_OK;
      if (!readSample) {
        if (requestNewSample) {
          lastSampleStartTime = clockNow;
          hr = pReader->ReadSample(dwStreamIndex, 0, NULL, NULL, NULL, NULL);
          if (FAILED(hr)) {
            return markCameraReaderTerminatedByHrStatus(
              "Failed to ReadSample in camera reader. [Error code] ", hrStatus);
          }
          LeaveCriticalSection(&critsec);
          return hr;
        }
        terminated = true;
        LeaveCriticalSection(&critsec);
        return hr;
      }

      DWORD count;
      hr = pSample->GetBufferCount(&count);
      if (FAILED(hr)) {
        return markCameraReaderTerminatedByHrStatus(
          "Failed to ReadSample in camera reader. [Error code] ", hrStatus);
      }

      if (count != 1)
        hr = S_FALSE;
      if (FAILED(hr)) {
        return markCameraReaderTerminatedByHrStatus(
          "Invalid buffer count in camera reader. [Error code] ", hrStatus);
      }

      ScopedCOMPtr<IMFMediaBuffer> pBuffer;
      hr = pSample->GetBufferByIndex(0, &pBuffer);
      if (FAILED(hr)) {
        return markCameraReaderTerminatedByHrStatus(
          "Failed to GetBufferByIndex in camera reader. [Error code] ", hrStatus);
      }

      BYTE * pBufferData = nullptr;
      hr = pBuffer->Lock(&pBufferData, nullptr, nullptr);
      if (FAILED(hr)) {
        return markCameraReaderTerminatedByHrStatus(
          "Failed to lock IMFMediaBuffer in camera reader. [Error code] ",
          hrStatus);
      }

      D3D11_MAPPED_SUBRESOURCE resource = {};
      hr = cameraDeviceManager.getDeviceContext()->Map(pCameraTextureTarget, 0, D3D11_MAP_READ_WRITE, 0, &resource);
      if (FAILED(hr)) {
        return markCameraReaderTerminatedByHrStatus(
          "Failed to map camera texture target in camera reader. [Error code] ",
          hrStatus);
      }

      DWORD currentLength = 0;
      pBuffer->GetCurrentLength(&currentLength);

      if (cameraTextureStride < 0) {
        hr = MFCopyImage((byte*)resource.pData, resource.RowPitch, pBufferData + (cameraTextureHeight-1)*(cameraTextureStride*-1), cameraTextureStride, cameraTextureWidth*4, cameraTextureHeight);
      }
      else {
        hr = MFCopyImage((byte*)resource.pData, resource.RowPitch, pBufferData, cameraTextureStride, cameraTextureWidth*4, cameraTextureHeight);
      }
      if (FAILED(hr)) {
        cameraDeviceManager.getDeviceContext()->Unmap(pCameraTextureTarget, 0);
        return markCameraReaderTerminatedByHrStatus(
          "Failed to MFCopyImage in camera reader. [Error code] ",
          hrStatus);
      }

      cameraDeviceManager.getDeviceContext()->Unmap(pCameraTextureTarget, 0);

      hr = pBuffer->Unlock();
      if (FAILED(hr)) {
        return markCameraReaderTerminatedByHrStatus(
          "Failed to unlock IMFMediaBuffer in camera reader. [Error code] ",
          hrStatus);
      }

      hr = pCameraTextureTargetKeyedMutex->AcquireSync(0, INFINITE);
      if (FAILED(hr)) {
        return markCameraReaderTerminatedByHrStatus(
          "Failed to open camera shared resource in camera reader. [Error code] ",
          hrStatus);
      }

      auto deviceContext = cameraDeviceManager.getDeviceContext();
      deviceContext->CopyResource(pCameraTextureShareable, pCameraTextureTarget);
      deviceContext->Flush();

      hr = pCameraTextureTargetKeyedMutex->ReleaseSync(0);
      if (FAILED(hr)) {
        return markCameraReaderTerminatedByHrStatus(
          "Failed to release camera shared resource in camera reader. [Error "
          "code] ",
          hrStatus);
      }

      filled = true;

      if (requestNewSample) {
        lastSampleStartTime = clockNow;
        hr = pReader->ReadSample(dwStreamIndex, 0, NULL, NULL, NULL, NULL);
        if (FAILED(hr)) {
          return markCameraReaderTerminatedByHrStatus(
            "Failed to ReadSample in camera reader. [Error code] ", hrStatus);
        }
        LeaveCriticalSection(&critsec);
        return hr;
      }
      terminated = true;
      LeaveCriticalSection(&critsec);
      return hr;
    }

    STDMETHODIMP CameraReader::OnEvent(DWORD, IMFMediaEvent *) { return S_OK; }

  }
}