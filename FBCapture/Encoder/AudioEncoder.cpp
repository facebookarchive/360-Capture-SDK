/****************************************************************************************************************

Filename	:	AudioEncoder.cpp
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#include "AudioEncoder.h"

namespace FBCapture {
  namespace Audio {

    AudioEncoder::AudioEncoder() : mediaSession_(NULL), mediaSource_(NULL), sink_(NULL) {}

    AudioEncoder::~AudioEncoder() {}

    FBCAPTURE_STATUS AudioEncoder::addSourceNode(IMFTopology* topology, IMFPresentationDescriptor* presDesc, IMFStreamDescriptor* streamDesc, IMFTopologyNode** node) {
      HRESULT hr;

      hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &(*node));
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Can't create topology node with MF_TOPOLOGY_SOURCESTREAM_NODE. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_CREATION_FAILED;
      }

      hr = (*node)->SetUnknown(MF_TOPONODE_SOURCE, mediaSource_);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on associating an IUnknown pointer with MF_TOPONODE_SOURCE. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = (*node)->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presDesc);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on associating an IUnknown pointer with MF_TOPONODE_PRESENTATION_DESCRIPTOR. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = (*node)->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDesc);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on associating an IUnknown pointer with MF_TOPONODE_STREAM_DESCRIPTOR. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = topology->AddNode(*node);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on adding a node to the topology. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AudioEncoder::addTransformNode(IMFTopology* topology, IMFTopologyNode* srcNode, IMFStreamDescriptor* streamDesc, IMFTopologyNode** node) {
      HRESULT hr;
      FBCAPTURE_STATUS status;
      IMFMediaType* mediaType = NULL;
      IMFMediaTypeHandler* mediaTypeHandler = NULL;
      GUID majorTypeGuid = GUID_NULL;
      DWORD mediaTypeCount;

      hr = streamDesc->GetMediaTypeHandler(&mediaTypeHandler);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on retrieving a media type handler for stream. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = mediaTypeHandler->GetMajorType(&majorTypeGuid);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on getting the major type of the format. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = mediaTypeHandler->GetMediaTypeCount(&mediaTypeCount);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on retrieving the number of media types in the object's list of supported media types. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = mediaTypeHandler->GetMediaTypeByIndex(0, &mediaType);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on retrieving a media type from object's list of supported media types. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      status = createNewMediaFormat(topology, srcNode, node, mediaType);
      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR_VAR("Failed on creating new media format for AAC. [Error code] ", to_string(hr));
        return status;
      }

      return status;
    }

    FBCAPTURE_STATUS AudioEncoder::createNewMediaFormat(IMFTopology* topology, IMFTopologyNode* srcNode, IMFTopologyNode** node, IMFMediaType* mediaType) {
      HRESULT hr;

      WAVEFORMATEX *inWavFormat = NULL, *outWavFormat = NULL;
      UINT32 waveFormatSize;
      UINT32 mftCount;
      IMFActivate** mfActivate = NULL;
      IMFTransform* mfTransform = NULL;
      IMFMediaType* outputMediaType = NULL;
      IMFMediaType* newMediaType = NULL;

      hr = MFCreateWaveFormatExFromMFMediaType(mediaType, &inWavFormat, &waveFormatSize);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on retrieving a GUID value associated with MF_MT_SUBTYPE. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_CREATING_WAV_FORMAT_FAILED;
      }

      MFT_REGISTER_TYPE_INFO inInfo = { MFMediaType_Audio, MFAudioFormat_PCM };
      MFT_REGISTER_TYPE_INFO outInfo = { MFMediaType_Audio, MFAudioFormat_AAC };

      hr = MFTEnumEx(MFT_CATEGORY_AUDIO_ENCODER, 0, &inInfo, &outInfo, &mfActivate, &mftCount);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on getting a list of audio encoder from Media Foundation Transforms. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_CREATION_FAILED;
      }

      hr = mfActivate[0]->ActivateObject(__uuidof(IMFTransform), (void**)&mfTransform);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on creating the object associated with the activation object. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &(*node));
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Can't create topology node with MF_TOPOLOGY_TRANSFORM_NODE. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_CREATION_FAILED;
      }

      hr = (*node)->SetObject(mfTransform);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting object associated in current node. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = topology->AddNode(*node);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on adding node in current node. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = srcNode->ConnectOutput(0, *node, 0);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on connecting output stream to current node. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED;
      }

      hr = MFCreateMediaType(&outputMediaType);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on creating new empty media type with current media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_MAJOR_TYPE' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_SUBTYPE' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_BITS_PER_SAMPLE' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_SAMPLES_PER_SECOND' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, inWavFormat->nChannels);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_NUM_CHANNELS' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 12000);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_AVG_BYTES_PER_SECOND' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, 1);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AAC_PAYLOAD_TYPE' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, 0x29);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AUDIO_BLOCK_ALIGNMENT' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, 0);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_ALL_SAMPLES_INDEPENDENT' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = outputMediaType->SetUINT32(MF_MT_AVG_BITRATE, 96000);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting 'MF_MT_AVG_BITRATE' in new media type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = mfTransform->SetOutputType(0, outputMediaType, 0);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting media type for output stream. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = mfTransform->GetOutputCurrentType(0, &newMediaType);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on getting media type for output stream. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      hr = MFCreateWaveFormatExFromMFMediaType(newMediaType, &outWavFormat, &waveFormatSize);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on converting a Media Foundation audio media to a output WavFormat. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED;
      }

      if (mfActivate != NULL) {
        for (UINT32 i = 0; i < mftCount; i++) {
          if (mfActivate[i] != NULL) {
            mfActivate[i]->Release();
          }
        }
      }

      CoTaskMemFree(mfActivate);
      CoTaskMemFree(inWavFormat);
      CoTaskMemFree(outWavFormat);

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AudioEncoder::addOutputNode(IMFTopology* topology, IMFTopologyNode* transformNode, IMFTopologyNode** outputNode) {
      HRESULT hr;
      DWORD sinkCount;
      IMFByteStream* byteStream = NULL;
      IMFStreamSink* streamSink = NULL;
      IMFMediaType* outputMediaType = NULL;
      IMFTransform* mfTransform = NULL;

      hr = transformNode->GetObjectW((IUnknown**)&mfTransform);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on getting object associated with current node. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = mfTransform->GetOutputCurrentType(0, &outputMediaType);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on getting current output mdedia type. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = MFCreateFile(MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE, dstFile_.c_str(), &byteStream);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on creating output file for aac. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = MFCreateADTSMediaSink(byteStream, outputMediaType, &sink_);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on creating instance of the audio data transport stream(ADTS) media sink. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = sink_->GetStreamSinkCount(&sinkCount);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on getting stream sink count. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = sink_->GetStreamSinkByIndex(0, &streamSink);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on getting stream sink specified by index 0. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &(*outputNode));
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Can't create topology node with MF_TOPOLOGY_OUTPUT_NODE. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = (*outputNode)->SetObject(streamSink);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting object in current node with stream sink. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = topology->AddNode(*outputNode);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on adding node to output node. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      hr = transformNode->ConnectOutput(0, *outputNode, 0);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on connecting node to output node. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED;
      }

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS AudioEncoder::audioTranscoding(const string& srcFile, const string& dstFile) {
      HRESULT hr = S_OK;
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
      BOOL selected;
      DWORD streamCount = 0;

      MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
      IMFSourceResolver *pSourceResolver = NULL;
      IMFTopology* topology = NULL;
      IMFPresentationDescriptor* presDESC = NULL;
      IMFStreamDescriptor* streamDESC = NULL;
      IMFTopologyNode* srcNode = NULL;
      IMFTopologyNode* transformNode = NULL;
      IMFTopologyNode* outputNode = NULL;

      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      srcFile_ = stringTypeConversion.from_bytes(srcFile);
      dstFile_ = stringTypeConversion.from_bytes(dstFile);

      hr = MFStartup(MF_VERSION);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Can't initialize Microsoft Media Foundation. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      IMFMediaEvent *pEvent = NULL;
      hr = MFCreateMediaSession(NULL, &mediaSession_);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Can't create media session. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      hr = MFCreateSourceResolver(&pSourceResolver);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Can't create source resolver, which is used to create media source from url. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      hr = pSourceResolver->CreateObjectFromURL(srcFile_.c_str(), MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, (IUnknown**)&mediaSource_);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on creating media source from input file. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_FILE_READING_ERROR;
      }

      hr = MFCreateTopology(&topology);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on creating topology. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      hr = mediaSource_->CreatePresentationDescriptor(&presDESC);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on creating presentation descriptor of media source. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      hr = presDESC->GetStreamDescriptorCount(&streamCount);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on retrieving number of stream descriptors in the presentation. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      hr = presDESC->GetStreamDescriptorByIndex(0, &selected, &streamDESC);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on retrieving number of stream descriptors by index 0. [Error code]", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      status = addSourceNode(topology, presDESC, streamDESC, &srcNode);
      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR("Audio transcoding is failed on addSourceNode().");
        return status;
      }

      status = addTransformNode(topology, srcNode, streamDESC, &transformNode);
      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR("Audio transcoding is failed on addTransformNode().");
        return status;
      }

      status = addOutputNode(topology, transformNode, &outputNode);
      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR("Audio transcoding is failed on addOutputNode().");
        return status;
      }

      hr = mediaSession_->SetTopology(0, topology);
      if (FAILED(hr)) {
        DEBUG_ERROR_VAR("Failed on setting a topology on the media session. [Error code] ", to_string(hr));
        return FBCAPTURE_STATUS_MF_INIT_FAILED;
      }

      status = handleMediaEvent();

      if (status != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR("Failed in handleMediaEvent().");
        return status;
      }

      SAFE_RELEASE(pSourceResolver);
      SAFE_RELEASE(topology);
      SAFE_RELEASE(presDESC);
      SAFE_RELEASE(streamDESC);
      SAFE_RELEASE(srcNode);
      SAFE_RELEASE(transformNode);
      SAFE_RELEASE(outputNode);

      shutdownSessions();

      remove(srcFile.c_str());  // Remove wav file after transcoding to aac

      return status;
    }

    FBCAPTURE_STATUS AudioEncoder::handleMediaEvent() {
      PROPVARIANT var;
      bool done = false;
      do {
        HRESULT hr;
        HRESULT mediaSessionStatus;
        IMFMediaEvent *mediaSessionEvent = NULL;
        MediaEventType mediaEventType = NULL;
        MF_TOPOSTATUS topo_status = (MF_TOPOSTATUS)0;

        hr = mediaSession_->GetEvent(0, &mediaSessionEvent);
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed on getting event from current media session. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_MF_HANDLING_MEDIA_SESSION_FAILED;
        }

        hr = mediaSessionEvent->GetStatus(&mediaSessionStatus);
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed on getting media session status. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_MF_HANDLING_MEDIA_SESSION_FAILED;
        }

        hr = mediaSessionEvent->GetType(&mediaEventType);
        if (FAILED(hr)) {
          DEBUG_ERROR_VAR("Failed on getting event type in current media session. [Error code] ", to_string(hr));
          return FBCAPTURE_STATUS_MF_HANDLING_MEDIA_SESSION_FAILED;
        }

        if (SUCCEEDED(hr) && SUCCEEDED(mediaSessionStatus)) {
          switch (mediaEventType) {
            case MESessionTopologyStatus:
              hr = mediaSessionEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&topo_status);
              if (SUCCEEDED(hr)) {
                switch (topo_status) {
                  case MF_TOPOSTATUS_READY:
                    // Fire up media playback (with no particular starting position)
                    PropVariantInit(&var);
                    var.vt = VT_EMPTY;
                    hr = mediaSession_->Start(&GUID_NULL, &var);
                    if (FAILED(hr)) {
                      DEBUG_ERROR_VAR("Failed on starting media session when topology status is ready. [Error code] ", to_string(hr));
                      break;
                    }
                    PropVariantClear(&var);
                    break;

                  case MF_TOPOSTATUS_STARTED_SOURCE:
                    break;

                  case MF_TOPOSTATUS_ENDED:
                    break;
                }
              } else {
                DEBUG_ERROR("Failed on getting topology status in current media session.");
              }

              break;

            case MESessionStarted:
              break;

            case MESessionEnded:
              hr = mediaSession_->Stop();
              if (FAILED(hr)) {
                DEBUG_ERROR_VAR("Failed on stopping media session. [Error code] ", to_string(hr));
              }
              break;

            case MESessionStopped:
              hr = mediaSession_->Close();
              if (FAILED(hr)) {
                DEBUG_ERROR_VAR("Failed on finalizing media session. [Error code] ", to_string(hr));
              }
              break;

            case MESessionClosed:
              done = TRUE;
              break;

            default:
              break;
          }
        }

        SAFE_RELEASE(mediaSessionEvent);

        if (FAILED(hr) || FAILED(mediaSessionStatus)) {
          DEBUG_ERROR("Failed on transcoing wav to aac file.");
          return FBCAPTURE_STATUS_MF_HANDLING_MEDIA_SESSION_FAILED;
        }

      } while (!done);

      return FBCAPTURE_STATUS_OK;
    }

    void AudioEncoder::shutdownSessions() {
      if (mediaSession_) {
        mediaSession_->Shutdown();
        SAFE_RELEASE(mediaSession_);
      }

      if (mediaSource_) {
        mediaSource_->Shutdown();
        SAFE_RELEASE(mediaSource_);
      }

      if (sink_) {
        sink_->Shutdown();
        SAFE_RELEASE(sink_);
      }

      MFShutdown();

    }
  }
}
