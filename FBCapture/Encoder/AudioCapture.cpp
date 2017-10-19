/****************************************************************************************************************

Filename	:	AudioCapture.cpp
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#include "AudioCapture.h"

enum BufferIndex {
  BufferIndex_Headphones = 0,
  BufferIndex_Microphone,
  BufferIndex_Max
};

enum BufferNums {
  No_Device = 0,
  Output_Device_Only,
  Input_Output_Device
};

namespace FBCapture {
  namespace Audio {

    IMMDeviceEnumerator* AudioCapture::mmDeviceEnumerator_ = NULL;

    IAudioCaptureClient* AudioCapture::outputAudioCaptureClient_ = NULL;
    IAudioClient* AudioCapture::outputAudioClient_ = NULL;
    IMMDevice* AudioCapture::mmOutputDevice_ = NULL;
    UINT32 AudioCapture::outputBlockAlign_ = 0;

    IAudioCaptureClient* AudioCapture::inputAudioCaptureClient_ = NULL;
    IAudioClient* AudioCapture::inputAudioClient_ = NULL;
    IMMDevice* AudioCapture::mmInputDevice_ = NULL;
    UINT32 AudioCapture::inputBlockAlign_ = 0;

    HMMIO AudioCapture::file_ = NULL;
    MMCKINFO AudioCapture::ckRIFF_ = { 0 };
    MMCKINFO AudioCapture::ckData_ = { 0 };
    MMCKINFO AudioCapture::chunk_;

    wstring AudioCapture::wavFileName_;
    LPCWSTR AudioCapture::fileName_ = NULL;
    LONG AudioCapture::outputBytesToWrite_;
    LONG AudioCapture::inputBytesToWrite_;
    BYTE* AudioCapture::outputData_;
    BYTE* AudioCapture::inputData_;
    WAVEFORMATEX* AudioCapture::outputPWFX_;
    WAVEFORMATEX* AudioCapture::inputPWFX_;
    size_t AudioCapture::captureIndex_ = 0;
    AudioBuffer* AudioCapture::buffer_ = NULL;

    AudioCapture::AudioCapture() {
      buffer_ = new AudioBuffer();
      useMicrophone_ = false;
      mmOutputDevice_ = NULL;
      mmInputDevice_ = NULL;
    }

    AudioCapture::~AudioCapture() {
      if (file_) {
        closeWavefile(file_, &ckData_, &ckRIFF_);
        mmioClose(file_, 0);
      }

      if (buffer_) {
        delete buffer_;
        buffer_ = NULL;
      }
    }


    FBCAPTURE_STATUS AudioCapture::stopAudioCapture() {
      MMRESULT hr = MMSYSERR_NOERROR;

      if (file_) {
        if (!closeWavefile(file_, &ckData_, &ckRIFF_)) {
          return FBCAPTURE_STATUS_RELEASING_WAV_FAILED;
        }

        hr = mmioClose(file_, 0);
        file_ = NULL;
        if (MMSYSERR_NOERROR != hr) {
          DEBUG_ERROR_VAR("Failed to close file was opened by using mmioOpen. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_RELEASING_WAV_FAILED;
        }
      }

      return FBCAPTURE_STATUS_OK;
    }

    bool AudioCapture::findRiftAudioSource(bool useRiftAudioEndpoint, bool isMic) {
      HRESULT hr = S_OK;
      bool ret = false;
      IMMDeviceCollection *pCollection = NULL;

      const string riftAudioSourceStr = "(Rift Audio)";

      hr = mmDeviceEnumerator_->EnumAudioEndpoints(isMic ? eCapture : eRender, DEVICE_STATE_ACTIVE, &pCollection);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to enumerate audio endpoints. [Error code] ", to_string(hr));
        ret = false;
        goto Exit;
      }

      UINT  count;
      LPWSTR pwszID = NULL;
      IPropertyStore *pProps = NULL;
      pCollection->GetCount(&count);
      if (count == 0) {
        DEBUG_ERROR("No endpoints found");
        ret = false;
        goto Exit;
      }

      IMMDevice* pickDevice;
      PROPVARIANT varName;
      // Searching Rift Audio Source
      for (ULONG i = 0; i < count; i++) {
        pCollection->Item(i, &pickDevice);
        pickDevice->GetId(&pwszID);
        pickDevice->OpenPropertyStore(STGM_READ, &pProps);
        PropVariantInit(&varName);
        pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        wstring wAudioSourceName(varName.pwszVal);

        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;

        string stringAudioSourceName = regex_replace(converter.to_bytes(wAudioSourceName), std::regex("[0-9]*- "), "");

        DEBUG_LOG_VAR("Audio device name: ", stringAudioSourceName.c_str());
        // Finding Rift Headphone Source
        if (stringAudioSourceName.find(riftAudioSourceStr) != std::string::npos) {
          DEBUG_LOG("Found Rift headphone. We're using Rift headphone as audio intput/output source");
          isMic ? mmDeviceEnumerator_->GetDevice(pwszID, &mmInputDevice_) : mmDeviceEnumerator_->GetDevice(pwszID, &mmOutputDevice_);
          ret = true;
          goto Exit;
        }
      }

    Exit:
      CoTaskMemFree(pwszID);
      pwszID = NULL;
      PropVariantClear(&varName);
      SAFE_RELEASE(pProps);
      SAFE_RELEASE(pickDevice);
      return ret;
    }

    bool AudioCapture::startAudioclient(IAudioClient* audioClient, IAudioCaptureClient** captureClient, bool isInput) {
      HRESULT hr = S_OK;
      WAVEFORMATEX* waveFormatEX = NULL;

      REFERENCE_TIME devicePeriod;
      hr = audioClient->GetDevicePeriod(&devicePeriod, NULL);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to get default device periodicity. [Error code] ", to_string(hr));
        return false;
      }

      hr = audioClient->GetMixFormat(&waveFormatEX);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to get default device format. [Error code] ", to_string(hr));
        return false;
      }

      if (isInput) {
        inputPWFX_ = waveFormatEX;
      } else {
        outputPWFX_ = waveFormatEX;
      }

      hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, isInput ? 0 : AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, waveFormatEX, NULL);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to initialize audio client. [Error code] ", to_string(hr));
        return false;
      }

      if (waveFormatEX->wBitsPerSample != 32) {
        DEBUG_ERROR("Unexpected output bit depth, expected 32-bit samples");
        return false;
      }

      if (!isInput && waveFormatEX->nChannels != 2) {
        DEBUG_ERROR("Unexpected output channel count, expected stereo");
        return false;
      } else if (isInput && waveFormatEX->nChannels != 1) {
        DEBUG_ERROR("Unexpected input channel count, expected mono");
        return false;
      }

      IAudioCaptureClient* client = NULL;

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

    FBCAPTURE_STATUS AudioCapture::initializeAudio(const string& srcFile, bool useRiftAudioEndpoint) {
      HRESULT hr = S_OK;
      useRiftAudioResources_ = useRiftAudioEndpoint;

      if (useRiftAudioResources_) {
        useMicrophone_ = true;
      }

      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      wavFileName_ = stringTypeConversion.from_bytes(srcFile);

      hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&mmDeviceEnumerator_
      );
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to create instance with CoCreateInstance. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED;
      }

      if (useRiftAudioResources_ && !findRiftAudioSource(useRiftAudioResources_, true) || !findRiftAudioSource(useRiftAudioResources_, false)) {
        DEBUG_LOG("Can't find Rift audio device. Just try to use default audio device");
        hr = mmDeviceEnumerator_->GetDefaultAudioEndpoint(eRender, eConsole, &mmOutputDevice_);
        useMicrophone_ = false;
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed to get default audio endpoint with GetDefaultAudioEndpoint. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED;
        }
      } else if (!useRiftAudioResources_) {  // Use default audio endpoint when we don't want to use rift as audio source
        hr = mmDeviceEnumerator_->GetDefaultAudioEndpoint(eRender, eConsole, &mmOutputDevice_);
        useMicrophone_ = false;
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed to get default audio endpoint with GetDefaultAudioEndpoint. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED;
        }
      }

      openFile(wavFileName_.c_str(), &file_);

      hr = mmOutputDevice_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&outputAudioClient_);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed to activate audio client", to_string(hr));
        return FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED;
      }

      if (!startAudioclient(outputAudioClient_, &outputAudioCaptureClient_, false)) {
        DEBUG_ERROR("Failed to start output audio client");
        return FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED;
      }

      if (useMicrophone_) {
        hr = mmInputDevice_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&inputAudioClient_);
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed to activate audio client. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED;
        }
        if (!startAudioclient(inputAudioClient_, &inputAudioCaptureClient_, true)) {
          DEBUG_ERROR("Failed to start input audio client");
          return FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED;
        }
      }

      if (!writeWaveHeader(file_, outputPWFX_, &ckRIFF_, &ckData_)) {
        DEBUG_ERROR("Failed to write wave header");
        return FBCAPTURE_STATUS_WRITTING_WAV_HEADER_FAILED;
      }

      buffer_->initizalize(useMicrophone_ ? Input_Output_Device : Output_Device_Only);
      buffer_->initializeBuffer(BufferIndex_Headphones, outputPWFX_->nChannels);
      outputBlockAlign_ = outputPWFX_->nBlockAlign;

      if (useMicrophone_) {
        buffer_->initializeBuffer(BufferIndex_Microphone, inputPWFX_->nChannels);
        inputBlockAlign_ = inputPWFX_->nBlockAlign;
      }

      return FBCAPTURE_STATUS_OK;
    }


    void AudioCapture::startAudioCapture(bool silenceMode) {
      HRESULT hr = S_OK;
      UINT32 nNextPacketSize;
      UINT32 outputNumFramesToRead;
      UINT32 inputNumFramesToRead;
      UINT64 outputPosition;
      UINT64 inputPosition;
      DWORD dwFlags;

      // Write Output Data
      for (hr = outputAudioCaptureClient_->GetNextPacketSize(&nNextPacketSize);	SUCCEEDED(hr) &&
           nNextPacketSize > 0;
           hr = outputAudioCaptureClient_->GetNextPacketSize(&nNextPacketSize)) {

        //** audio data from speaker
        hr = outputAudioCaptureClient_->GetBuffer(&outputData_, &outputNumFramesToRead, &dwFlags, &outputPosition, NULL);
        if (FAILED(hr)) {
          DEBUG_LOG("Failed to get buffer from IAudioCaptureClient::GetBuffer");
          break;
        }

        if (AUDCLNT_BUFFERFLAGS_SILENT == dwFlags) {
          DEBUG_LOG("Silent status without any audio");
        }

        outputBytesToWrite_ = outputNumFramesToRead * outputBlockAlign_;

        if (0 == outputNumFramesToRead) {
          DEBUG_LOG("No data can be read from Speaker--IAudioCaptureClient::GetBuffer");
          break;
        }

        hr = outputAudioCaptureClient_->ReleaseBuffer(outputNumFramesToRead);
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed to release buffer. [Error code] ", to_string(hr));
          break;
        }

        if (useRiftAudioResources_ && inputAudioCaptureClient_) {
          hr = inputAudioCaptureClient_->GetBuffer(&inputData_, &inputNumFramesToRead, &dwFlags, &inputPosition, NULL);
          if (FAILED(hr)) {
            DEBUG_LOG("Failed to get buffer from IAudioCaptureClient::GetBuffer");
            break;
          }

          if (AUDCLNT_BUFFERFLAGS_SILENT == dwFlags) {
            DEBUG_LOG("Silent status without any audio");
          }

          inputBytesToWrite_ = inputNumFramesToRead * inputBlockAlign_;

          if (0 == inputNumFramesToRead) {
            DEBUG_LOG("No data can be read from Microphone--IAudioCaptureClient::GetBuffer");
            break;
          }

          hr = inputAudioCaptureClient_->ReleaseBuffer(inputNumFramesToRead);
          if (FAILED(hr)) {
            DEBUG_ERROR_VAR("Failed to release buffer. [Error code] ", to_string(hr));
            break;
          }
          buffer_->write(BufferIndex_Microphone, reinterpret_cast<float*>(inputData_), inputNumFramesToRead); // PAS check this length!
        }

        buffer_->write(BufferIndex_Headphones, reinterpret_cast<float*>(outputData_), outputNumFramesToRead); // PAS check this length!

        const float* finalOutput = nullptr;
        size_t length = 0;
        buffer_->getBuffer(&finalOutput, &length, silenceMode);
        mmioWrite(file_, reinterpret_cast<PCCH>(finalOutput), length * sizeof(float));
      }
    }


    bool AudioCapture::openFile(LPCWSTR szFileName, HMMIO *phFile) {
      fileName_ = szFileName;
      *phFile = mmioOpen(
        // some flags cause mmioOpen write to this buffer
        // but not any that we're using
        const_cast<LPWSTR>(fileName_),
        NULL,
        MMIO_READWRITE | MMIO_CREATE | MMIO_EXCLUSIVE
      );

      if (NULL == *phFile) {
        DEBUG_ERROR("Failed to create audio file");
        return false;
      }

      return true;
    }

    bool AudioCapture::writeWaveHeader(HMMIO file, LPCWAVEFORMATEX outputPWFX, MMCKINFO *pckRiff, MMCKINFO *pckData) {
      MMRESULT hr;

      pckRiff->ckid = MAKEFOURCC('R', 'I', 'F', 'F');
      pckRiff->fccType = MAKEFOURCC('W', 'A', 'V', 'E');
      pckRiff->cksize = 0;

      hr = mmioCreateChunk(file, pckRiff, MMIO_CREATERIFF);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to create 'RIFF' chunk. [Error code] ", to_string(hr));
        return false;
      }

      //
      chunk_.ckid = MAKEFOURCC('f', 'm', 't', ' ');
      chunk_.cksize = sizeof(PCMWAVEFORMAT);
      hr = mmioCreateChunk(file, &chunk_, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to create 'fmt' chunk. [Error code] ", to_string(hr));
        return false;
      }

      LONG lBytesInWfx = sizeof(WAVEFORMATEX) + outputPWFX->cbSize;
      LONG lBytesWritten = mmioWrite(file, reinterpret_cast<PCHAR>(const_cast<LPWAVEFORMATEX>(outputPWFX)), lBytesInWfx);
      if (lBytesWritten != lBytesInWfx) {
        DEBUG_ERROR("Failed to write WAVEFORMATEX data");
        return false;
      }

      hr = mmioAscend(file, &chunk_, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to ascend from 'fmt' chunk. [Error code] ", to_string(hr));
        return false;
      }


      //
      chunk_.ckid = MAKEFOURCC('f', 'a', 'c', 't');
      hr = mmioCreateChunk(file, &chunk_, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to create a 'fact' chunk. [Error code] ", to_string(hr));
        return false;
      }

      DWORD frames = 0;
      lBytesWritten = mmioWrite(file, reinterpret_cast<PCHAR>(&frames), sizeof(frames));
      if (lBytesWritten != sizeof(frames)) {
        DEBUG_ERROR("mmioWrite wrote unexpected bytes");
        return false;
      }

      hr = mmioAscend(file, &chunk_, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to ascend from 'fmt' chunk. [Error code] ", to_string(hr));
        return false;
      }

      // make a 'data' chunk and leave the data pointer there
      pckData->ckid = MAKEFOURCC('d', 'a', 't', 'a');
      hr = mmioCreateChunk(file, pckData, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to create 'data' chunk. [Error code] ", to_string(hr));
        return false;
      }

      return true;
    }

    bool AudioCapture::closeWavefile(HMMIO file, MMCKINFO *pckRiff, MMCKINFO *pckData) {
      MMRESULT hr;

      hr = mmioAscend(file, pckData, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to ascend out of a chunk in a RIFF file. [Error code] ", to_string(hr));
        return false;
      }

      hr = mmioAscend(file, pckRiff, 0);
      if (MMSYSERR_NOERROR != hr) {
        DEBUG_ERROR_VAR("Failed to ascend out of a chunk in a Data file. [Error code] ", to_string(hr));
        return false;
      }

      return true;
    }
  }
}
