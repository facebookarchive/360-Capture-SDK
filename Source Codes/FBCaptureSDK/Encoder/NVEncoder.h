/***************************************************************************************************************

Filename	:	NVEncoder.h
Content		:	NVidia Encoder implementation for creating h264 format video
Created		:	December 13, 2016
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/
#pragma once

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include <direct.h>
#include <codecvt>
#define GetCurrentDir _getcwd
#if defined(WIN32)
#include <d3d11.h>
#include <wincodec.h>
#pragma warning(disable : 4996)
#endif
#include "NVidia/common/inc/nvEncodeAPI.h"
#include "NVidia/common/inc/nvCPUOPSys.h"
#include "NVidia/common/inc/NvHWEncoder.h"
#include "ScreenGrab.h"
#include "Common.h"
#include "ScopedCOMPtr.h"

#define MAX_ENCODE_QUEUE 32

// BITSTREAM_BUFFER_SIZE macro
#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024

using namespace FBCapture::Common;
using namespace Directx;
using namespace std;

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
				pendingndex_(0), buffer_(NULL) {}

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

		// Event types for UnitySetGraphicsDevice
		enum GfxDeviceEventType {
			kGfxDeviceEventInitialize = 0,
			kGfxDeviceEventShutdown = 1,
			kGfxDeviceEventBeforeReset = 2,
			kGfxDeviceEventAfterReset = 3,
		};

		// Graphics device identifiers in Unity
		enum GfxDeviceRenderer {
			kGfxRendererOpenGL = 0, // OpenGL
			kGfxRendererD3D9,		// Direct3D 9
			kGfxRendererD3D11		// Direct3D 11
		};

		class NVEncoder {
		public:
			NVEncoder(ID3D11Device* device);  // Constructor
			virtual ~NVEncoder();  // Destructor      

			FBCAPTURE_STATUS encodeMain(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping);  // Main loop function for encoding      
			FBCAPTURE_STATUS saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool is360);  // Take screenshot																																																								
			FBCAPTURE_STATUS flushInputTextures(); // Flush queued textures in buffers to create video
			FBCAPTURE_STATUS initNVEncodingSession();			

		protected:
			// To access the NVidia HW Encoder interfaces
			CNvHWEncoder *nvHWEncoder_ = nullptr;
			uint32_t encodeBufferCount_ = 0;
			uint32_t picStruct_ = 0;
			EncodeOutputBuffer eosOutputBfr_ = {};
			EncodeBuffer encodeBuffer_[MAX_ENCODE_QUEUE] = {};
			EncodeConfig encodeConfig_ = {};
			CNvQueue<EncodeBuffer> encodeBufferQueue_ = {};

			// DX 11 interfaces      
			ID3D11Texture2D* fromTexturePtr_ = nullptr;
			ScopedCOMPtr<ID3D11DeviceContext> context_ = nullptr;			
			ScopedCOMPtr<ID3D11Texture2D> encodingTexure_ = nullptr;
			ScopedCOMPtr<ID3D11Device> device_ = nullptr;
			D3D11_TEXTURE2D_DESC globalTexDesc_ = {};
			D3D11_BOX dirtyRegion_ = {};

			string videoFileName_ = {};
			bool encodingInitiated_ = false;

		protected:

			FBCAPTURE_STATUS createD3D11Resources(ID3D11Texture2D* texture);

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

			// Set all configurations need to be set for encoding
			// It's called only once when starting encoding
			FBCAPTURE_STATUS setEncodeConfigures(const wstring& fullSavePath, uint32_t width, uint32_t height, int bitrate, int fps);

			// Set texture dirty region
			// It's called only once in entire app life time
			void setTextureDirtyRegion();

			// Release all resources allocated for encoding
			NVENCSTATUS releaseEncodeResources();

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