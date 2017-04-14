/****************************************************************************************************************

Filename	:	Muxing.h
Content		:
Created		:
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#pragma once

#include "mp4muxing.h"
#include "logsrecipient.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <codecvt>
#include "windows.h"
#include "Log.h"

using namespace FBCapture::Log;
using namespace std;
using namespace libwamediacommon;
using namespace libmp4operations;

namespace FBCapture { namespace Mux {
	class Muxing {
	public:
		Muxing();
		virtual ~Muxing();

	private:
		int frameIndex;

		string videoFile_;
		string audioFile_;

	private:
		string changeFormatString(const string& path, const string& oldFormat, const string& newFormat);

	public:
		int muxingMedia(const wstring& videoFile, const string& audioFile, bool isLive);
	};
}}
