/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/


#include "AudioCustomRawDataCapture.h"

namespace FBCapture {
  namespace Audio {

    AudioCustomRawDataCapture::AudioCustomRawDataCapture() {
      needToCloseCaptureFile = false;
      needToOpenCaptureFile = true;
    }

    AudioCustomRawDataCapture::~AudioCustomRawDataCapture() {
      releaseCaptureResources();
    }

    FBCAPTURE_STATUS AudioCustomRawDataCapture::openCaptureFile(const string& srcFile, uint32_t sampleRate, uint32_t channel) {
      MMRESULT mmResult;
      DEBUG_LOG("Start custom audio raw data capture");

      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      wavFileName_ = stringTypeConversion.from_bytes(srcFile);

      file_ = mmioOpen(
        // some flags cause mmioOpen write to this buffer
        // but not any that we're using
        const_cast<LPWSTR>(wavFileName_.c_str()),
        nullptr,
        MMIO_CREATE | MMIO_WRITE | MMIO_EXCLUSIVE
      );

      if (nullptr == file_) {
        DEBUG_ERROR("Failed to create raw audio file.");
        return FBCAPTURE_STATUS_WRITING_WAV_HEADER_FAILED;
      }

      // make a RIFF/WAVE chunk
      mmResult = mmioCreateChunk(file_, &ckRIFF_, MMIO_CREATERIFF);
      if (MMSYSERR_NOERROR != mmResult) {
        DEBUG_ERROR_VAR("Failed to create 'RIFF' chunk. [Error code] ", to_string(mmResult));
        return FBCAPTURE_STATUS_WRITING_WAV_HEADER_FAILED;
      }

      // make a 'fmt ' chunk (within the RIFF/WAVE chunk)			
      mmResult = mmioCreateChunk(file_, &ckFMT_, 0);
      if (MMSYSERR_NOERROR != mmResult) {
        DEBUG_ERROR_VAR("Failed to create 'fmt' chunk. [Error code] ", to_string(mmResult));
        return FBCAPTURE_STATUS_WRITING_WAV_HEADER_FAILED;
      }

      // Write the WAVEFORMATEX for float PCM data
      format = {};
      format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
      format.nChannels = channel;
      format.nSamplesPerSec = sampleRate;
      format.wBitsPerSample = 32;  // 32bit float
      format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
      format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;
      format.cbSize = 0;

      LONG lBytesInWfx = sizeof(WAVEFORMATEX) + format.cbSize;
      LONG lBytesWritten =
        mmioWrite(
          file_,
          reinterpret_cast<PCHAR>(&format),
          lBytesInWfx
        );
      if (lBytesWritten != lBytesInWfx) {
        DEBUG_ERROR("Failed to write WAVEFORMATEX data");
        return FBCAPTURE_STATUS_WRITING_WAV_HEADER_FAILED;
      }

      // ascend from the 'fmt ' chunk
      mmResult = mmioAscend(file_, &ckFMT_, 0);
      if (MMSYSERR_NOERROR != mmResult) {
        DEBUG_ERROR_VAR("Failed to ascend from 'fmt' chunk. [Error code] ", to_string(mmResult));
        return FBCAPTURE_STATUS_WRITING_WAV_HEADER_FAILED;
      }

      // make a 'data' chunk and leave the data pointer there
      mmResult = mmioCreateChunk(file_, &ckData_, 0);
      if (MMSYSERR_NOERROR != mmResult) {
        DEBUG_ERROR_VAR("Failed to create 'data' chunk. [Error code] ", to_string(mmResult));
        return FBCAPTURE_STATUS_WRITING_WAV_HEADER_FAILED;
      }

      needToOpenCaptureFile = false;
      needToInitializeResources = false;

      return FBCAPTURE_STATUS_OK;
    }

    void AudioCustomRawDataCapture::continueAudioCapture(float* audioRawData, uint32_t bufferSize) {      
        mmioWrite(file_, reinterpret_cast<PCCH>(audioRawData), bufferSize * sizeof(float) * format.nChannels);
    }

    void AudioCustomRawDataCapture::releaseCaptureResources() {
      needToInitializeResources = true;
    }
  }
}
