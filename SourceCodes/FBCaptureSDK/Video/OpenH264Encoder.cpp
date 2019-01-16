/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "OpenH264Encoder.h"
#include "Graphics/TextureRender.h"
#include "Graphics/ScreenVertexShader.h"

using namespace FBCapture::Render;
namespace FBCapture {
	namespace Video {
		OpenH264Encoder::OpenH264Encoder(ID3D11Device* device) {
			device_ = device;
			encodingInitiated_ = false;
		}

		FBCAPTURE_STATUS OpenH264Encoder::initEncodingSession() {
			int32_t ret = 0;

			if (svcEncoder_) {
				return FBCAPTURE_STATUS_OK;
			}

			ret = WelsCreateSVCEncoder(&svcEncoder_);
			if (ret != cmResultSuccess) {
				DEBUG_ERROR_VAR("Failed to create encoder. [Error Code] ", to_string(ret));
				return FBCAPTURE_STATUS_ENCODER_CREATION_FAILED;
			}

			return FBCAPTURE_STATUS_OK;
		}

		FBCAPTURE_STATUS OpenH264Encoder::setEncodeConfigs(const wstring& fullSavePath,
																											 uint32_t width,
																											 uint32_t height,
																											 int bitrate,
																											 int fps) {
			int32_t ret = 0;			
			uint32_t kiPicResSize = width * height * 3 >> 1;			
			pYUV420_ = new uint8_t[kiPicResSize];

			// Set source picture configures
			memset(&srcPic_, 0, sizeof(SSourcePicture));
			srcPic_.iColorFormat = videoFormatI420;
			srcPic_.uiTimeStamp = 0;
			srcPic_.iPicWidth = width;
			srcPic_.iPicHeight = height;
			srcPic_.iStride[0] = srcPic_.iPicWidth;
			srcPic_.iStride[1] = srcPic_.iStride[2] = srcPic_.iStride[0] >> 1;
			srcPic_.pData[0] = pYUV420_;
			srcPic_.pData[1] = srcPic_.pData[0] + (srcPic_.iPicWidth * srcPic_.iPicHeight);
			srcPic_.pData[2] = srcPic_.pData[1] + (srcPic_.iPicWidth * srcPic_.iPicHeight >> 2);

			// Set encoding configs						
			svcEncoder_->GetDefaultParams(&encodeConfig_);
			encodeConfig_.iUsageType = SCREEN_CONTENT_REAL_TIME;
			encodeConfig_.fMaxFrameRate = static_cast<float>(fps);
			encodeConfig_.iTargetBitrate = bitrate;
			encodeConfig_.iPicWidth = width;
			encodeConfig_.iPicHeight = height;
			encodeConfig_.iRCMode = RC_QUALITY_MODE;
			encodeConfig_.iTemporalLayerNum = 1;
			for (int layer = 0; layer < encodeConfig_.iSpatialLayerNum; layer++) {
				encodeConfig_.sSpatialLayers[layer].iVideoWidth = width;
				encodeConfig_.sSpatialLayers[layer].iVideoHeight = height;
				encodeConfig_.sSpatialLayers[layer].fFrameRate = static_cast<float>(fps);
				encodeConfig_.sSpatialLayers[layer].iSpatialBitrate = bitrate;
				encodeConfig_.sSpatialLayers[layer].iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;
				encodeConfig_.sSpatialLayers[layer].sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;
			}

			ret = svcEncoder_->InitializeExt(&encodeConfig_);
			if (ret != cmResultSuccess) {
				DEBUG_ERROR_VAR("Failed on initilaizing encoder with given extension parameters. [Error Code] ",
					to_string(ret));
				return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
			}

			file_ = _wfopen(fullSavePath.c_str(), L"wb");
			if (file_ == NULL) {
				DEBUG_ERROR("Failed on creating file with _wfopen");
				return FBCAPTURE_STATUS_OUTPUT_FILE_CREATION_FAILED;
			}

			return FBCAPTURE_STATUS_OK;
		}

