/****************************************************************************************************************

Filename	:	AudioEncoder.cpp
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee, Joseph Rios

Copyright	:

****************************************************************************************************************/

#include "AudioEncoder.h"

#include <array>
#include <vector>

namespace FBCapture {
namespace Audio {

/**
 *  Generates ADTS header according to the supplied parameters:
 *  raw_aac_packet_length   - size of the compressed aac frame
 *  channels         - 1(mono), 2 (stereo)
 *  samplingRate      - 44100, 48000
 *  Note the fullLength must count in the ADTS header itself.
 *  See: http://wiki.multimedia.cx/index.php?title=ADTS
 *  Also: http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio#Channel_Configurations
 **/
static std::array<uint8_t, 7> getAdtsHeader(size_t raw_acc_length,
                                            uint8_t channels, int sampleRate) {
  int adtsLength = 7;
  std::array<uint8_t, 7> adts;
  // Variables Recycled by addADTStoPacket
  uint8_t channelCount = channels;
  uint8_t profile = 2; // AAC LC

  uint8_t sampleRateIndex = 0;
  switch (sampleRate) {
  case 96000:
    sampleRateIndex = 0;
    break;
  case 88200:
    sampleRateIndex = 1;
    break;
  case 64000:
    sampleRateIndex = 2;
    break;
  case 48000:
    sampleRateIndex = 3;
    break;
  case 44100:
    sampleRateIndex = 4;
    break;
  case 32000:
    sampleRateIndex = 5;
    break;
  case 24000:
    sampleRateIndex = 6;
    break;
  case 22050:
    sampleRateIndex = 7;
    break;
  case 16000:
    sampleRateIndex = 8;
    break;
  case 12000:
    sampleRateIndex = 9;
    break;
  case 11025:
    sampleRateIndex = 10;
    break;
  case 8000:
    sampleRateIndex = 11;
    break;
  case 7350:
    sampleRateIndex = 12;
    break;
  default:
    sampleRateIndex = 15;
  }

  size_t fullLength = adtsLength + raw_acc_length;
  // fill in ADTS data

  // 11111111    = syncword
  adts[0] = (char)0xFF;

  // 1111 1 00 1  = syncword MPEG-2 Layer CRC
  adts[1] = (char)0xF9;

  adts[2] = (char)(((profile - 1) << 6) + (sampleRateIndex << 2) +
                   (channelCount >> 2));
  adts[3] = (char)(((channelCount & 3) << 6) + (fullLength >> 11));
  adts[4] = (char)((fullLength & 0x7FF) >> 3);
  adts[5] = (char)(((fullLength & 7) << 5) + 0x1F);
  adts[6] = (char)0xFC;
  return adts;
}

FBCAPTURE_STATUS AudioEncoder::continueAudioTranscoding(const string &srcFile,
                                                        const string &dstFile) {
	DEBUG_LOG("Start audio transcoding");
  // Prepare destination file
  wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
  srcFile_ = stringTypeConversion.from_bytes(srcFile);
  dstFile_ = stringTypeConversion.from_bytes(dstFile);

  auto cleanupHandler = [&](HMMIO__* ptr) {
    mmioClose(ptr, 0);
    remove(srcFile.c_str()); // Remove wav file after transcoding to aac
  };
  auto phFile = std::unique_ptr<HMMIO__, decltype(cleanupHandler)>
    (
      mmioOpen(
        // some flags cause mmioOpen write to this buffer
        // but not any that we're using
        const_cast<LPWSTR>(dstFile_.c_str()), NULL,
        MMIO_CREATE | MMIO_WRITE | MMIO_EXCLUSIVE),
      cleanupHandler
      );
  if (!phFile) {
    DEBUG_ERROR("Failed to create transcoded audio file.");
    return FBCAPTURE_STATUS_OUTPUT_FILE_CREATION_FAILED;
  }

  // Prepare source reader
  HRESULT hr = S_OK;
  ScopedCOMPtr<IMFSourceReader> reader = nullptr;
  if (!mfActivate) {
    auto status = initializeAudioTranscoding(&reader);
    if (status != FBCAPTURE_STATUS_OK) {
      return status;
    }
  } else {
    hr = MFCreateSourceReaderFromURL(srcFile_.c_str(), nullptr, &reader);
    if (FAILED(hr)) {
      DEBUG_ERROR_VAR("Failed to create source reader. [Error code] ",
                      to_string(hr));
      return FBCAPTURE_STATUS_MF_SOURCE_READER_CREATION_FAILED;
    }

    hr = reader->SetCurrentMediaType(
        static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr,
        mftInMediaType);
    if (FAILED(hr)) {
      DEBUG_ERROR_VAR("Failed to set pcm media type. [Error code] ",
                      to_string(hr));
      return FBCAPTURE_STATUS_MF_MEDIA_TYPE_SET_FAILED;
    }
  }

  hr = reader->SetStreamSelection(
      static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), TRUE);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to select audio stream. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_STREAM_SELECTION_FAILED;
  }

