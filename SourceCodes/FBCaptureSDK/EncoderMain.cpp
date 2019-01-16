/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "EncoderMain.h"

namespace FBCapture {
  namespace Common {

    // Capture SDK Version Update String
    const string sdkVersion = "2.35";

    EncoderMain::EncoderMain() {
      DEBUG_LOG_VAR("Capture SDK Version: ", sdkVersion);
    }

    EncoderMain::~EncoderMain() {
    }

    vector<wstring> EncoderMain::splitString(wstring& str) {
      vector<wstring> stringArr;
      wstringstream stringStream(str); // Turn the string into a stream.
      wstring tok;

      while (getline(stringStream, tok, L'.')) {
        stringArr.push_back(tok);
      }

      return stringArr;
    }

    FBCAPTURE_STATUS EncoderMain::initEncoderComponents() {

      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      if (checkGPUManufacturer() == GRAPHICS_CARD::NVIDIA && this->nvEncoder == nullptr) {
        this->nvEncoder = std::make_unique<NVEncoder>(this->device_);
      }

      if (checkGPUManufacturer() == GRAPHICS_CARD::AMD && this->amdEncoder == nullptr) {
        this->amdEncoder = std::make_unique<AMDEncoder>(this->device_);
      }
		
      if (this->flvMuxer == nullptr) {
        this->flvMuxer = std::make_unique<FLVMuxer>();
      }

      if (this->mp4Muxer == nullptr) {
        this->mp4Muxer = std::make_unique<MP4Muxer>();
      }

      if (this->audioDeviceCapture == nullptr) {
        this->audioDeviceCapture = std::make_unique<AudioDeviceCapture>();
      }

      if (this->audioCustomRawDataCapture == nullptr) {
        this->audioCustomRawDataCapture = std::make_unique<AudioCustomRawDataCapture>();
      }

      if (this->audioEncoder == nullptr) {
        this->audioEncoder = std::make_unique<AudioEncoder>();
      }

      if (this->rtmp == nullptr) {
        this->rtmp = std::make_unique<LibRTMP>();
      }

      // Using "%AppData%" folder for saving sliced live video files
      LPWSTR wszPath = nullptr;
      HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &wszPath);
      if (SUCCEEDED(hr)) {
        this->liveFolder = wszPath;
        this->liveFolder += L"\\FBEncoder\\";
        if (!CreateDirectory(this->liveFolder.c_str(), nullptr)) {
          if (ERROR_ALREADY_EXISTS == GetLastError()) {
            DEBUG_LOG("Output folder already existed");
          }
        }
      } else {
        this->liveFolder.clear();  // Just use root folder when it failed to create folder
      }

      return status;
    }

    FBCAPTURE_STATUS EncoderMain::checkGraphicsCardCapability() {

      if (checkGPUManufacturer() == GRAPHICS_CARD::UNSUPPORTED_DEVICE) {
        DEBUG_ERROR("Unsupported graphics card. It will use software encoder.");
        return FBCAPTURE_STATUS_UNSUPPORTED_GRAPHICS_CARD;
      }

      return FBCAPTURE_STATUS_OK;
    }

    GRAPHICS_CARD EncoderMain::checkGPUManufacturer() {
      if (this->amdDevice) {
        return GRAPHICS_CARD::AMD;
      } else if (this->nvidiaDevice) {
        return GRAPHICS_CARD::NVIDIA;
      } else {
        return GRAPHICS_CARD::UNSUPPORTED_DEVICE;
      }
    }