		FBCAPTURE_STATUS OpenH264Encoder::encodeProcess(const void* texturePtr,
																										const wstring& fullSavePath,
																										int bitrate,
																										int fps,
																										bool needFlipping) {
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
			int32_t ret = 0;

			status = createD3D11Resources((ID3D11Texture2D*)texturePtr, encodingTexure_);
			if (status != FBCAPTURE_STATUS_OK) {
				DEBUG_ERROR("Failed to create texture");
				return status;
			}

			// Initialize Encoder
			if (!encodingInitiated_) {
				status = setEncodeConfigs(fullSavePath, globalTexDesc_.Width, globalTexDesc_.Height, bitrate, fps);  // Set Encode Configs
				if (status != FBCAPTURE_STATUS_OK) {
					return status;
				}
				setTextureDirtyRegion();
				encodingInitiated_ = true;
			}

			D3D11_MAPPED_SUBRESOURCE resource = {};
			status = mapTexture(resource);
			if (status != FBCAPTURE_STATUS_OK) {
				DEBUG_ERROR("Failed on context mapping");
				return status;
			}

			// TODO Write seperate shader doing chroma subsampling.
			chromaSubSamplingFromYUV444toYUV420(reinterpret_cast<byte*>(resource.pData), reinterpret_cast<byte*>(pYUV420_), srcPic_.iPicWidth, srcPic_.iPicHeight);

			// To encoder this frame			
			memset(&frameBSInfo_, 0, sizeof(SFrameBSInfo));
			ret = svcEncoder_->EncodeFrame(&srcPic_, &frameBSInfo_);
			if (ret == cmResultSuccess) {
				int layer = 0;
				int frameSize = 0;
				while (layer < frameBSInfo_.iLayerNum) {
					SLayerBSInfo* pLayerBsInfo = &frameBSInfo_.sLayerInfo[layer];
					if (pLayerBsInfo != NULL) {
						int layerSize = 0;
						int nalIdx = pLayerBsInfo->iNalCount - 1;
						do {
							layerSize += pLayerBsInfo->pNalLengthInByte[nalIdx];
							--nalIdx;
						} while (nalIdx >= 0);

						fwrite(pLayerBsInfo->pBsBuf, 1, layerSize, file_); // write pure bit stream into file
						frameSize += layerSize;
					}
					++layer;
				}
			} else {
				DEBUG_ERROR_VAR("Failed on encoding frame. [Error Code] ", to_string(ret));
				return FBCAPTURE_STATUS_ENCODE_PICTURE_FAILED;
			}

			return status;
		}

		void OpenH264Encoder::chromaSubSamplingFromYUV444toYUV420(byte* yuv444, byte* yuv420, int width, int height) {
			int frameSize = width * height;
			int chromasize = frameSize / 4;

			int yIndex = 0;
			int uIndex = frameSize;
			int vIndex = frameSize + chromasize;

			int R = 0, G = 0, B = 0, Y = 0, U = 0, V = 0;
			int yuv444Index = 0;

			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					Y = yuv444[yuv444Index++];
					U = yuv444[yuv444Index++];
					V = yuv444[yuv444Index++];
					yuv444Index++; // Skip Alpha channel

					yuv420[yIndex++] = Y;

					// TODO Consider interplorating multiple neighbor's UV vales to get better UV on YUV420 planar
					if (j % 2 == 0 && i % 2 == 0) {
						yuv420[uIndex++] = U;
						yuv420[vIndex++] = V;
					}
				}
			}		
		}

		FBCAPTURE_STATUS OpenH264Encoder::flushInputTextures() {
			encodingInitiated_ = false;

			if (file_) {
				fclose(file_);
				file_ = nullptr;
			}

			if (pYUV420_) {
				delete[] pYUV420_;
				pYUV420_ = nullptr;
			}

			if (svcEncoder_) {
				svcEncoder_->Uninitialize();
				WelsDestroySVCEncoder(svcEncoder_);
				svcEncoder_ = nullptr;
			}

			return FBCAPTURE_STATUS_OK;
		}
	}
}