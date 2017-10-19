/****************************************************************************************************************

Filename	:	AudioCapture.h
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/
#pragma once
#include <Windows.h>
#include <iostream>
#include <codecvt>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "AudioEncoder.h"
#include "AudioBuffer.h"
#include <iostream>
#include <initguid.h>
#include <stdio.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <regex>
#include <functiondiscoverykeys_devpkey.h>
#include "log.h"
#include "ErrorCodes.h"

using namespace FBCapture::Common;
using namespace std;

namespace FBCapture {
  namespace Audio {

    class AudioCapture {
    public:
      AudioCapture();
      virtual ~AudioCapture();
      FBCAPTURE_STATUS initializeAudio(const string& srcFile, bool useRiftAudioEndpoint);
      void startAudioCapture(bool silenceMode);
      FBCAPTURE_STATUS stopAudioCapture();

    private:
      bool writeWaveHeader(HMMIO file, LPCWAVEFORMATEX pwfx, MMCKINFO *pckRiff, MMCKINFO *pckData);
      bool closeWavefile(HMMIO file, MMCKINFO *pckRiff, MMCKINFO *pckData);
      bool openFile(LPCWSTR szFileName, HMMIO *phFile);
      bool findRiftAudioSource(bool useRiftAudioEndpoint, bool useMic);
      bool startAudioclient(IAudioClient* audioClient, IAudioCaptureClient** captureClient, bool isInput);

    private:
      static HMMIO file_;
      static IAudioCaptureClient* outputAudioCaptureClient_;
      static IAudioClient* outputAudioClient_;
      static IMMDeviceEnumerator *mmDeviceEnumerator_;
      static IMMDevice *mmOutputDevice_;
      static IAudioCaptureClient* inputAudioCaptureClient_;
      static IAudioClient* inputAudioClient_;
      static IMMDevice* mmInputDevice_;
      static MMCKINFO ckRIFF_;
      static MMCKINFO ckData_;
      static MMCKINFO chunk_;
      static LPCWSTR fileName_;
      static wstring wavFileName_;
      static string aacFileName_;
      static UINT32 outputBlockAlign_;
      static UINT32 inputBlockAlign_;
      static LONG outputBytesToWrite_;
      static LONG inputBytesToWrite_;
      static BYTE* outputData_;
      static BYTE* inputData_;
      static WAVEFORMATEX *outputPWFX_;
      static WAVEFORMATEX *inputPWFX_;
      static size_t captureIndex_;
      static AudioBuffer* buffer_;

      bool useMicrophone_;
      bool useRiftAudioResources_;
    };
  }
}