    FBCAPTURE_STATUS EncoderMain::releaseEncodeResources() {

      this->needEncodingSessionInit = true;

      // We want to destroy nvidia encoder only because AMF doesn't open encoding session on AMF init
      if (checkGPUManufacturer() == GRAPHICS_CARD::NVIDIA && this->nvEncoder) {
        return this->nvEncoder->releaseEncodingResources();
      }

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS EncoderMain::dummyEncodingSession() {

      this->needEncodingSessionInit = true;

      // Only nVidia driver requires real input buffer to destroy encoder clearly.
      // Otherwise, nVidia driver keeps holding endoer session created before, even though we called "NvEncDestroyEncoder" function.
      if (checkGPUManufacturer() == GRAPHICS_CARD::NVIDIA && this->nvEncoder) {
        return this->nvEncoder->dummyTextureEncoding();
      }

      return FBCAPTURE_STATUS_OK;
    }


    FBCAPTURE_STATUS EncoderMain::initSessionANDcheckDriverCapability() {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

			if (isSoftwareEncoderEnabled()) {
				if (this->swEncoder == nullptr) {
					this->swEncoder = std::make_unique<OpenH264Encoder>(this->device_);
				}
				status = this->swEncoder->initEncodingSession();
				if (status != FBCAPTURE_STATUS_OK) {
					return status;
				}
			} else if (checkGPUManufacturer() == GRAPHICS_CARD::NVIDIA && this->nvEncoder) {
				status = this->nvEncoder->initEncodingSession();
				if (status != FBCAPTURE_STATUS_OK) {
					return status;
				}				
      } else if (checkGPUManufacturer() == GRAPHICS_CARD::AMD && this->amdEncoder) {
        status = this->amdEncoder->initEncodingSession();
        if (status != FBCAPTURE_STATUS_OK) {
          return status;
        }
      }

      this->needEncodingSessionInit = false;

      return status;
    }

    wstring EncoderMain::generateLiveVideoFileWString() {
      wstring liveVideoFile = this->prevVideoH264;
      liveVideoFile.erase(this->prevVideoH264.length() - this->h264Extension.length(), this->h264Extension.length());
      liveVideoFile += this->flvExtension;
      return liveVideoFile;
    }

    FBCAPTURE_STATUS EncoderMain::startEncoding(const void* texturePtr, const TCHAR* fullSavePath, bool isLive, int bitrate, int fps, bool needFlipping) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      if (this->stopEncProcess)
        return status;

      this->isLive = isLive;

      if (needEncodingSessionInit) {
        status = initSessionANDcheckDriverCapability();
        if (status != FBCAPTURE_STATUS_OK) {
          return status;
        }
      }

      if (!this->initiatedUnixTime) {
        this->initiatedUnixTime = true;
        this->unixTime = std::time(nullptr);  // Using unix time for live video files' identity
      }

      if (!this->isLive && this->updateInputName) {  // VOD mode
        this->updateInputName = false;
        this->videoH264 = fullSavePath;
        if (this->videoH264.find(this->mp4Extension) != string::npos) {  // Convert file name to h264 when input name is mp4 format
          this->videoH264.replace(this->videoH264.find(this->mp4Extension), this->mp4Extension.length(), this->h264Extension);
        }
      } else if (this->isLive) {  // Live mode
        this->videoH264 = this->liveFolder + to_wstring(this->unixTime) + this->h264Extension;
      }

      // Encoding with render texture
			if (texturePtr == nullptr) {			
				DEBUG_LOG("It's invalid texture pointer: null");
			} else if (isSoftwareEncoderEnabled()) {
				status = this->swEncoder->encodeProcess(texturePtr, this->videoH264, bitrate, fps, needFlipping);
				if (status != FBCAPTURE_STATUS_OK) {
					this->stopEncProcess = true;
					return status;
				}
			} else if (checkGPUManufacturer() == GRAPHICS_CARD::NVIDIA) {
				status = this->nvEncoder->encodeProcess(texturePtr, this->videoH264, bitrate, fps, needFlipping);
				if (status != FBCAPTURE_STATUS_OK) {
					this->stopEncProcess = true;
					return status;
				}
			} else if (checkGPUManufacturer() == GRAPHICS_CARD::AMD) {
				status = this->amdEncoder->encodeProcess(texturePtr, this->videoH264, bitrate, fps, needFlipping);
				if (status != FBCAPTURE_STATUS_OK) {
					this->stopEncProcess = true;
					return status;
				}
			}

      return status;
    }

