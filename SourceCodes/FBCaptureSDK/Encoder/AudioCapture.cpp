/****************************************************************************************************************

Filename	:	AudioCapture.cpp
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#include "AudioCapture.h"
#include <stdint.h>

namespace FBCapture {
  namespace Audio {

    IMMDeviceEnumerator* AudioCapture::mmDeviceEnumerator_ = {};
    IAudioCaptureClient* AudioCapture::outputAudioCaptureClient_ = {};
    IAudioClient* AudioCapture::outputAudioClient_ = {};
    IMMDevice* AudioCapture::mmOutputDevice_ = {};
    UINT32 AudioCapture::outputBlockAlign_ = 0;
    IAudioCaptureClient* AudioCapture::inputAudioCaptureClient_ = {};
    IAudioClient* AudioCapture::inputAudioClient_ = {};
    IMMDevice* AudioCapture::mmInputDevice_ = {};
    UINT32 AudioCapture::inputBlockAlign_ = 0;

    HMMIO AudioCapture::file_ = {};
    MMCKINFO AudioCapture::ckRIFF_ = {};
    MMCKINFO AudioCapture::ckData_ = {};
    MMCKINFO AudioCapture::ckFMT_ = {};

    wstring AudioCapture::wavFileName_;
    LPCWSTR AudioCapture::fileName_ = nullptr;
    BYTE* AudioCapture::outputData_ = nullptr;
    BYTE* AudioCapture::inputData_ = nullptr;
    WAVEFORMATEX* AudioCapture::outputPWFX_ = {};
    WAVEFORMATEX* AudioCapture::inputPWFX_ = {};
    size_t AudioCapture::captureIndex_ = 0;
    AudioBuffer* AudioCapture::buffer_ = nullptr;

    AudioCapture::AudioCapture() {
      useMicrophone_ = false;
      needToInitializeDevices = true;
      needToCloseCaptureFile = false;
      needToOpenCaptureFile = true;
      lastAudioCapture_ = std::chrono::steady_clock::now();

			// Riff chunk
			ckRIFF_.ckid = MAKEFOURCC('R', 'I', 'F', 'F');
			ckRIFF_.fccType = MAKEFOURCC('W', 'A', 'V', 'E');

			// fmt chunk
			ckFMT_.ckid = MAKEFOURCC('f', 'm', 't', ' ');

			// data chunk
			ckData_.ckid = MAKEFOURCC('d', 'a', 't', 'a');
    }

    AudioCapture::~AudioCapture() {
      closeCaptureFile();
      closeDevices();
    }

    bool AudioCapture::findAudioSource(bool isMic, LPCWSTR withIMMDeviceId) {
      bool ret = false;
      IMMDevice* pickDevice = {};

      HRESULT hr = mmDeviceEnumerator_->GetDevice(withIMMDeviceId, &pickDevice);
      if (SUCCEEDED(hr) && pickDevice != nullptr) {
	      if (isMic) {
		      mmInputDevice_ = pickDevice;
	      }
	      else {
		      mmOutputDevice_ = pickDevice;
	      }
	      ret = true;
      }

      if (!ret) {
	      SAFE_RELEASE(pickDevice);
      }
      return ret;
    }

    bool AudioCapture::findVRDeviceAudioSource(bool isMic, VRDeviceType vrDevice) {
      HRESULT hr = S_OK;
      bool ret = false;
      IMMDeviceCollection *pCollection = {};
      IMMDevice* pickDevice = {};
      UINT  count = 0;
      LPWSTR pwszID = {};
      IPropertyStore *pProps = {};
      PROPVARIANT varName = {};
      string AudioSourcesName;

      if (vrDevice == OCULUS_RIFT) {
	      AudioSourcesName = "(Rift Audio)";
      } else if (vrDevice == HTC_VIVE) {
	      AudioSourcesName = "(HTC Vive)";
      } else {
	      AudioSourcesName = "None";
      }

      hr = mmDeviceEnumerator_->EnumAudioEndpoints(isMic ? eCapture : eRender, DEVICE_STATE_ACTIVE, &pCollection);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to enumerate audio endpoints. [Error code] ", to_string(hr));
        ret = false;
        goto Exit;
      }

      pCollection->GetCount(&count);
      if (count == 0) {
        DEBUG_ERROR("No endpoints found");
        ret = false;
        goto Exit;
      }
     
      // Searching VR Audio Sources
      for (ULONG i = 0; i < count; i++) {
        SAFE_RELEASE(pickDevice);
        pCollection->Item(i, &pickDevice);
        CoTaskMemFree(pwszID);
        pickDevice->GetId(&pwszID);
        SAFE_RELEASE(pProps);
        pickDevice->OpenPropertyStore(STGM_READ, &pProps);
        PropVariantClear(&varName);
        PropVariantInit(&varName);
        pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        wstring wAudioSourceName(varName.pwszVal);

        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;

        string stringAudioSourceName = regex_replace(converter.to_bytes(wAudioSourceName), std::regex("[0-9]*- "), "");

        DEBUG_LOG_VAR("Audio device name: ", stringAudioSourceName.c_str());
        // Finding VR Headphone Sources
        if (stringAudioSourceName.find(AudioSourcesName) != std::string::npos) {
          DEBUG_LOG("Found VR audio device. We're using them as audio intput/output source");
          isMic ? mmDeviceEnumerator_->GetDevice(pwszID, &mmInputDevice_) : mmDeviceEnumerator_->GetDevice(pwszID, &mmOutputDevice_);
          ret = true;
          goto Exit;
        }
      }

    Exit:
      CoTaskMemFree(pwszID);
      pwszID = {};        
      PropVariantClear(&varName);      
      SAFE_RELEASE(pProps);
      SAFE_RELEASE(pickDevice);
      return ret;
    }

    static void logAudioWaveFormat(WAVEFORMATEX* format) {
      DEBUG_LOG_VAR("  size", to_string(format->cbSize));
      DEBUG_LOG_VAR("  nAvgBytesPerSec", to_string(format->nAvgBytesPerSec));
      DEBUG_LOG_VAR("  nBlockAlign", to_string(format->nBlockAlign));
      DEBUG_LOG_VAR("  nChannels", to_string(format->nChannels));
      DEBUG_LOG_VAR("  nSamplesPerSec", to_string(format->nSamplesPerSec));
      DEBUG_LOG_VAR("  wBitsPerSample", to_string(format->wBitsPerSample));
      DEBUG_LOG_VAR("  wFormatTag", to_string(format->wFormatTag));
    }

    static HRESULT isAudioWaveFormatSupported(IAudioClient* audioClient, WAVEFORMATEX* format) {
      WAVEFORMATEX* closestFormat = {};
      HRESULT hr = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, format, &closestFormat);
      if (closestFormat) {
        CoTaskMemFree(closestFormat);
        closestFormat = {};
      }
      return hr;
    }

    static void configureTargetAudioWaveFormat(WAVEFORMATEX* format, int nChannels, int nSamplesPerSec) {
      format->wFormatTag = WAVE_FORMAT_PCM;
      format->nChannels = nChannels;
      format->wBitsPerSample = 16;
      format->nSamplesPerSec = nSamplesPerSec;
      format->nBlockAlign = (format->wBitsPerSample * format->nChannels) / 8;
      format->nAvgBytesPerSec = (format->nSamplesPerSec * format->nBlockAlign);
      format->cbSize = 0;
    }

    const int firstTargetNumSamplesPerSec = 48000;
    const int secondTargetNumSamplesPerSec = 44100;

    static HRESULT findTargetOutputAudioWaveFormat(IAudioClient* audioClient, WAVEFORMATEX* format) {
      HRESULT hr = S_OK;
      configureTargetAudioWaveFormat(format, 2, firstTargetNumSamplesPerSec);
      hr = isAudioWaveFormatSupported(audioClient, format);
	  if (hr == S_OK) {
		return hr;
	  }

      configureTargetAudioWaveFormat(format, 2, secondTargetNumSamplesPerSec);
      hr = isAudioWaveFormatSupported(audioClient, format);
      return hr;
    }

    static HRESULT findTargetInputAudioWaveFormat(IAudioClient* audioClient, WAVEFORMATEX* format) {
      HRESULT hr = S_OK;
      configureTargetAudioWaveFormat(format, 2, firstTargetNumSamplesPerSec);
      hr = isAudioWaveFormatSupported(audioClient, format);
	  if (hr == S_OK) {
		return hr;
	  }

      configureTargetAudioWaveFormat(format, 2, secondTargetNumSamplesPerSec);
      hr = isAudioWaveFormatSupported(audioClient, format);
	  if (hr == S_OK) {
		return hr;
	  }

      configureTargetAudioWaveFormat(format, 1, firstTargetNumSamplesPerSec);
      hr = isAudioWaveFormatSupported(audioClient, format);
	  if (hr == S_OK) {
		return hr;
	  }

      configureTargetAudioWaveFormat(format, 1, secondTargetNumSamplesPerSec);
      hr = isAudioWaveFormatSupported(audioClient, format);
      return hr;
    }

    bool AudioCapture::startAudioclient(IAudioClient* audioClient, IAudioCaptureClient** captureClient, bool isInput) {

      HRESULT hr = S_OK;
      REFERENCE_TIME defaultDevicePeriod = 0;
      REFERENCE_TIME minimumDevicePeriod = 0;
      UINT32 numBufferFrames = 0;

      hr = audioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to get default device periodicity. [Error code] ", to_string(hr));
        return false;
      }

	  hr = audioClient->GetMixFormat(isInput ? &inputPWFX_ : &outputPWFX_);
	  if (FAILED(hr)) {
		DEBUG_ERROR_VAR("Failed to get default device format. [Error code] ", to_string(hr));
		return false;
	  }

      if (isInput) {
        hr = findTargetInputAudioWaveFormat(audioClient, inputPWFX_);
        if (hr != S_OK) {
          DEBUG_ERROR_VAR("Unable to find compatible audio format for input device. [Error code] ", to_string(hr));
          return false;
        }
      } else {
        hr = findTargetOutputAudioWaveFormat(audioClient, outputPWFX_);
        if (hr != S_OK) {
          DEBUG_ERROR_VAR("Unable to find compatible audio format for output device. [Error code] ", to_string(hr));
          return false;
        }
      }

      REFERENCE_TIME hnsBufferDuration = defaultDevicePeriod * 8; //TODO Pass in as configuration
	  hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, isInput ? 0 : AUDCLNT_STREAMFLAGS_LOOPBACK, hnsBufferDuration, 0, isInput ? inputPWFX_ : outputPWFX_, nullptr);
	  if (FAILED(hr)) {
		DEBUG_ERROR_VAR("Failed to initialize audio client. [Error code] ", to_string(hr));
		DEBUG_LOG_VAR("HnsBufferDuration requested: ", to_string(hnsBufferDuration));
        DEBUG_LOG("Format requested:");
        logAudioWaveFormat(isInput ? inputPWFX_ : outputPWFX_);
        return false;
      }

      hr = audioClient->GetBufferSize(&numBufferFrames);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to get buffer size for audio client. [Error code] ", to_string(hr));
        return false;
      }

	  if (!isInput && outputPWFX_->wBitsPerSample != 16) {
		DEBUG_ERROR("Unexpected output bit depth, expected 16-bit samples");
		return false;
	  } else if (isInput && inputPWFX_->wBitsPerSample != 16) {
		DEBUG_ERROR("Unexpected output bit depth, expected 16-bit samples");
		return false;
	  }

      if (!isInput && outputPWFX_->nChannels != 2) {
        DEBUG_ERROR("Unexpected output channel count, expected stereo");
        return false;
      } else if (isInput && inputPWFX_->nChannels != 1 && inputPWFX_->nChannels != 2) {
        DEBUG_ERROR("Unexpected input channel count, expected stereo or mono");
        return false;
      }

      IAudioCaptureClient* client = nullptr;

      hr = audioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&client);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to activate an IAudioCaptureClient. [Error code] ", to_string(hr));
        return false;
      }

      hr = audioClient->Start();
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to start audio client. [Error code] ", to_string(hr));
        return false;
      }

      *captureClient = client;

      return true;
    }

    FBCAPTURE_STATUS AudioCapture::openCaptureFile(const string& srcFile) {
			DEBUG_LOG("Start audio capture");

      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      wavFileName_ = stringTypeConversion.from_bytes(srcFile);

      openFile(wavFileName_.c_str(), &file_);

      if (!writeWaveHeader(outputPWFX_, &file_)) {
        closeCaptureFile();
        DEBUG_ERROR("Failed to write wave header");
        return FBCAPTURE_STATUS_WRITING_WAV_HEADER_FAILED;
      }

      needToOpenCaptureFile = false;

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AudioCapture::initializeDevices(bool useVRAudioEndpoint, VRDeviceType vrDevice, LPCWSTR useMicIMMDeviceId) {
      HRESULT hr = S_OK;
      useVRAudioResources_ = useVRAudioEndpoint;

      if (useVRAudioResources_) {
        useMicrophone_ = true;
      }

      buffer_ = new AudioBuffer();

      hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&mmDeviceEnumerator_
      );
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to create instance with CoCreateInstance. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED;
      }

			if (useVRAudioResources_) {
				if (!findVRDeviceAudioSource(false, vrDevice)) {
					DEBUG_LOG("Can't find VR output audio device. Just try using default audio output");
					hr = mmDeviceEnumerator_->GetDefaultAudioEndpoint(eRender, eConsole, &mmOutputDevice_);
					if (FAILED(hr)) {						
						DEBUG_ERROR_VAR("Failed to get default output audio endpoint with GetDefaultAudioEndpoint. [Error code] ", to_string(hr));		
						DEBUG_ERROR("We need to pick at least one output audio resource. Now it will capture without audio");
						return FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED;
					}
				}

				if (!findVRDeviceAudioSource(true, vrDevice)) {
					DEBUG_LOG("Can't find VR input audio devices. Just try using default audio input");
					hr = mmDeviceEnumerator_->GetDefaultAudioEndpoint(eCapture, eConsole, &mmInputDevice_);
					if (FAILED(hr)) {
						DEBUG_ERROR_VAR("Failed to get default input audio endpoint with GetDefaultAudioEndpoint. [Error code] ", to_string(hr));					
						DEBUG_ERROR("But we want to keep capturing output audio without input resource(mic)");
						useMicrophone_ = false;
					}
				}
			}	else if (!useVRAudioResources_) {
        // Use default audio endpoint when we don't want to use VR devices as audio source
        hr = mmDeviceEnumerator_->GetDefaultAudioEndpoint(eRender, eConsole, &mmOutputDevice_);
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed to get default audio endpoint with GetDefaultAudioEndpoint. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED;
        }
				if (useMicIMMDeviceId == nullptr) {
					useMicrophone_ = false;
				} else {
					useMicrophone_ = findAudioSource(true, useMicIMMDeviceId);
					if (!useMicrophone_) {
						DEBUG_LOG("Can't find target mic audio device. Skipping mic capture.");
					}
				}
      }

			if (mmOutputDevice_) {
				hr = mmOutputDevice_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&outputAudioClient_);
				if (FAILED(hr)) {
					DEBUG_ERROR_VAR("Failed to activate audio client", to_string(hr));
					return FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED;
				}
				if (!startAudioclient(outputAudioClient_, &outputAudioCaptureClient_, false)) {
					DEBUG_ERROR("Failed to start output audio client");
					return FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED;
				}
			}

      if (mmInputDevice_) {
        hr = mmInputDevice_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&inputAudioClient_);
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed to activate audio client. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED;
        }
        if (!startAudioclient(inputAudioClient_, &inputAudioCaptureClient_, true)) {
          DEBUG_ERROR("Failed to start input audio client");
          return FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED;
        }
      }

      buffer_->releaseBuffers();
      buffer_->initizalize(useMicrophone_ ? Input_Output_Device : Output_Device_Only);
      buffer_->initializeBuffer(BufferIndex_Headphones, outputPWFX_->nChannels);
      outputBlockAlign_ = outputPWFX_->nBlockAlign;

      if (useMicrophone_) {
        buffer_->initializeBuffer(BufferIndex_Microphone, inputPWFX_->nChannels);
        inputBlockAlign_ = inputPWFX_->nBlockAlign;
      }

      needToInitializeDevices = false;
      lastAudioCapture_ = std::chrono::steady_clock::now();

      return FBCAPTURE_STATUS_OK;
    }

    void AudioCapture::continueAudioCapture(bool enabledAudioCapture, bool enabledMicCapture) {
			
			if (outputAudioCaptureClient_ == nullptr) {
				return;
			}

      HRESULT hr = S_OK;
      UINT32 nNextPacketSize = 0;
      UINT32 outputNumFramesToRead = 0;
      UINT32 inputNumFramesToRead = 0;
      UINT64 outputPosition = 0;
      UINT64 inputPosition = 0;
      DWORD dwFlags = 0;
      UINT32 outputFramesRead = 0;
      UINT32 inputFramesRead = 0;
      UINT32 expectedOutputFramesToRead = 0;
      UINT32 expectedInputFramesToRead = 0;

      auto now = std::chrono::steady_clock::now();
      auto diffMilliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(now -
          lastAudioCapture_)
        .count();

      if (outputPWFX_) {
        expectedOutputFramesToRead = outputPWFX_->nSamplesPerSec / 1000 * (diffMilliseconds);
      }
      if (inputPWFX_) {
        expectedInputFramesToRead = inputPWFX_->nSamplesPerSec / 1000 * (diffMilliseconds);
      }

      lastAudioCapture_ = now;		

			// Write Output Data
			for (hr = outputAudioCaptureClient_->GetNextPacketSize(&nNextPacketSize);	SUCCEEDED(hr) &&
					 nNextPacketSize > 0;
					 hr = outputAudioCaptureClient_->GetNextPacketSize(&nNextPacketSize)) {

				//** audio data from speaker
				hr = outputAudioCaptureClient_->GetBuffer(&outputData_, &outputNumFramesToRead, &dwFlags, &outputPosition, nullptr);
				if (FAILED(hr)) {
					DEBUG_ERROR_VAR("Failed to get buffer from IAudioCaptureClient::GetBuffer", to_string(hr));
					break;
				}

				outputFramesRead += outputNumFramesToRead;

				if (AUDCLNT_BUFFERFLAGS_SILENT == dwFlags) {
					DEBUG_LOG("Silent status without any audio");
				}

				if (0 == outputNumFramesToRead) {
					DEBUG_LOG("No data can be read from Speaker--IAudioCaptureClient::GetBuffer");
				}

				hr = outputAudioCaptureClient_->ReleaseBuffer(outputNumFramesToRead);
				if (FAILED(hr)) {
					DEBUG_ERROR_VAR("Failed to release buffer. [Error code] ", to_string(hr));
					break;
				}

				buffer_->write(BufferIndex_Headphones, reinterpret_cast<int16_t*>(outputData_), outputNumFramesToRead); // PAS check this length!

				if (mmInputDevice_ && inputAudioCaptureClient_) {
					hr = inputAudioCaptureClient_->GetBuffer(&inputData_, &inputNumFramesToRead, &dwFlags, &inputPosition, nullptr);
					if (FAILED(hr)) {
						DEBUG_ERROR_VAR("Failed to get buffer from IAudioCaptureClient::GetBuffer", to_string(hr));
						break;
					}

					inputFramesRead += inputNumFramesToRead;

					if (AUDCLNT_BUFFERFLAGS_SILENT == dwFlags) {
						DEBUG_LOG("Silent status without any audio");
					}

					if (0 == inputNumFramesToRead) {
						DEBUG_LOG("No data can be read from Microphone--IAudioCaptureClient::GetBuffer");
					}

					hr = inputAudioCaptureClient_->ReleaseBuffer(inputNumFramesToRead);
					if (FAILED(hr)) {
						DEBUG_ERROR_VAR("Failed to release buffer. [Error code] ", to_string(hr));
						break;
					}
					buffer_->write(BufferIndex_Microphone, reinterpret_cast<int16_t*>(inputData_), inputNumFramesToRead); // PAS check this length!
				}

				const int16_t* finalOutput = nullptr;
				size_t length = 0;
				buffer_->getBuffer(&finalOutput, &length, enabledAudioCapture, enabledMicCapture);
				mmioWrite(file_, reinterpret_cast<PCCH>(finalOutput), length * sizeof(int16_t));
			}

      static std::vector<BYTE> silence;

      bool forcedSilence = false;
      if (outputFramesRead == 0) {
        DWORD state;
        mmOutputDevice_->GetState(&state);
        if (state != DEVICE_STATE_ACTIVE) {
          silence.resize(expectedOutputFramesToRead * outputPWFX_->nChannels * sizeof(int16_t), 0);
          buffer_->write(BufferIndex_Headphones, reinterpret_cast<int16_t*>(&silence[0]), expectedOutputFramesToRead);
          forcedSilence = true;
          enabledAudioCapture = false;
        }
      }

      if (useMicrophone_ && inputAudioCaptureClient_ && inputFramesRead == 0) {
        DWORD state;
        mmInputDevice_->GetState(&state);
        if (state != DEVICE_STATE_ACTIVE) {
          silence.resize(expectedInputFramesToRead * inputPWFX_->nChannels * sizeof(int16_t), 0);
          buffer_->write(BufferIndex_Microphone, reinterpret_cast<int16_t*>(&silence[0]), expectedInputFramesToRead);
          forcedSilence = true;
          enabledMicCapture = false;
        }
      }

      if (forcedSilence) {
        const int16_t* finalOutput = nullptr;
        size_t length = 0;
        buffer_->getBuffer(&finalOutput, &length, enabledAudioCapture, enabledMicCapture);
        mmioWrite(file_, reinterpret_cast<PCCH>(finalOutput), length * sizeof(int16_t));
      }
		}

    bool AudioCapture::openFile(LPCWSTR szFileName, HMMIO *phFile) {
      fileName_ = szFileName;
      *phFile = mmioOpen(
        // some flags cause mmioOpen write to this buffer
        // but not any that we're using
        const_cast<LPWSTR>(fileName_),
				nullptr,
        MMIO_CREATE | MMIO_WRITE | MMIO_EXCLUSIVE
      );

      if (nullptr == *phFile) {
        DEBUG_ERROR("Failed to create raw audio file.");
        return false;
      }

      return true;
    }

    bool AudioCapture::writeWaveHeader(WAVEFORMATEX* outputPWFX, HMMIO* phFile) {
      MMRESULT hr;

      hr = mmioCreateChunk(*phFile, &ckRIFF_, MMIO_CREATERIFF);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to create 'RIFF' chunk. [Error code] ", to_string(hr));
        return false;
      }

      // fmt chunk
      hr = mmioCreateChunk(*phFile, &ckFMT_, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to create 'fmt' chunk. [Error code] ", to_string(hr));
        return false;
      }

			// Write outputPWFX data to file
      LONG lBytesInWfx = sizeof(WAVEFORMATEX) + outputPWFX->cbSize;
      LONG lBytesWritten = mmioWrite(*phFile, reinterpret_cast<PCHAR>(outputPWFX), lBytesInWfx);
      if (lBytesWritten != lBytesInWfx) {
        DEBUG_ERROR("Failed to write WAVEFORMATEX data");
        return false;
      }

      // ascend from a fmt chunk
      hr = mmioAscend(*phFile, &ckFMT_, 0);
      if (MMSYSERR_NOERROR != hr)  {
        DEBUG_ERROR_VAR("Failed to ascend from 'fmt' chunk. [Error code] ", to_string(hr));
        return false;
      }

      // make a 'data' chunk and leave the data pointer there
      hr = mmioCreateChunk(*phFile, &ckData_, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to create 'data' chunk. [Error code] ", to_string(hr));
        return false;
      }

      return true;
    }

    FBCAPTURE_STATUS AudioCapture::closeCaptureFile() {

      if (file_) {

        auto cleanupHandler = [](HMMIO__* ptr) {
          mmioClose(ptr, 0);
          file_ = nullptr;
        };
        auto filePtr = std::unique_ptr<HMMIO__, decltype(cleanupHandler)>(file_, cleanupHandler);

        MMRESULT hr;
        hr = mmioAscend(filePtr.get(), &ckData_, 0);
        if (MMSYSERR_NOERROR != hr) {
          DEBUG_LOG_VAR("Failed to ascend out of a chunk in a RIFF file. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_RELEASING_WAV_FAILED;
        }

        hr = mmioAscend(filePtr.get(), &ckRIFF_, 0);
        if (MMSYSERR_NOERROR != hr) {
          DEBUG_LOG_VAR("Failed to ascend out of a chunk in a Data file. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_RELEASING_WAV_FAILED;
        }

        DEBUG_LOG("Released wave file resources");
      }

      needToCloseCaptureFile = false;
      needToOpenCaptureFile = true;

      return FBCAPTURE_STATUS_OK;
    }

    void AudioCapture::closeDevices() {
      if (inputPWFX_) {
        CoTaskMemFree(inputPWFX_);
        inputPWFX_ = {};
      }
      if (outputPWFX_) {
        CoTaskMemFree(outputPWFX_);
        outputPWFX_ = {};
      }
      if (outputAudioCaptureClient_) {
        outputAudioCaptureClient_->Release();
        outputAudioCaptureClient_ = {};
      }
      if (outputAudioClient_) {
        outputAudioClient_->Release();
        outputAudioClient_ = {};
      }
      if (mmOutputDevice_) {
        mmOutputDevice_->Release();
        mmOutputDevice_ = {};
      }
      if (inputAudioCaptureClient_) {
        inputAudioCaptureClient_->Release();
        inputAudioCaptureClient_ = {};
      }
      if (mmInputDevice_) {
        mmInputDevice_->Release();
        mmInputDevice_ = {};
      }
      if (buffer_) {
        delete buffer_;
        buffer_ = nullptr;
      }
      outputBlockAlign_ = 0;
      inputBlockAlign_ = 0;

      needToInitializeDevices = true;

      DEBUG_LOG("Released audio capture devices");
    }
  }
}
