/****************************************************************************************************************

Filename	:	AudioEncoder.cpp
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#include "AudioEncoder.h"

namespace FBCapture { namespace Audio {

		AudioEncoder::AudioEncoder() : mediaSession_(NULL), mediaSource_(NULL), sink_(NULL){ }

		AudioEncoder::~AudioEncoder() { }

		int AudioEncoder::addSourceNode(IMFTopology* topology, IMFPresentationDescriptor* presDesc, IMFStreamDescriptor* streamDesc, IMFTopologyNode** node)
		{
			HRESULT hr;

			hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &(*node));
			if (FAILED(hr)) {
				DEBUG_ERROR("Can't create topology node with MF_TOPOLOGY_SOURCESTREAM_NODE.");
				return -1;
			}

			hr = (*node)->SetUnknown(MF_TOPONODE_SOURCE, mediaSource_);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on associating an IUnknown pointer with MF_TOPONODE_SOURCE.");
				return -1;
			}

			hr = (*node)->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presDesc);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on associating an IUnknown pointer with MF_TOPONODE_PRESENTATION_DESCRIPTOR.");
				return -1;
			}

			hr = (*node)->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDesc);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on associating an IUnknown pointer with MF_TOPONODE_STREAM_DESCRIPTOR.");
				return -1;
			}

			hr = topology->AddNode(*node);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on adding a node to the topology.");
				return -1;
			}

			return 0;
		}

		int AudioEncoder::addTransformNode(IMFTopology* topology, IMFTopologyNode* srcNode, IMFStreamDescriptor* streamDesc, IMFTopologyNode** node) 
		{
			HRESULT hr;
			IMFMediaType* mediaType = NULL;
			IMFMediaTypeHandler* mediaTypeHandler = NULL;
			GUID majorTypeGuid = GUID_NULL;
			DWORD mediaTypeCount;

			hr = streamDesc->GetMediaTypeHandler(&mediaTypeHandler);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on retrieving a media type handler for stream.");
				return -1;
			}

			hr = mediaTypeHandler->GetMajorType(&majorTypeGuid);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on getting the major type of the format.");
				return -1;
			}

			hr = mediaTypeHandler->GetMediaTypeCount(&mediaTypeCount);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on retrieving the number of media types in the object's list of supported media types.");
				return -1;
			}
			
			hr = mediaTypeHandler->GetMediaTypeByIndex(0, &mediaType);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on retrieving a media type from object's list of supported media types.");
				return -1;
			}		

			if (createNewMediaFormat(topology, srcNode, node, mediaType) < 0) {
				DEBUG_ERROR("Failed on creating new media format for AAC");
				return -1;
			}			

			return 0;
		}

		int AudioEncoder::createNewMediaFormat(IMFTopology* topology, IMFTopologyNode* srcNode, IMFTopologyNode** node, IMFMediaType* mediaType)
		{
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
				DEBUG_ERROR("Failed on retrieving a GUID value associated with MF_MT_SUBTYPE.");
				return -1;
			}

			MFT_REGISTER_TYPE_INFO inInfo = { MFMediaType_Audio, MFAudioFormat_PCM };
			MFT_REGISTER_TYPE_INFO outInfo = { MFMediaType_Audio, MFAudioFormat_AAC };

			hr = MFTEnumEx(MFT_CATEGORY_AUDIO_ENCODER, 0, &inInfo, &outInfo, &mfActivate, &mftCount);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on getting a list of audio encoder from Media Foundation Transforms.");
				return -1;
			}

			hr = mfActivate[0]->ActivateObject(__uuidof(IMFTransform), (void**)&mfTransform);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating the object associated with the activation object.");
				return -1;
			}

			hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &(*node));
			if (FAILED(hr)) {
				DEBUG_ERROR("Can't create topology node with MF_TOPOLOGY_TRANSFORM_NODE.");
				return -1;
			}

			hr = (*node)->SetObject(mfTransform);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting object associated in current node.");
				return -1;
			}

			hr = topology->AddNode(*node);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on adding node in current node.");
				return -1;
			}

			hr = srcNode->ConnectOutput(0, *node, 0);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on connecting output stream to current node.");
				return -1;
			}

			hr = MFCreateMediaType(&outputMediaType);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating new empty media type with current media type.");
				return -1;
			}

			hr = outputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_MAJOR_TYPE' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_SUBTYPE' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_AUDIO_BITS_PER_SAMPLE' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, inWavFormat->nSamplesPerSec);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_AUDIO_SAMPLES_PER_SECOND' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, inWavFormat->nChannels);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_AUDIO_NUM_CHANNELS' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 12000);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_AUDIO_AVG_BYTES_PER_SECOND' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, 0);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_AAC_PAYLOAD_TYPE' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, 0x29);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_AUDIO_BLOCK_ALIGNMENT' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, 0);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_ALL_SAMPLES_INDEPENDENT' in new media type.");
				return -1;
			}

			hr = outputMediaType->SetUINT32(MF_MT_AVG_BITRATE, 96000);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting 'MF_MT_AVG_BITRATE' in new media type.");
				return -1;
			}

			hr = mfTransform->SetOutputType(0, outputMediaType, 0);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting media type for output stream.");
				return -1;
			}

			hr = mfTransform->GetOutputCurrentType(0, &newMediaType);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on getting media type for output stream.");
				return -1;
			}

			hr = MFCreateWaveFormatExFromMFMediaType(newMediaType, &outWavFormat, &waveFormatSize);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on converting a Media Foundation audio media to a output WavFormat.");
				return -1;
			}

			if (mfActivate != NULL) {
				for (UINT32 i = 0;i < mftCount;i++) {
					if (mfActivate[i] != NULL) {
						mfActivate[i]->Release();
					}
				}
			}

			CoTaskMemFree(mfActivate);
			CoTaskMemFree(inWavFormat);
			CoTaskMemFree(outWavFormat);

			return 0;
		}

		int AudioEncoder::addOutputNode(IMFTopology* topology, IMFTopologyNode* transformNode, IMFTopologyNode** outputNode)
		{
			HRESULT hr;
			DWORD sinkCount;
			IMFByteStream* byteStream = NULL;
			IMFStreamSink* streamSink = NULL;
			IMFMediaType* outputMediaType = NULL;
			IMFTransform* mfTransform = NULL;

			hr = transformNode->GetObjectW((IUnknown**)&mfTransform);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on getting object associated with current node.");
				return -1;
			}

			hr = mfTransform->GetOutputCurrentType(0, &outputMediaType);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on getting current output mdedia type.");
				return -1;
			}

			hr = MFCreateFile(MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE, dstFile_.c_str(), &byteStream);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating output file for aac.");
				return -1;
			}

			hr = MFCreateMPEG4MediaSink(byteStream, NULL, outputMediaType, &sink_);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating media sink for authoring MP4 file.");
				return -1;
			}
			
			hr = sink_->GetStreamSinkCount(&sinkCount);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on getting stream sink count.");
				return -1;
			}

			hr = sink_->GetStreamSinkByIndex(0, &streamSink);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on getting stream sink specified by index 0.");
				return -1;
			}

			hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &(*outputNode));
			if (FAILED(hr)) {
				DEBUG_ERROR("Can't create topology node with MF_TOPOLOGY_OUTPUT_NODE.");
				return -1;
			}

			hr = (*outputNode)->SetObject(streamSink);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting object in current node with stream sink.");
				return -1;
			}

			hr = topology->AddNode(*outputNode);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on adding node to output node.");
				return -1;
			}

			hr = transformNode->ConnectOutput(0, *outputNode, 0);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on connecting node to output node.");
				return -1;
			}

			return 0;
		}

		int AudioEncoder::audioTranscoding(const string& srcFile, const string& dstFile)
		{
			HRESULT hr = S_OK;
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
			if (FAILED(hr))	{
				DEBUG_ERROR("Can't initialize Microsoft Media Foundation.");
				return -1;
			}

			IMFMediaEvent *pEvent = NULL;
			hr = MFCreateMediaSession(NULL, &mediaSession_);
			if (FAILED(hr)) {
				DEBUG_ERROR("Can't create media session.");
				return -1;
			}

			hr = MFCreateSourceResolver(&pSourceResolver);
			if (FAILED(hr)) {
				DEBUG_ERROR("Can't create source resolver, which is used to create media source from url.");
				return -1;
			}

			hr = pSourceResolver->CreateObjectFromURL(srcFile_.c_str(), MF_RESOLUTION_MEDIASOURCE,	NULL, &ObjectType, (IUnknown**)&mediaSource_);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating media source from input file.");
				return -1;
			}

			hr = MFCreateTopology(&topology);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating topology.");
				return -1;
			}

			hr = mediaSource_->CreatePresentationDescriptor(&presDESC);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on creating presentation descriptor of media source.");
				return -1;
			}
				
			hr = presDESC->GetStreamDescriptorCount(&streamCount);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on retrieving number of stream descriptors in the presentation.");
				return -1;
			}

			hr = presDESC->GetStreamDescriptorByIndex(0, &selected, &streamDESC);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on retrieving number of stream descriptors by index 0.");
				return -1;
			}

			if (addSourceNode(topology, presDESC, streamDESC, &srcNode) < 0) {
				DEBUG_ERROR("Audio transcoding is failed on addSourceNode().");
				return -1;
			}

			if (addTransformNode(topology, srcNode, streamDESC, &transformNode) < 0) {
				DEBUG_ERROR("Audio transcoding is failed on addTransformNode().");
				return -1;
			}

			if (addOutputNode(topology, transformNode, &outputNode) < 0) {
				DEBUG_ERROR("Audio transcoding is failed on addOutputNode().");
				return -1;
			}

			hr = mediaSession_->SetTopology(0, topology);
			if (FAILED(hr)) {
				DEBUG_ERROR("Failed on setting a topology on the media session.");
				return -1;
			}			
			
			if (handleMediaEvent() < 0) {
				DEBUG_ERROR("Failed in handleMediaEvent().");
				return -1;
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

			return 0;
		}

		int AudioEncoder::handleMediaEvent()
		{
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
					DEBUG_ERROR("Failed on getting event from current media session.");					
				}

				hr = mediaSessionEvent->GetStatus(&mediaSessionStatus);
				if (FAILED(hr)) {
					DEBUG_ERROR("Failed on getting media session status.");					
				}

				hr = mediaSessionEvent->GetType(&mediaEventType);
				if (FAILED(hr)) {
					DEBUG_ERROR("Failed on getting event type in current media session.");					
				}

				if (SUCCEEDED(hr) && SUCCEEDED(mediaSessionStatus))	{
					switch (mediaEventType)	{
					case MESessionTopologyStatus:
						hr = mediaSessionEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&topo_status);						
						if (SUCCEEDED(hr))	{
							switch (topo_status) {
							case MF_TOPOSTATUS_READY:								
								// Fire up media playback (with no particular starting position)
								PropVariantInit(&var);
								var.vt = VT_EMPTY;
								hr = mediaSession_->Start(&GUID_NULL, &var);
								if (FAILED(hr)) {
									DEBUG_ERROR("Failed on starting media session when topology status is ready.");		
									break;
								}
								PropVariantClear(&var);
								break;

							case MF_TOPOSTATUS_STARTED_SOURCE:								
								break;

							case MF_TOPOSTATUS_ENDED:
								break;
							}
						}
						else {
							DEBUG_ERROR("Failed on getting topology status in current media session.");
						}

						break;

					case MESessionStarted:
						break;

					case MESessionEnded:						
						hr = mediaSession_->Stop();
						if (FAILED(hr)) {
							DEBUG_ERROR("Failed on stopping media session.");							
						}
						break;

					case MESessionStopped:						
						hr = mediaSession_->Close();
						if (FAILED(hr)) {
							DEBUG_ERROR("Failed on finalizing media session.");							
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

				if (FAILED(hr) || FAILED(mediaSessionStatus))	{
					DEBUG_ERROR("Failed on transcoing wav to aac file.");
					return -1;
				}

			} while (!done);

			return 0;
		}		

		void AudioEncoder::shutdownSessions()
		{
			if (mediaSession_) {
				mediaSession_->Shutdown();
			}

			if (mediaSource_) {
				mediaSource_->Shutdown();
			}

			if (sink_) {
				sink_->Shutdown();
			}

			SAFE_RELEASE(mediaSession_);
			SAFE_RELEASE(mediaSource_);
			SAFE_RELEASE(sink_);

			MFShutdown();

		}
}}