    FBCAPTURE_STATUS EncoderMain::audioDeviceCaptureEncoding(bool useVRAudioEndpoint, bool enabledAudioCapture, bool enabledMicCapture,
                                                VRDeviceType vrDevice, LPCWSTR useMicIMMDeviceId) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      if (this->stopEncProcess || this->videoH264.empty()) {
        return status;
      }

      if (this->audioDeviceCapture->needToInitializeResources) {
        status = this->audioDeviceCapture->initializeCaptureResources(useVRAudioEndpoint, vrDevice, useMicIMMDeviceId);
        if (status == FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED) {
          this->noAvailableAudioDevice = true;
          return status;
        } else if (status != FBCAPTURE_STATUS_OK) {
          this->stopEncProcess = true;
          return status;
        }
      }

      if (this->audioDeviceCapture->needToOpenCaptureFile) {
        if (!this->isLive) {  // VOD mode
          wstring videoPath = this->videoH264;
          videoPath.erase(this->videoH264.length() - this->h264Extension.length(), this->h264Extension.length());

          wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
          string audioPath = stringTypeConversion.to_bytes(videoPath);

          this->audioWAV = audioPath + this->wavExtension;
          this->audioAAC = audioPath + this->aacExtension;
        } else {  // Live mode
          wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
          string folderPath = stringTypeConversion.to_bytes(this->liveFolder);
          this->audioWAV = folderPath + to_string(this->unixTime) + this->wavExtension;
          this->audioAAC = folderPath + to_string(this->unixTime) + this->aacExtension;
        }

        status = this->audioDeviceCapture->openCaptureFile(this->audioWAV);
        if (status != FBCAPTURE_STATUS_OK) {
          this->stopEncProcess = true;
          return status;
        }
      }

      if (!this->audioDeviceCapture->needToCloseCaptureFile) {
        this->audioDeviceCapture->continueAudioCapture(enabledAudioCapture, enabledMicCapture);
      } else {
        status = this->audioDeviceCapture->closeCaptureFile();
        if (status != FBCAPTURE_STATUS_OK) {
          this->stopEncProcess = true;
          return status;
        }
      }

