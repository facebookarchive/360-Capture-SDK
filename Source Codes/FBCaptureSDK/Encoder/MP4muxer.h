/****************************************************************************************************************

Filename	:	Muxing.h
Content		:
Created		:
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#pragma once
#include "mp4muxing.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <codecvt>
#include "Log.h"
#include "Common.h"

#include "spatialmedia/metadata_utils.h"

using namespace std;
using namespace libmp4operations;
using namespace FBCapture::Common;
using namespace VrDeviceType;
using namespace StereoMode;
using namespace ProjectionType;

#define APPEND_METADATA_SUFFIX 1

namespace FBCapture {
	namespace Mux {
		class MP4Muxer {
		public:
			MP4Muxer();
			virtual ~MP4Muxer();

		private:
			int frameIndex;

			string videoFile_;
			string audioFile_;

		private:
			string replaceString(const string& inputStr, const string& searchStr, const string& replaceStr);

		public:
			FBCAPTURE_STATUS muxingMedia(const wstring& videoFile, const string& audioFile, PROJECTIONTYPE projectionType, STEREO_MODE stereoMode, bool is360);
		};
	}
}
