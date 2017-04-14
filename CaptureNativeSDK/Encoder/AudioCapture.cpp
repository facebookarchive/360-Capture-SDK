/****************************************************************************************************************

Filename	:	AudioCapture.cpp
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#include "AudioCapture.h"

namespace FBCapture { namespace Audio {

	IMMDeviceEnumerator* AudioCapture::mmDeviceEnumerator_ = NULL;
	IAudioCaptureClient* AudioCapture::audioCaptureClient_ = NULL;
	IAudioClient* AudioCapture::audioClient_ = NULL;
	IMMDevice* AudioCapture::mmDevice_ = NULL;
	HMMIO AudioCapture::file_ = NULL;
	bool AudioCapture::int16_ = false;
	MMCKINFO AudioCapture::ckRIFF_ = { 0 };
	MMCKINFO AudioCapture::ckData_ = { 0 };
	UINT32 AudioCapture::pnFrames_ = 0;
	UINT32 AudioCapture::blockAlign_ = 0;
	wstring AudioCapture::wavFileName_;
	LPCWSTR AudioCapture::fileName_ = NULL;

	AudioCapture::AudioCapture()
	{}

	AudioCapture::~AudioCapture()
	{
		if (file_) {
			closeWavefile(file_, &ckData_, &ckRIFF_);
			mmioClose(file_, 0);
		}
	}


	void AudioCapture::stopAudioCapture()
	{
		MMRESULT hr = MMSYSERR_NOERROR;

		if (!closeWavefile(file_, &ckData_, &ckRIFF_)) {
			DEBUG_ERROR("Failed to ascend out of a chunk in a RIFF file");
			return;
		}

		hr = mmioClose(file_, 0);
		file_ = NULL;
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to close file was opened by using mmioOpen");
			return;
		}
	}

	bool AudioCapture::initializeAudio(const string& srcFile)
	{
		HRESULT hr = S_OK;

		wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
		wavFileName_ = stringTypeConversion.from_bytes(srcFile);

		hr = CoCreateInstance(
			__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator),
			(void**)&mmDeviceEnumerator_
		);
		if (FAILED(hr)) {
			DEBUG_ERROR("Failed to create instance with CoCreateInstance");
			return false;
		}

		hr = mmDeviceEnumerator_->GetDefaultAudioEndpoint(eRender, eConsole, &mmDevice_);
		if (FAILED(hr)) {
			DEBUG_ERROR("Failed to get default audio endpoint with GetDefaultAudioEndpoint");
			return false;
		}

		openFile(wavFileName_.c_str(), &file_);

		hr = mmDevice_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audioClient_);
		if (FAILED(hr)) {
			DEBUG_ERROR("Failed to activate audio client");
			return false;
		}

		REFERENCE_TIME hnsDefaultDevicePeriod;
		hr = audioClient_->GetDevicePeriod(&hnsDefaultDevicePeriod, NULL);
		if (FAILED(hr)) {
			DEBUG_ERROR("Failed to get default device periodicity");
			return false;
		}

		WAVEFORMATEX *pwfx;
		hr = audioClient_->GetMixFormat(&pwfx);
		if (FAILED(hr)) {
			DEBUG_ERROR("Failed to get default device format");
			return false;
		}

		if (int16_) {
			// coerce int-16 wave format
			switch (pwfx->wFormatTag) {
			case WAVE_FORMAT_IEEE_FLOAT:
				pwfx->wFormatTag = WAVE_FORMAT_PCM;
				pwfx->wBitsPerSample = 16;
				pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
				pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
				break;

			case WAVE_FORMAT_EXTENSIBLE:
			{
				// naked scope for case-local variable
				PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
				if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
					pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
					pEx->Samples.wValidBitsPerSample = 16;
					pwfx->wBitsPerSample = 16;
					pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
					pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
				}
				else {
					DEBUG_ERROR("Failed to coerce mix format to int-16");
					return false;
				}
			}
			break;

			default:
				DEBUG_ERROR("Failed to coerce WAVEFORMATEX with wFormatTag");
				return false;
			}
		}

		ckRIFF_ = { 0 };
		ckData_ = { 0 };

		if (!writeWaveHeader(file_, pwfx, &ckRIFF_, &ckData_)) {
			DEBUG_ERROR("Failed to write wave header");
			return false;
		}

		blockAlign_ = pwfx->nBlockAlign;
		pnFrames_ = 0;

		hr = audioClient_->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, pwfx, 0);
		if (FAILED(hr)) {
			DEBUG_ERROR("Failed to initialize audio client");
			return false;
		}

		hr = audioClient_->GetService(__uuidof(IAudioCaptureClient), (void**)&audioCaptureClient_);
		if (FAILED(hr)) {
			DEBUG_ERROR("Failed to activate an IAudioCaptureClient");
			return false;
		}

		hr = audioClient_->Start();
		if (FAILED(hr)) {
			DEBUG_ERROR("Failed to start audio client");
			return false;
		}

		return true;
	}

	void AudioCapture::startAudioCapture() {
		HRESULT hr = S_OK;
		UINT32 nNextPacketSize;

		for (hr = audioCaptureClient_->GetNextPacketSize(&nNextPacketSize);	SUCCEEDED(hr) &&
			nNextPacketSize > 0;
			hr = audioCaptureClient_->GetNextPacketSize(&nNextPacketSize)) {
			// get the captured data
			BYTE *pData;
			UINT32 nNumFramesToRead;
			DWORD dwFlags;

			hr = audioCaptureClient_->GetBuffer(&pData, &nNumFramesToRead, &dwFlags, NULL, NULL);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed to get buffer from IAudioCaptureClient::GetBuffer");
				break;
			}

			if (AUDCLNT_BUFFERFLAGS_SILENT == dwFlags) {
				DEBUG_LOG("Silent status without any audio");
			}

			if (0 == nNumFramesToRead) {
				DEBUG_ERROR("No data which can be read from IAudioCaptureClient::GetBuffer");
				break;
			}

			LONG lBytesToWrite = nNumFramesToRead * blockAlign_;
#pragma prefast(suppress: __WARNING_INCORRECT_ANNOTATION, "IAudioCaptureClient::GetBuffer SAL annotation implies a 1-byte buffer")
			LONG lBytesWritten = mmioWrite(file_, reinterpret_cast<PCHAR>(pData), lBytesToWrite);
			if (lBytesToWrite != lBytesWritten) {
				DEBUG_ERROR_VAR("Failed to write audio data. Expected bytes", std::to_string(lBytesToWrite));
				break;
			}

			hr = audioCaptureClient_->ReleaseBuffer(nNumFramesToRead);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed to release buffer");
				break;
			}
		}
	}

	bool AudioCapture::openFile(LPCWSTR szFileName, HMMIO *phFile)
	{

		MMIOINFO mi = { 0 };
		fileName_ = szFileName;
		*phFile = mmioOpen(
			// some flags cause mmioOpen write to this buffer
			// but not any that we're using
			const_cast<LPWSTR>(fileName_),
			&mi,
			MMIO_WRITE | MMIO_CREATE
		);

		if (NULL == *phFile) {
			DEBUG_ERROR("Failed to create audio file");
			return false;
		}

		return true;
	}

	bool AudioCapture::writeWaveHeader(HMMIO file, LPCWAVEFORMATEX pwfx, MMCKINFO *pckRiff, MMCKINFO *pckData)
	{
		MMRESULT hr;

		pckRiff->ckid = MAKEFOURCC('R', 'I', 'F', 'F');
		pckRiff->fccType = MAKEFOURCC('W', 'A', 'V', 'E');
		pckRiff->cksize = 0;

		hr = mmioCreateChunk(file, pckRiff, MMIO_CREATERIFF);
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to create 'RIFF' chunk");
			return false;
		}

		MMCKINFO chunk;
		chunk.ckid = MAKEFOURCC('f', 'm', 't', ' ');
		chunk.cksize = sizeof(PCMWAVEFORMAT);
		hr = mmioCreateChunk(file, &chunk, 0);
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to create 'fmt' chunk");
			return false;
		}

		LONG lBytesInWfx = sizeof(WAVEFORMATEX) + pwfx->cbSize;
		LONG lBytesWritten = mmioWrite(file, reinterpret_cast<PCHAR>(const_cast<LPWAVEFORMATEX>(pwfx)), lBytesInWfx);
		if (lBytesWritten != lBytesInWfx) {
			DEBUG_ERROR("Failed to write WAVEFORMATEX data");
			return false;
		}

		hr = mmioAscend(file, &chunk, 0);
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to ascend from 'fmt' chunk");
			return false;
		}

		chunk.ckid = MAKEFOURCC('f', 'a', 'c', 't');
		hr = mmioCreateChunk(file, &chunk, 0);
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to create a 'fact' chunk");
			return false;
		}

		DWORD frames = 0;
		lBytesWritten = mmioWrite(file, reinterpret_cast<PCHAR>(&frames), sizeof(frames));
		if (lBytesWritten != sizeof(frames)) {
			DEBUG_ERROR("mmioWrite wrote unexpected bytes");
			return false;
		}

		hr = mmioAscend(file, &chunk, 0);
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to ascend from 'fmt' chunk");
			return false;
		}

		// make a 'data' chunk and leave the data pointer there
		pckData->ckid = MAKEFOURCC('d', 'a', 't', 'a');
		hr = mmioCreateChunk(file, pckData, 0);
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to create 'data' chunk");
			return false;
		}

		return true;
	}

	bool AudioCapture::closeWavefile(HMMIO file, MMCKINFO *pckRiff, MMCKINFO *pckData)
	{
		MMRESULT hr;

		hr = mmioAscend(file, pckData, 0);
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to ascend out of a chunk in a RIFF file");
			return false;
		}

		hr = mmioAscend(file, pckRiff, 0);
		if (MMSYSERR_NOERROR != hr) {
			DEBUG_ERROR("Failed to ascend out of a chunk in a Data file");
			return false;
		}

		return true;
	}
}}