      return status;
    }

    FBCAPTURE_STATUS EncoderMain::audioRawDataEncoding(float* audioData, uint32_t channel, uint32_t sampleRate, uint32_t bufferSize) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      if (this->stopEncProcess || this->videoH264.empty()) {
        return status;
      }

      if (this->audioCustomRawDataCapture->needToOpenCaptureFile) {
        if (!this->isLive) {  // VOD mode
          wstring videoPath = this->videoH264;
          videoPath.erase(this->videoH264.length() - this->h264Extension.length(), this->h264Extension.length());

          wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
          string audioPath = stringTypeConversion.to_bytes(videoPath);

          this->audioWAV = audioPath + this->wavExtension;
          this->audioAAC = audioPath + this->aacExtension;
        } else {  // Live mode
          wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
          string folderPath = stringTypeConversion.to_bytes(this->liveFolder);
          this->audioWAV = folderPath + to_string(this->unixTime) + this->wavExtension;
          this->audioAAC = folderPath + to_string(this->unixTime) + this->aacExtension;
        }

        status = this->audioCustomRawDataCapture->openCaptureFile(this->audioWAV, sampleRate, channel);
        if (status != FBCAPTURE_STATUS_OK) {
          this->stopEncProcess = true;
          return status;
        }
      }

      if (!this->audioCustomRawDataCapture->needToCloseCaptureFile) {
        this->audioCustomRawDataCapture->continueAudioCapture(audioData, bufferSize);
      } else {
        status = this->audioCustomRawDataCapture->closeCaptureFile();
        if (status != FBCAPTURE_STATUS_OK) {
          this->stopEncProcess = true;
          return status;
        }
      }

      return status;
    }

    FBCAPTURE_STATUS EncoderMain::stopEncoding(bool forceStop) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      if (this->stopEncProcess)  // Only we can flush buffers after encoding starts
        return status;

      this->updateInputName = true;

      if (this->updateInputName) {  // Update unix time for next frame video and audio
        this->unixTime = std::time(nullptr);
        this->updateInputName = !this->updateInputName;
      }

      // Store previous file name to use in muxing
      this->prevAudioWAV = this->audioWAV;
      this->prevAudioAAC = this->audioAAC;
      this->prevVideoH264 = this->videoH264;

      // Update file names for next slice
      if (this->isLive) {
        wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
        string folderPath = stringTypeConversion.to_bytes(this->liveFolder);
        this->audioWAV = folderPath + to_string(this->unixTime) + this->wavExtension;
        this->audioAAC = folderPath + to_string(this->unixTime) + this->aacExtension;
        this->videoH264 = this->liveFolder + to_wstring(this->unixTime) + this->h264Extension;
      }

      // Flushing inputs
			if (isSoftwareEncoderEnabled()) {
				status = this->swEncoder->flushInputTextures();
				if (status != FBCAPTURE_STATUS_OK) {
					this->stopEncProcess = true;
					return status;
				}
			} else if (checkGPUManufacturer() == GRAPHICS_CARD::NVIDIA) {
				status = this->nvEncoder->flushInputTextures();
        if (status != FBCAPTURE_STATUS_OK) {
          this->stopEncProcess = true;
          return status;
        }
      } else if (checkGPUManufacturer() == GRAPHICS_CARD::AMD) {
        status = this->amdEncoder->flushInputTextures();
        if (status != FBCAPTURE_STATUS_OK) {
          this->stopEncProcess = true;
          return status;
        }
      }

      this->needEncodingSessionInit = true;
      this->movingToMuxingStage = true;

      if (isClientPcmAudioInputEnabled()) {
        if (!this->audioCustomRawDataCapture->needToInitializeResources) {
          this->audioCustomRawDataCapture->needToCloseCaptureFile = true;
          this->stopEncodingSession = forceStop;
          if (this->stopEncodingSession) {
            this->audioCustomRawDataCapture->closeCaptureFile();
          }
        }
      } else {
        if (!this->audioDeviceCapture->needToInitializeResources) {
          this->audioDeviceCapture->needToCloseCaptureFile = true;
          this->stopEncodingSession = forceStop;
          if (this->stopEncodingSession) {
            this->audioDeviceCapture->closeCaptureFile();
          }
        }
      }

      return status;
    }

    FBCAPTURE_STATUS EncoderMain::muxingData(PROJECTIONTYPE projectionType, STEREO_MODE stereMode, const float fps, bool is360) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
      if (!this->movingToMuxingStage || this->stopEncProcess)
        return status;

      while (true) {
        if (this->stopEncProcess)
          break;

        Sleep(1);

        if ((!isClientPcmAudioInputEnabled() && this->audioDeviceCapture->needToCloseCaptureFile) ||
            (isClientPcmAudioInputEnabled() && this->audioCustomRawDataCapture->needToCloseCaptureFile))
          continue;

        // Transcoding wav to aac when we get audio input
        if (this->noAvailableAudioDevice) {
          this->prevAudioAAC = {};  // Generate video only on muxing layer when no available audio device is attached
        } else {
          status = this->audioEncoder->continueAudioTranscoding(this->prevAudioWAV, this->prevAudioAAC);
          if (status == FBCAPTURE_STATUS_MF_SOURCE_READER_CREATION_FAILED) {
            this->prevAudioAAC = {};  // Generate video only on muxing layer when we fail to create aac audio file
          } else if (status != FBCAPTURE_STATUS_OK) {
            this->stopEncProcess = true;
            return status;
          }
        }

        // Muxing
        if (this->isLive) {
          status = this->flvMuxer->muxingMedia(this->prevVideoH264, this->prevAudioAAC, fps);
          if (status != FBCAPTURE_STATUS_OK) {
            this->stopEncProcess = true;
            return status;
          }
        } else {
          status = this->mp4Muxer->muxingMedia(this->prevVideoH264, this->prevAudioAAC, projectionType, stereMode, fps, is360);
          if (status != FBCAPTURE_STATUS_OK) {
            this->stopEncProcess = true;
            return status;
          }
        }

        // Clean up audio resouces in same thread
        if (this->stopEncodingSession) {
          this->stopEncodingSession = false; // Reset for next encoding session
          if (!isClientPcmAudioInputEnabled() && this->audioDeviceCapture) {
            this->audioDeviceCapture->releaseCaptureResources();
          } else {
            this->audioCustomRawDataCapture->releaseCaptureResources();
          }

          if (this->audioEncoder) {
            this->audioEncoder->shutdown();
          }
        }

        break;
      }
      this->movingToMuxingStage = false;
      this->isMuxed = true;
      this->stopEncProcess = false;

      return status;
    }

    FBCAPTURE_STATUS EncoderMain::saveScreenShot(const void* texturePtr, const TCHAR* fullSavePath, bool is360) {
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

			if (texturePtr == nullptr) {
				DEBUG_ERROR("Invalid render texture pointer(null)");
				return FBCAPTURE_STATUS_INVALID_TEXTURE_POINTER;
			}

			if (_tcslen(fullSavePath) == 0) {  //No input file path for screenshot
				DEBUG_ERROR("Didn't put screenshot file name");
				return FBCAPTURE_STATUS_NO_INPUT_FILE;
			}

			screenShotEncoder = std::make_unique<ScreenShotEncoder>(this->device_);
			status = screenShotEncoder->saveScreenShot(texturePtr, fullSavePath, is360);
			if (status != FBCAPTURE_STATUS_OK) {
				return status;
			}

			DEBUG_LOG("Finished screenshot successfully");

			return status;
    }

    FBCAPTURE_STATUS EncoderMain::startLiveStream(const TCHAR* streamUrl) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      if (!this->isMuxed || this->stopEncProcess || !this->isLive)
        return status;

      if (this->rtmp && this->isLive) {
        wstring liveVideoFile = generateLiveVideoFileWString();
        if (!liveVideoFile.empty() && (status = this->rtmp->connectRTMPWithFlv(streamUrl, liveVideoFile.c_str())) != FBCAPTURE_STATUS_OK) {
          return status;
        }
      } else {
        DEBUG_LOG("Need to enable live streaming option in Unity script");
      }

      return status;
    }

    void EncoderMain::stopLiveStream() {
      if (this->rtmp) {
        this->rtmp->close();
      }

      // Remove any potential lingering files due to failures
      remove(this->audioWAV.c_str());
      remove(this->audioAAC.c_str());
      _wremove(this->videoH264.c_str());
      remove(this->prevAudioWAV.c_str());
      remove(this->prevAudioAAC.c_str());
      _wremove(this->prevVideoH264.c_str());
      if (!this->prevVideoH264.empty()) {
        wstring liveVideoFile = generateLiveVideoFileWString();
        _wremove(liveVideoFile.c_str());
      }
    }

    void EncoderMain::resetResources() {
      this->initiatedUnixTime = false;
      this->stopEncProcess = false;
      this->isLive = false;
      this->unixTime = 0;
      this->updateInputName = true;
      this->isMuxed = false;
      this->stopEncodingSession = false;
      this->needEncodingSessionInit = true;
      this->movingToMuxingStage = false;
      this->noAvailableAudioDevice = false;
			this->softwareEncoder = false;
    }

    void EncoderMain::setGraphicsDeviceD3D11(ID3D11Device* device) {
      this->device_ = (ID3D11Device*)device;
    }

		void EncoderMain::setGPUManufacturer(GRAPHICS_CARD gpuDevice) {
			if (gpuDevice == GRAPHICS_CARD::NVIDIA) {
				this->nvidiaDevice = true;
			} else if (gpuDevice == GRAPHICS_CARD::AMD) {
				this->amdDevice = true;
			} 
		}
		void EncoderMain::setSoftwareEncoder() {			
			// Need to disable other GPU device when SW encoder is enabled
			this->nvidiaDevice = false;
			this->amdDevice = false;			
			this->softwareEncoder = true;
		}
	
		void EncoderMain::setClientPcmAudioInput(bool enabled) {
			this->clientPcmAudioInputEnabled = true;
		}

  }
}
