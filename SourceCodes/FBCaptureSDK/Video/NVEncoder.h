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

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include <direct.h>
#include <codecvt>
#include "3rdParty/NVidia/common/inc/nvEncodeAPI.h"
#include "3rdParty/NVidia/common/inc/nvCPUOPSys.h"
#include "3rdParty/NVidia/common/inc/NvHWEncoder.h"
#include "Video/EncoderManager.h"

#define MAX_ENCODE_QUEUE 32

// BITSTREAM_BUFFER_SIZE macro
#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024

namespace FBCapture {
  namespace Video {

    // The class for queuing is used to manage encode buffers
    template<class T>
    class CNvQueue {
      T** buffer_;
      unsigned int size_;
      unsigned int pendingCount_;
      unsigned int availableIdx_;
      unsigned int pendingndex_;
    public:
      CNvQueue() : size_(0), pendingCount_(0), availableIdx_(0),
        pendingndex_(0), buffer_(NULL) {
      }

      ~CNvQueue() {
        delete[] buffer_;
      }

      bool initialize(T *items, unsigned int size) {
        size_ = size;
        pendingCount_ = 0;
        availableIdx_ = 0;
        pendingndex_ = 0;
        buffer_ = new T *[size_];
        for (unsigned int i = 0; i < size_; i++) {
          buffer_[i] = &items[i];
        }
        return true;
      }

      T * getAvailable() {
        T *item = NULL;
        if (pendingCount_ == size_) {
          return NULL;
        }
        item = buffer_[availableIdx_];
        availableIdx_ = (availableIdx_ + 1) % size_;
        pendingCount_ += 1;
        return item;
      }

      T* getPending() {
        if (pendingCount_ == 0) {
          return NULL;
        }

        T *item = buffer_[pendingndex_];
        pendingndex_ = (pendingndex_ + 1) % size_;
        pendingCount_ -= 1;
        return item;
      }
    };

    // Device type to be used for NV encoder initialization
    typedef enum {
      NV_ENC_DX9 = 0,
      NV_ENC_DX11 = 1,
      NV_ENC_CUDA = 2,
      NV_ENC_DX10 = 3,
    } NvEncodeDeviceType; 

    class NVEncoder : public EncoderManager {
    public:
      NVEncoder(ID3D11Device* device);  // Constructor
      virtual ~NVEncoder();  // Destructor      

		public:
			FBCAPTURE_STATUS initEncodingSession() override;
			FBCAPTURE_STATUS encodeProcess(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) override;  // Main loop function for encoding			
			FBCAPTURE_STATUS flushInputTextures() override; // Flush queued textures in buffers to create video			
			FBCAPTURE_STATUS releaseEncodingResources();  // Release all resources allocated for encoding			
			FBCAPTURE_STATUS dummyTextureEncoding();  // Dummy texture encoding. We need to encode dummy texture for one frame so that we can clearly destroy encoder after capability check	
    protected:
      // To access the NVidia HW Encoder interfaces
      CNvHWEncoder *nvHWEncoder_ = nullptr;
      uint32_t encodeBufferCount_ = 0;
      uint32_t picStruct_ = 0;
      EncodeOutputBuffer eosOutputBfr_ = {};
      EncodeBuffer encodeBuffer_[MAX_ENCODE_QUEUE] = {};
      EncodeConfig encodeConfig_ = {};
      CNvQueue<EncodeBuffer> encodeBufferQueue_ = {};

    protected:
			FBCAPTURE_STATUS setEncodeConfigs(const wstring& fullSavePath, uint32_t width, uint32_t height, int bitrate, int fps) override;

      // Encode images
      // This function should be called in the last stage in function calls
      // i.e. After allocating buffers and copying resources
      NVENCSTATUS encodeFrame(uint32_t width, uint32_t height, NV_ENC_BUFFER_FORMAT inputformat);

      // Allocate Buffers
      // Input buffers and bitsteam buffers need to be allocated only once
      NVENCSTATUS allocateIOBuffers(uint32_t uInputWidth, uint32_t uInputHeight, NV_ENC_BUFFER_FORMAT inputFormat);

      // Copy textures to encode buffer
      NVENCSTATUS	copyReources(uint32_t width, uint32_t height);

      // Flush encoder
      // This function will be called when encoding is done
      NVENCSTATUS flushEncoder();

      // Release allocated buffers in AllocateIOBuffers after flushing or terminating process
      void releaseIOBuffers();

      // Release resources related to DX11
      void releaseD3D11Resources();

      // Gets the feature level of the hardware device
      // https://msdn.microsoft.com/en-us/library/windows/desktop/ff476329(v=vs.85).aspx
      virtual bool getUsesReverseZ() { return (int)device_->GetFeatureLevel() >= (int)D3D_FEATURE_LEVEL_10_0; }
    };

    // NVEncodeAPI entry point
    typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);
  }
}
