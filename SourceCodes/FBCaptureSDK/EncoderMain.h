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

#include "RTMP/LibRTMP.h"
#include "Audio/AudioDeviceCapture.h"
#include "Audio/AudioCustomRawDataCapture.h"
#include "Video/NVEncoder.h"
#include "Video/AMDEncoder.h"
#include "Video/OpenH264Encoder.h"
#include "Video/FLVmuxer.h"
#include "Video/MP4muxer.h"
#include "Screenshot/ScreenshotEncoder.h"
#include "Common/Log.h"
#include "Common/Common.h"
#include <sstream>
#include <shlobj.h>
#include <KnownFolders.h>
#include <ctime>
#include <stdio.h>

using namespace FBCapture::Video;
using namespace FBCapture::Audio;
using namespace FBCapture::Screenshot;
using namespace FBCapture::Mux;

namespace FBCapture {
	namespace Common {

		class EncoderMain {

		public:
			EncoderMain();
			~EncoderMain();

		private:
			// Class instances for capture and encode
			std::unique_ptr<NVEncoder>  nvEncoder = {};
			std::unique_ptr<AMDEncoder> amdEncoder = {};
			std::unique_ptr<OpenH264Encoder> swEncoder = {};
			std::unique_ptr<ScreenShotEncoder> screenShotEncoder = {};
			std::unique_ptr<FLVMuxer> flvMuxer = {};
			std::unique_ptr<MP4Muxer> mp4Muxer = {};
			std::unique_ptr<AudioDeviceCapture> audioDeviceCapture = {};
			std::unique_ptr<AudioCustomRawDataCapture> audioCustomRawDataCapture = {};
			std::unique_ptr<AudioEncoder> audioEncoder = {};
			std::unique_ptr<LibRTMP> rtmp = {};

			ID3D11Device* device_ = nullptr;

			bool isLive = false;  // Check if it's Live or VOD mode
			bool silenceMode = false;
			atomic<bool> updateInputName = true;  // Set true when need to update Unix time for unique live video and audio file names
			atomic<bool> stopEncProcess = false;  // Stop encoding process when any critical encoding process is failed
			atomic<bool> isMuxed = false;  // Set to be true after muxing
			atomic<bool> stopEncodingSession = false;  // Set to be true after muxing
			atomic<bool> noAvailableAudioDevice = false;  // Set to be true when available audio divice is not attached

																										// Strings for folder
			wstring liveFolder = {};
			wstring videoH264 = {};
			wstring prevVideoH264 = {};
			string audioWAV = {};
			string prevAudioWAV = {};
			string audioAAC = {};
			string prevAudioAAC = {};

			const string wavExtension = ".wav";
			const string aacExtension = ".aac";
			const wstring h264Extension = L".h264";
			const wstring flvExtension = L".flv";
			const wstring mp4Extension = L".mp4";

			time_t unixTime = 0;
			bool initiatedUnixTime = {};
			bool needEncodingSessionInit = {};
			bool movingToMuxingStage = {};
			bool clientPcmAudioInputEnabled = {};
			bool amdDevice = {};  // When it's amd
			bool nvidiaDevice = {};  // When it's nvidia
			bool softwareEncoder = {};  // software encoder fallback

		public:
			FBCAPTURE_STATUS initEncoderComponents();
			FBCAPTURE_STATUS checkGraphicsCardCapability();
			FBCAPTURE_STATUS initSessionANDcheckDriverCapability();
			FBCAPTURE_STATUS releaseEncodeResources();
			FBCAPTURE_STATUS dummyEncodingSession();			
			FBCAPTURE_STATUS startEncoding(const void* texturePtr, const TCHAR* fullSavePath, bool isLive, int bitrate, int fps, bool needFlipping);
			FBCAPTURE_STATUS audioDeviceCaptureEncoding(bool useVRAudioEndpoint, bool enabledAudioCapture, bool enabledMicCapture, VRDeviceType vrDevice, LPCWSTR useMicIMMDeviceId);
			FBCAPTURE_STATUS audioRawDataEncoding(float* audioData, uint32_t channel, uint32_t sampleRate, uint32_t bufferSize);
			FBCAPTURE_STATUS stopEncoding(bool forceStop);
			FBCAPTURE_STATUS muxingData(PROJECTIONTYPE projectionType, STEREO_MODE stereMode, const float fps, bool is360);
			FBCAPTURE_STATUS saveScreenShot(const void* texturePtr, const TCHAR* fullSavePath, bool is360);
			FBCAPTURE_STATUS startLiveStream(const TCHAR* streamUrl);
			GRAPHICS_CARD checkGPUManufacturer();
			void stopLiveStream();
			void resetResources();
			void setGraphicsDeviceD3D11(ID3D11Device* device);			
			void setGPUManufacturer(GRAPHICS_CARD gpuDevice);
			void setSoftwareEncoder();
			void setClientPcmAudioInput(bool enabled);
			bool isSoftwareEncoderEnabled() const { return softwareEncoder; }
			bool isClientPcmAudioInputEnabled() const { return clientPcmAudioInputEnabled; }
			vector<wstring> splitString(wstring& str);
			wstring generateLiveVideoFileWString();
		};
	}
}
