/****************************************************************************************************************

Filename	:	AudioCapture.h
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/
#pragma once
#include <Windows.h>
#include <iostream>
#include <codecvt>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "AudioEncoder.h"
#include <iostream>
#include <initguid.h>
#include <stdio.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>
#include "log.h"

using namespace FBCapture::Log;
using namespace std;

namespace FBCapture { namespace Audio {

	class AudioCapture
	{
	public:
		AudioCapture();
		virtual ~AudioCapture();
		bool initializeAudio(const string& srcFile);
		void startAudioCapture();
		void stopAudioCapture();

	private:
		bool writeWaveHeader(HMMIO file, LPCWAVEFORMATEX pwfx, MMCKINFO *pckRiff, MMCKINFO *pckData);
		bool closeWavefile(HMMIO file, MMCKINFO *pckRiff, MMCKINFO *pckData);
		bool openFile(LPCWSTR szFileName, HMMIO *phFile);

	private:
		static HMMIO file_;
		static IAudioCaptureClient* audioCaptureClient_;
		static IAudioClient* audioClient_;
		static IMMDeviceEnumerator *mmDeviceEnumerator_;
		static IMMDevice *mmDevice_;
		static bool int16_;
		static MMCKINFO ckRIFF_;
		static MMCKINFO ckData_;
		static UINT32 pnFrames_;
		static UINT32 blockAlign_;
		static LPCWSTR fileName_;
		static wstring wavFileName_;
		static string aacFileName_;
	};
}}
