/****************************************************************************************************************

Filename	:	Muxing.cpp
Content		:
Created		:
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#include "Muxing.h"

namespace FBCapture { namespace Mux {

	Muxing::Muxing() {}

	Muxing::~Muxing()
	{
		remove(videoFile_.c_str());  // Remove h264 video file
		remove(audioFile_.c_str());  // Remove aac audio file
	}

	string Muxing::changeFormatString(const string& path, const string& oldFormat, const string& newFormat) {
		string newPath( path );
		
		newPath.erase(path.length() - oldFormat.length(), oldFormat.length());
		newPath += newFormat;

		return newPath;
	}

	int Muxing::muxingMedia(const wstring& videoFile, const string& audioFile, bool isLive)
	{
		string outputfile;
		uint32_t nErrorCode = 0;

		wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
		videoFile_ = stringTypeConversion.to_bytes(videoFile);
		audioFile_ = audioFile;

		if (isLive) {
			string newformat("flv");
			string oldFormat("h264");
			outputfile = changeFormatString(videoFile_, oldFormat, newformat);
		}
		else {
			string newformat("mp4");
			string oldFormat("h264");
			outputfile = changeFormatString(videoFile_, oldFormat, newformat);			
			nErrorCode = muxAVStreams(audioFile_.c_str(),
				videoFile_.c_str(),
				outputfile.c_str(),
				0.0f,
				0.0f,
				NO_DURATION_SPECIFIED,
				UNKNOWN_FRAMES_PER_SECOND,
				(eVideoRotationMode)VIDEO_ROTATION_MODE_NO_ROTATION,
				false);

			if (0 != nErrorCode) {
				DEBUG_LOG_VAR("Muxing failed, error: ", to_string(nErrorCode));
			}
		}			

		remove(videoFile_.c_str());  // Remove h264 video file
		remove(audioFile_.c_str());  // Remove aac audio file
	
		return nErrorCode;
	}	
}}
