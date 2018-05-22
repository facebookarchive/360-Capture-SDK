/****************************************************************************************************************

Filename	:	NVEncoder.cpp
Content		:	NVidia Encoder implementation for creating h264 format video
Created		:	December 13, 2016
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#include "NVEncoder.h"
#include "NVidia/common/inc/nvFileIO.h"
#include "NVidia/common/inc/nvUtils.h"
#include "Log.h"

// To print out error string got from NV HW encoder API
const char* nvidiaStatus[] = { "NV_ENC_SUCCESS", "NV_ENC_ERR_NO_ENCODE_DEVICE", "NV_ENC_ERR_UNSUPPORTED_DEVICE", "NV_ENC_ERR_INVALID_ENCODERDEVICE",
"NV_ENC_ERR_INVALID_DEVICE", "NV_ENC_ERR_DEVICE_NOT_EXIST", "NV_ENC_ERR_INVALID_PTR", "NV_ENC_ERR_INVALID_EVENT",
"NV_ENC_ERR_INVALID_PARAM", "NV_ENC_ERR_INVALID_CALL", "NV_ENC_ERR_OUT_OF_MEMORY", "NV_ENC_ERR_ENCODER_NOT_INITIALIZED",
"NV_ENC_ERR_UNSUPPORTED_PARAM", "NV_ENC_ERR_LOCK_BUSY", "NV_ENC_ERR_NOT_ENOUGH_BUFFER", "NV_ENC_ERR_INVALID_VERSION",
"NV_ENC_ERR_MAP_FAILED", "NV_ENC_ERR_NEED_MORE_INPUT", "NV_ENC_ERR_ENCODER_BUSY", "NV_ENC_ERR_EVENT_NOT_REGISTERD",
"NV_ENC_ERR_GENERIC", "NV_ENC_ERR_INCOMPATIBLE_CLIENT_KEY", "NV_ENC_ERR_UNIMPLEMENTED", "NV_ENC_ERR_RESOURCE_REGISTER_FAILED",
"NV_ENC_ERR_RESOURCE_NOT_REGISTERED", "NV_ENC_ERR_RESOURCE_NOT_MAPPED" };

namespace FBCapture {
	namespace Video {

		NVEncoder::NVEncoder(ID3D11Device* device) {
			device_ = device;

			encodingInitiated_ = false;
			encodeBufferCount_ = 1;
			eosOutputBfr_ = {};

			memset(encodeBuffer_, 0, MAX_ENCODE_QUEUE * sizeof(EncodeBuffer));
		}

		NVEncoder::~NVEncoder() {
			NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

			// Clean up session
			if (nvHWEncoder_) {
				nvHWEncoder_->NvEncDestroyEncoder();
				delete nvHWEncoder_;
				nvHWEncoder_ = nullptr;
			}

			releaseD3D11Resources();

		}

		void NVEncoder::setTextureDirtyRegion() {
			// Set texure dirty region
			dirtyRegion_.left = 0;
			dirtyRegion_.right = globalTexDesc_.Width;
			dirtyRegion_.top = 0;
			dirtyRegion_.bottom = globalTexDesc_.Height;
			dirtyRegion_.front = 0;
			dirtyRegion_.back = 1;
		}


		FBCAPTURE_STATUS NVEncoder::setEncodeConfigures(const wstring& fullSavePath, uint32_t width, uint32_t height, int bitrate, int fps) {
			wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
			videoFileName_ = stringTypeConversion.to_bytes(fullSavePath);

			// Set encoding configuration
			memset(&encodeConfig_, 0, sizeof(EncodeConfig));
			encodeConfig_.endFrameIdx = INT_MAX;
			encodeConfig_.bitrate = bitrate;
			encodeConfig_.rcMode = NV_ENC_PARAMS_RC_CONSTQP;
			encodeConfig_.gopLength = NVENC_INFINITE_GOPLENGTH;
			encodeConfig_.deviceType = NV_ENC_DX11;
			encodeConfig_.codec = NV_ENC_H264;
			encodeConfig_.fps = fps;
			encodeConfig_.qp = 28;
			encodeConfig_.i_quant_factor = DEFAULT_I_QFACTOR;
			encodeConfig_.b_quant_factor = DEFAULT_B_QFACTOR;
			encodeConfig_.i_quant_offset = DEFAULT_I_QOFFSET;
			encodeConfig_.b_quant_offset = DEFAULT_B_QOFFSET;
			encodeConfig_.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
			encodeConfig_.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
			encodeConfig_.inputFormat = NV_ENC_BUFFER_FORMAT_ABGR;
			encodeConfig_.width = width;
			encodeConfig_.height = height;
			encodeConfig_.outputFileName = videoFileName_.c_str();
			encodeConfig_.fOutput = fopen(encodeConfig_.outputFileName, "wb");
			if (encodeConfig_.fOutput == nullptr) {
				DEBUG_ERROR_VAR("Failed to create ", videoFileName_.c_str());
				fclose(encodeConfig_.fOutput);
				remove(videoFileName_.c_str());
				return FBCAPTURE_STATUS_OUTPUT_FILE_OPEN_FAILED;
			}

			if (!encodeConfig_.outputFileName || encodeConfig_.width == 0 || encodeConfig_.height == 0) {
				DEBUG_ERROR("Invalid texture file");
				return FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED;
			}

			return FBCAPTURE_STATUS_OK;
		}

		void NVEncoder::releaseD3D11Resources() {
			encodingTexure_ = nullptr;
			device_ = nullptr;
			context_ = nullptr;
		}

		NVENCSTATUS NVEncoder::releaseEncodeResources() {
			NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

			releaseIOBuffers();

			if (encodeConfig_.fOutput) {
				fclose(encodeConfig_.fOutput);
			}

			// Clean up current encode component
			if (nvHWEncoder_->m_pEncodeAPI) {
				nvStatus = nvHWEncoder_->NvEncDestroyEncoder();
				delete nvHWEncoder_->m_pEncodeAPI;
				nvHWEncoder_->m_pEncodeAPI = nullptr;
			}

			return nvStatus;
		}

		void NVEncoder::releaseIOBuffers() {
			for (uint32_t i = 0; i < encodeBufferCount_; i++) {
				if (encodeBuffer_[i].stInputBfr.pNVSurface) {
					encodeBuffer_[i].stInputBfr.pNVSurface->Release();
					encodeBuffer_[i].stInputBfr.pNVSurface = nullptr;
				}

				if (encodeBuffer_[i].stInputBfr.hInputSurface) {
					nvHWEncoder_->NvEncDestroyInputBuffer(encodeBuffer_[i].stInputBfr.hInputSurface);
					encodeBuffer_->stInputBfr.hInputSurface = nullptr;
				}

				if (encodeBuffer_[i].stOutputBfr.hBitstreamBuffer) {
					nvHWEncoder_->NvEncDestroyBitstreamBuffer(encodeBuffer_[i].stOutputBfr.hBitstreamBuffer);
					encodeBuffer_->stOutputBfr.hBitstreamBuffer = nullptr;
				}

				if (encodeBuffer_[i].stOutputBfr.hOutputEvent) {
					nvHWEncoder_->NvEncUnregisterAsyncEvent(encodeBuffer_[i].stOutputBfr.hOutputEvent);
					nvCloseFile(encodeBuffer_[i].stOutputBfr.hOutputEvent);
					encodeBuffer_[i].stOutputBfr.hOutputEvent = nullptr;
				}
			}

			if (eosOutputBfr_.hOutputEvent) {
				nvHWEncoder_->NvEncUnregisterAsyncEvent(eosOutputBfr_.hOutputEvent);
				nvCloseFile(eosOutputBfr_.hOutputEvent);
				eosOutputBfr_.hOutputEvent = nullptr;
			}
		}

		NVENCSTATUS NVEncoder::flushEncoder() {
			NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

			if (eosOutputBfr_.hOutputEvent == NULL)
				return NV_ENC_ERR_INVALID_CALL;

			nvStatus = nvHWEncoder_->NvEncFlushEncoderQueue(eosOutputBfr_.hOutputEvent);
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR_VAR("Failed on flush. Error code: ", nvidiaStatus[nvStatus]);
				assert(0);
				return nvStatus;
			}

			EncodeBuffer *encodeBuffer = encodeBufferQueue_.getPending();
			while (encodeBuffer) {
				nvHWEncoder_->ProcessOutput(encodeBuffer);
				encodeBuffer = encodeBufferQueue_.getPending();
				// UnMap the input buffer after frame is done
				if (encodeBuffer && encodeBuffer->stInputBfr.hInputSurface) {
					nvStatus = nvHWEncoder_->NvEncUnmapInputResource(encodeBuffer->stInputBfr.hInputSurface);
					encodeBuffer->stInputBfr.hInputSurface = NULL;
				}
			}

			if (WaitForSingleObject(eosOutputBfr_.hOutputEvent, 500) != WAIT_OBJECT_0) {
				assert(0);
				nvStatus = NV_ENC_ERR_GENERIC;
			}

			return nvStatus;
		}

		NVENCSTATUS NVEncoder::allocateIOBuffers(uint32_t width, uint32_t height, NV_ENC_BUFFER_FORMAT inputFormat) {
			NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

			// Initialize encode buffer
			encodeBufferQueue_.initialize(encodeBuffer_, encodeBufferCount_);
			for (uint32_t i = 0; i < encodeBufferCount_; i++) {

				// Create input buffer
				nvStatus = nvHWEncoder_->NvEncCreateInputBuffer(width, height, &encodeBuffer_[i].stInputBfr.hInputSurface, inputFormat);
				if (nvStatus != NV_ENC_SUCCESS) {
					DEBUG_ERROR_VAR("Creating input buffer has failed. Error code: ", nvidiaStatus[nvStatus]);
					return nvStatus;
				}

				encodeBuffer_[i].stInputBfr.bufferFmt = inputFormat;
				encodeBuffer_[i].stInputBfr.dwWidth = width;
				encodeBuffer_[i].stInputBfr.dwHeight = height;
				// Bit stream buffer
				nvStatus = nvHWEncoder_->NvEncCreateBitstreamBuffer(BITSTREAM_BUFFER_SIZE, &encodeBuffer_[i].stOutputBfr.hBitstreamBuffer);
				if (nvStatus != NV_ENC_SUCCESS) {
					DEBUG_ERROR_VAR("Creating bit stream buffer has failed. Error code: ", nvidiaStatus[nvStatus]);
					return nvStatus;
				}

				encodeBuffer_[i].stOutputBfr.dwBitstreamBufferSize = BITSTREAM_BUFFER_SIZE;
				// hOutputEvent
				nvStatus = nvHWEncoder_->NvEncRegisterAsyncEvent(&encodeBuffer_[i].stOutputBfr.hOutputEvent);
				if (nvStatus != NV_ENC_SUCCESS) {
					DEBUG_ERROR_VAR("Registering async event has failed. Error code: ", nvidiaStatus[nvStatus]);
					return nvStatus;
				}
			}
			eosOutputBfr_.bEOSFlag = TRUE;

			nvStatus = nvHWEncoder_->NvEncRegisterAsyncEvent(&eosOutputBfr_.hOutputEvent);
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR_VAR("Registering asyn event has failed. Error code: ", nvidiaStatus[nvStatus]);
				return nvStatus;
			}

			return nvStatus;
		}

		NVENCSTATUS NVEncoder::copyReources(uint32_t width, uint32_t height) {
			NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
			D3D11_MAPPED_SUBRESOURCE resource = {};

			device_->GetImmediateContext(&context_);
			context_->CopySubresourceRegion(encodingTexure_, 0, 0, 0, 0, fromTexturePtr_, 0, &dirtyRegion_);
			HRESULT hr = context_->Map(encodingTexure_, 0, D3D11_MAP_READ_WRITE, 0, &resource);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on context mapping");
				return NV_ENC_ERR_GENERIC;
			}

			// Unmap buffer
			context_->Unmap(encodingTexure_, 0);

			// lock input buffer
			NV_ENC_LOCK_INPUT_BUFFER lockInputBufferParams = {};

			uint32_t pitch = 0;
			lockInputBufferParams.bufferDataPtr = nullptr;

			nvStatus = nvHWEncoder_->NvEncLockInputBuffer(encodeBuffer_->stInputBfr.hInputSurface, &lockInputBufferParams.bufferDataPtr, &pitch);
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR_VAR("Creating nVidia input buffer failed. [Error code] ", nvidiaStatus[nvStatus]);
				return nvStatus;
			}

			// Write into Encode buffer
			memcpy(lockInputBufferParams.bufferDataPtr, resource.pData, height * resource.RowPitch);

			// Unlock input buffer
			nvStatus = nvHWEncoder_->NvEncUnlockInputBuffer(encodeBuffer_->stInputBfr.hInputSurface);
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR_VAR("Failed on nVidia unlock input buffer. [Error code] ", nvidiaStatus[nvStatus]);
				return nvStatus;
			}

			return nvStatus;
		}

		FBCAPTURE_STATUS NVEncoder::initNVEncodingSession() {
			NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

			if (nvHWEncoder_ == nullptr) {
				nvHWEncoder_ = new CNvHWEncoder;
			}

			nvStatus = nvHWEncoder_->Initialize(device_, NV_ENC_DEVICE_TYPE_DIRECTX);
			if (nvStatus == NV_ENC_ERR_INVALID_VERSION) {
				DEBUG_ERROR("Not supported NVidia graphics driver version. Driver version should be 379.95 or newer.");
				status = FBCAPTURE_STATUS_UNSUPPORTED_GRAPHICS_CARD_DRIVER_VERSION;
				return status;
			} else if (nvStatus == NV_ENC_ERR_OUT_OF_MEMORY) {
				DEBUG_ERROR("Hardware encoder doesn't allow to open multiple encoding seesion. Please close another app using different encoding session.");
				status = FBCAPTURE_STATUS_MULTIPLE_ENCODING_SESSION;
				return status;
			} else if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR_VAR("Failed on initializing encoder. [Error code]", nvidiaStatus[nvStatus]);
				status = FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
				return status;
			}

			return status;
		}

		FBCAPTURE_STATUS NVEncoder::createD3D11Resources(ID3D11Texture2D* texture) {
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
			HRESULT hr = S_OK;

			// Create new texture based on RenderTexture in Unity			
			fromTexturePtr_ = texture;
			fromTexturePtr_->GetDesc(&globalTexDesc_);
			globalTexDesc_.BindFlags = 0;
			globalTexDesc_.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
			globalTexDesc_.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			globalTexDesc_.Usage = D3D11_USAGE_STAGING;
			globalTexDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			if (globalTexDesc_.Width > 4096 || globalTexDesc_.Height > 4096) {
				DEBUG_ERROR("Invalid texture resolution. Max resolution is 4096 x 4096 on NVIDIA graphics card");
				return FBCAPTURE_STATUS_INVALID_TEXTURE_RESOLUTION;
			}

			encodingTexure_ = nullptr;
			hr = device_->CreateTexture2D(&globalTexDesc_, nullptr, &encodingTexure_);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating new texture");
				return FBCAPTURE_STATUS_TEXTURE_CREATION_FAILED;
			}

			return status;
		}

		FBCAPTURE_STATUS NVEncoder::encodeMain(const void* texturePtr, const wstring& fullSavePath, int bitrate, int fps, bool needFlipping) {
			int numFramesEncoded = 0;
			NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

			status = createD3D11Resources((ID3D11Texture2D*)texturePtr);
			if (status != FBCAPTURE_STATUS_OK) {
				DEBUG_ERROR("Failed to create texture");
				return status;
			}

			// Initialize Encoder
			if (!encodingInitiated_) {
				status = initNVEncodingSession();
				if (status != FBCAPTURE_STATUS_OK) {
					DEBUG_ERROR("Failed to initialize nvidia encoding session");
					return status;
				}

				// Set Encode Configs
				status = setEncodeConfigures(fullSavePath, globalTexDesc_.Width, globalTexDesc_.Height, bitrate, fps);
				if (status != FBCAPTURE_STATUS_OK) {
					return status;
				}

				encodeConfig_.presetGUID = nvHWEncoder_->GetPresetGUID(encodeConfig_.encoderPreset, encodeConfig_.codec);

				nvStatus = nvHWEncoder_->CreateEncoder(&encodeConfig_);
				if (nvStatus != NV_ENC_SUCCESS) {
					DEBUG_ERROR_VAR("Failed on creating encoder. [Error code]", nvidiaStatus[nvStatus]);
					return FBCAPTURE_STATUS_ENCODER_CREATION_FAILED;
				}

				nvStatus = allocateIOBuffers(encodeConfig_.width, encodeConfig_.height, encodeConfig_.inputFormat);
				if (nvStatus != NV_ENC_SUCCESS) {
					DEBUG_ERROR_VAR("Failed on allocating IO buffers. [Error code]", nvidiaStatus[nvStatus]);
					return FBCAPTURE_STATUS_IO_BUFFER_ALLOCATION_FAILED;
				}

				setTextureDirtyRegion();
				encodingInitiated_ = true;
			}
		
			//Copy framebuffer to encoding buffers
			nvStatus = copyReources(encodeConfig_.width, encodeConfig_.height);
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR("Failed on copying framebuffers to encode  input buffers");
				return FBCAPTURE_STATUS_TEXTURE_RESOURCES_COPY_FAILED;
			}

			// Encode
			nvStatus = encodeFrame(encodeConfig_.width, encodeConfig_.height, encodeConfig_.inputFormat);
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR("Failed on copying framebuffers to encode  input buffers");
				return FBCAPTURE_STATUS_ENCODE_PICTURE_FAILED;
			}

			return status;
		}

		FBCAPTURE_STATUS NVEncoder::flushInputTextures() {
			NVENCSTATUS nvStatus;

			nvStatus = flushEncoder();
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR_VAR("Failed to flush inputs from buffer. [Error code] ", nvidiaStatus[nvStatus]);
				return FBCAPTURE_STATUS_ENCODE_FLUSH_FAILED;
			}

			nvStatus = releaseEncodeResources();
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR_VAR("Failed to release resources. [Error code] ", nvidiaStatus[nvStatus]);
				return FBCAPTURE_STATUS_ENCODE_FLUSH_FAILED;
			}

			encodingInitiated_ = false;

			return FBCAPTURE_STATUS_OK;
		}

		NVENCSTATUS NVEncoder::encodeFrame(uint32_t width, uint32_t height, NV_ENC_BUFFER_FORMAT inputformat) {
			NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

			uint32_t lockedPitch = 0;
			EncodeBuffer *pEncodeBuffer = NULL;

			int8_t* qpDeltaMapArray = NULL;
			unsigned int qpDeltaMapArraySize = 0;

			pEncodeBuffer = encodeBufferQueue_.getAvailable();
			if (!pEncodeBuffer) {
				nvHWEncoder_->ProcessOutput(encodeBufferQueue_.getPending());
				pEncodeBuffer = encodeBufferQueue_.getAvailable();
			}

			nvStatus = nvHWEncoder_->NvEncEncodeFrame(pEncodeBuffer, NULL, width, height, NV_ENC_PIC_STRUCT_FRAME, qpDeltaMapArray, qpDeltaMapArraySize);
			if (nvStatus != NV_ENC_SUCCESS) {
				DEBUG_ERROR_VAR("Failed on encoding frames. Error code:", nvidiaStatus[nvStatus]);
				return nvStatus;
			}

			return nvStatus;
		}

		FBCAPTURE_STATUS NVEncoder::saveScreenShot(const void* texturePtr, const wstring& fullSavePath, bool is360) {
			HRESULT hr = E_FAIL;
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

			createD3D11Resources((ID3D11Texture2D*)texturePtr);
			setTextureDirtyRegion();

			D3D11_MAPPED_SUBRESOURCE resource = {};
			device_->GetImmediateContext(&context_);
			context_->CopySubresourceRegion(encodingTexure_, 0, 0, 0, 0, fromTexturePtr_, 0, &dirtyRegion_);			

			// Screen Capture: Save texture to image file
			hr = SaveWICTextureToFile(context_, encodingTexure_, GUID_ContainerFormatJpeg, fullSavePath.c_str(), nullptr, nullptr, is360);
			if (FAILED(hr)) {
				DEBUG_ERROR_VAR("Failed on creating image file. [Error code] ", to_string(hr));
				return FBCAPTURE_STATUS_WIC_SAVE_IMAGE_FAILED;
			}

			DEBUG_LOG("Succeeded screen capture");

			return status;
		}
	}
}