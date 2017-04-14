/****************************************************************************************************************

Filename	:	AudioEncoder.h
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee

Copyright	:
FFmpeg version: 20170321-db7a05d
****************************************************************************************************************/

#pragma once
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <codecvt>
#include "Log.h"

#define SAFE_RELEASE(a) if (a) { a->Release(); a= NULL; }

using namespace std;
using namespace std::chrono;
using namespace FBCapture::Log;

namespace FBCapture { namespace Audio {

	class AudioEncoder {

	public:
		AudioEncoder();
		virtual ~AudioEncoder();
		int audioTranscoding(const string& srcFile, const string& dstFile);

	private:
		int addSourceNode(IMFTopology* topology, IMFPresentationDescriptor* presDesc, IMFStreamDescriptor* streamDesc, IMFTopologyNode** node);
		int addTransformNode(IMFTopology* topology, IMFTopologyNode* srcNode, IMFStreamDescriptor* streamDesc, IMFTopologyNode** node);
		int addOutputNode(IMFTopology* topology, IMFTopologyNode* transformNode, IMFTopologyNode** outputNode);
		int createNewMediaFormat(IMFTopology* topology, IMFTopologyNode* srcNode, IMFTopologyNode** node, IMFMediaType* mediaType);
		int handleMediaEvent();
		void shutdownSessions();

	private:		
		IMFMediaSession* mediaSession_;
		IMFMediaSource* mediaSource_;
		IMFMediaSink* sink_;
		wstring srcFile_;
		wstring dstFile_;
		
	};
}}