  // While there is source data to read
  std::vector<BYTE> audioData = {};
  while (SUCCEEDED(hr)) {
    DWORD dwFlags = 0;

    hr = MFCreateSample(&(outputData.pSample));
    if (FAILED(hr)) {
      DEBUG_ERROR_VAR("Failed to create output sample. [Error code] ",
                      to_string(hr));
      return FBCAPTURE_STATUS_MF_CREATE_SAMPLE_FAILED;
    }
    // Auto release at scope end
    ScopedCOMPtr<IMFSample> outputSample(outputData.pSample);

    hr = outputSample->AddBuffer(outMediaBuffer);
    if (FAILED(hr)) {
      DEBUG_ERROR_VAR("Failed to add buffer to output sample. [Error code] ",
                      to_string(hr));
      return FBCAPTURE_STATUS_MF_SAMPLE_ADD_BUFFER_FAILED;
    }

    // Read the next sample.
    ScopedCOMPtr<IMFSample> inputSample;
    hr = reader->ReadSample(
        static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr,
        &dwFlags, nullptr, &inputSample);
    if (FAILED(hr)) {
      DEBUG_ERROR_VAR("Failed to read input pcm audio sample. [Error code] ",
                      to_string(hr));
      return FBCAPTURE_STATUS_MF_READ_SAMPLE_FAILED;
    }
    if ((dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) {
      break;
    }
    if (inputSample == nullptr) {
      continue;
    }

    hr = mfTransform->ProcessInput(0, inputSample, 0);
    if (FAILED(hr)) {
      DEBUG_ERROR_VAR("Failed to transform pcm audio input. [Error code] ",
                      to_string(hr));
      return FBCAPTURE_STATUS_MF_TRANSFORM_FAILED;
    }

    // While there is output samples
    while (true) {
      hr = mfTransform->ProcessOutput(0, 1, &outputData, &processOutputStatus);
      if (FAILED(hr)) {
        if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
          hr = S_OK;
          break;
        } else {
          DEBUG_ERROR_VAR("Failed to read adts aac audio output. [Error code] ",
                          to_string(hr));
          return FBCAPTURE_STATUS_MF_READ_SAMPLE_FAILED;
        }
      }

      DWORD bufferSize = 0;
      BYTE *data = nullptr;
      hr = outMediaBuffer->Lock(&data, nullptr, &bufferSize);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR(
            "Failed to lock adts aac audio out media buffer. [Error code] ",
            to_string(hr));
        return FBCAPTURE_STATUS_MF_BUFFER_LOCK_FAILED;
      }

      audioData.resize(bufferSize);
      CopyMemory(audioData.data(), data, bufferSize);
      hr = outMediaBuffer->Unlock();
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR(
            "Failed to unlock adts aac audio out media buffer. [Error code] ",
            to_string(hr));
        return FBCAPTURE_STATUS_MF_BUFFER_LOCK_FAILED;
      }

      auto adtsHeader = getAdtsHeader(audioData.size(), channels, sampleRate);
      mmioWrite(phFile.get(), reinterpret_cast<PCCH>(&adtsHeader[0]),
                adtsHeader.size() * sizeof(uint8_t));
      mmioWrite(phFile.get(), reinterpret_cast<PCCH>(audioData.data()),
                audioData.size() * sizeof(BYTE));
      audioData.clear();
    }
  }

  return FBCAPTURE_STATUS_OK;
}

