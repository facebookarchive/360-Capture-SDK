/****************************************************************************************************************

Filename	:	AudioEncoder.h
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee, Dennis Won

Copyright	:

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
#include "ErrorCodes.h"

#define SAFE_RELEASE(a) if (a) { a->Release(); a= NULL; }

using namespace std;
using namespace std::chrono;
using namespace FBCapture::Common;

namespace FBCapture {
  namespace Audio {

    class AudioEncoder {

    public:
      AudioEncoder();
      virtual ~AudioEncoder();
      FBCAPTURE_STATUS audioTranscoding(const string& srcFile, const string& dstFile);

    private:
      FBCAPTURE_STATUS addSourceNode(IMFTopology* topology, IMFPresentationDescriptor* presDesc, IMFStreamDescriptor* streamDesc, IMFTopologyNode** node);
      FBCAPTURE_STATUS addTransformNode(IMFTopology* topology, IMFTopologyNode* srcNode, IMFStreamDescriptor* streamDesc, IMFTopologyNode** node);
      FBCAPTURE_STATUS addOutputNode(IMFTopology* topology, IMFTopologyNode* transformNode, IMFTopologyNode** outputNode);
      FBCAPTURE_STATUS createNewMediaFormat(IMFTopology* topology, IMFTopologyNode* srcNode, IMFTopologyNode** node, IMFMediaType* mediaType);
      FBCAPTURE_STATUS handleMediaEvent();
      void shutdownSessions();

    private:
      IMFMediaSession* mediaSession_;
      IMFMediaSource* mediaSource_;
      IMFMediaSink* sink_;
      wstring srcFile_;
      wstring dstFile_;

    };
  }
}