FBCAPTURE_STATUS
AudioEncoder::initializeAudioTranscoding(IMFSourceReader **reader) {
  HRESULT hr = MFStartup(MF_VERSION);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to startup MF. [Error code] ", to_string(hr));
    return FBCAPTURE_STATUS_MF_STARTUP_FAILED;
  }

  MFT_REGISTER_TYPE_INFO inInfo = {MFMediaType_Audio, MFAudioFormat_PCM};
  MFT_REGISTER_TYPE_INFO outInfo = {MFMediaType_Audio, MFAudioFormat_AAC};

  hr = MFTEnumEx(MFT_CATEGORY_AUDIO_ENCODER, 0, &inInfo, &outInfo, &mfActivate,
                 &mftCount);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on getting a list of audio encoder from Media "
                    "Foundation Transforms. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_TRANSFORM_CREATION_FAILED;
  }

  hr = mfActivate[0]->ActivateObject(__uuidof(IMFTransform),
                                     (void **)&mfTransform);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on creating the object associated with the "
                    "activation object. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_TRANSFORM_CREATION_FAILED;
  }

  hr = MFCreateSourceReaderFromURL(srcFile_.c_str(), nullptr, reader);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to create source reader. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_SOURCE_READER_CREATION_FAILED;
  }

  hr = (*reader)->SetStreamSelection(
      static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), TRUE);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to select audio stream. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_STREAM_SELECTION_FAILED;
  }

  ScopedCOMPtr<IMFMediaType> partialType = nullptr;
  hr = MFCreateMediaType(&partialType);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to create partial media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CREATION_FAILED;
  }

  hr = partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR(
        "Failed to set partial media type major type as audio. [Error code] ",
        to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR(
        "Failed to set partial media type sub type as pcm. [Error code] ",
        to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = (*reader)->SetCurrentMediaType(
      static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr,
      partialType);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to set pcm media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_SET_FAILED;
  }

  hr = (*reader)->GetCurrentMediaType(
      static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &mftInMediaType);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to get pcm media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_GET_FAILED;
  }

  WAVEFORMATEX *format = nullptr;
  UINT32 size = 0;
  hr = MFCreateWaveFormatExFromMFMediaType(mftInMediaType, &format, &size);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR(
        "Failed to convert pcm media type to wav format ex. [Error code] ",
        to_string(hr));
    return FBCAPTURE_STATUS_MF_CREATE_WAV_FORMAT_FROM_MEDIA_TYPE_FAILED;
  }
  sampleRate = format->nSamplesPerSec;
  channels = format->nChannels;
  CoTaskMemFree(format);
  format = nullptr;

  hr = MFCreateMediaType(&mftOutMediaType);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to create new adts aac media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CREATION_FAILED;
  }

  hr = mftOutMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_MAJOR_TYPE' in new media type. "
                    "[Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR(
        "Failed on setting 'MF_MT_SUBTYPE' in new media type. [Error code] ",
        to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_BITS_PER_SAMPLE' in new "
                    "media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_SAMPLES_PER_SECOND' in "
                    "new media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_NUM_CHANNELS' in new "
                    "media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 12000);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_AVG_BYTES_PER_SECOND' in "
                    "new media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, 0);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AAC_PAYLOAD_TYPE' in new media "
                    "type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION,
                                  0x29);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR(
        "Failed on setting 'MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION' in new "
        "media type. [Error code] ",
        to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_BLOCK_ALIGNMENT' in new "
                    "media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, 0);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_ALL_SAMPLES_INDEPENDENT' in "
                    "new media type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mftOutMediaType->SetUINT32(MF_MT_AVG_BITRATE, 96000);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AVG_BITRATE' in new media "
                    "type. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mfTransform->SetInputType(0, mftInMediaType, 0);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR(
        "Failed to set input media type on transform. [Error code] ",
        to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  hr = mfTransform->SetOutputType(0, mftOutMediaType, 0);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR(
        "Failed to set output media type on transform. [Error code] ",
        to_string(hr));
    return FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED;
  }

  outputData.dwStreamID = 0;
  hr = mfTransform->GetOutputStreamInfo(outputData.dwStreamID, &outStreamInfo);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR(
        "Failed to get output stream info from transform. [Error code] ",
        to_string(hr));
    return FBCAPTURE_STATUS_MF_TRANSFORM_OUTPUT_STREAM_INFO_FAILED;
  }

  hr = MFCreateMemoryBuffer(outStreamInfo.cbSize, &outMediaBuffer);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to create memory buffer. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_CREATE_MEMORY_BUFFER_FAILED;
  }

  hr = outMediaBuffer->SetCurrentLength(outStreamInfo.cbSize);
  if (FAILED(hr)) {
    DEBUG_ERROR_VAR("Failed to set length on memory buffer. [Error code] ",
                    to_string(hr));
    return FBCAPTURE_STATUS_MF_CREATE_MEMORY_BUFFER_FAILED;
  }

  return FBCAPTURE_STATUS_OK;
}

void AudioEncoder::shutdown() {

  mftOutMediaType = nullptr;
  mftInMediaType = nullptr;
  mfTransform = nullptr;
  outMediaBuffer = nullptr;

  if (mfActivate) {
    for (uint32_t idx = 0; idx < mftCount; idx++) {
      mfActivate[idx]->Release();
    }
    CoTaskMemFree(mfActivate);
    mfActivate = nullptr;

    MFShutdown();
  }

	mftCount = 0;
	channels = 0;
	sampleRate = 0;
}

} // namespace Audio
} // namespace FBCapture
