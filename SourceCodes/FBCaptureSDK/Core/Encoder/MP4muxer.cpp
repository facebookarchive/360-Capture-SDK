/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "MP4muxer.h"

namespace FBCapture {
	namespace Mux {
		const string kStitchingSoftware = "FBCAPTURESDK";

		MP4Muxer::MP4Muxer() {}

		MP4Muxer::~MP4Muxer() {
			remove(videoFile_.c_str());  // Remove h264 video file
			remove(audioFile_.c_str());  // Remove aac audio file
		}

		string MP4Muxer::replaceString(const string& inputStr, const string& searchStr, const string& replaceStr) {
			string newFullStr(inputStr);

			if (newFullStr.find(searchStr) != string::npos) {  // Convert file name to h264 when input name is mp4 format
				newFullStr.replace(newFullStr.find(searchStr), searchStr.length(), replaceStr);
			}

			return newFullStr;
		}

		FBCAPTURE_STATUS MP4Muxer::muxingMedia(const wstring& videoFile, const string& audioFile, PROJECTIONTYPE projectionType, STEREO_MODE stereoMode, bool is360) {
			FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
			Utils utils;
			Metadata md;

			string outputfile;
			const string kMP4Ext = ".mp4";
			const string kH264Ext = ".h264";
			const string kMetadataExt = "_injected.mp4";
			uint32_t nErrorCode = 0;

			wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
			videoFile_ = stringTypeConversion.to_bytes(videoFile);
			audioFile_ = audioFile;

			outputfile = replaceString(videoFile_, kH264Ext, kMP4Ext);
			nErrorCode = mp4muxAVStreams(audioFile_.c_str(),
																	 videoFile_.c_str(),
																	 outputfile.c_str(),
																	 0.0f,
																	 0.0f,
																	 NO_DURATION_SPECIFIED,
																	 UNKNOWN_FRAMES_PER_SECOND,
																	 (eVideoRotationMode)VIDEO_ROTATION_MODE_NO_ROTATION,
																	 false);

			if (0 != nErrorCode) {
				DEBUG_ERROR_VAR("Muxing failed, error: ", to_string(nErrorCode));
				status = FBCAPTURE_STATUS_WAMDEIA_MUXING_FAILED;
			}

			// After muxing, inject spherical video metadata for 360 video			
				auto& strVideoXml =
					utils.generate_spherical_xml(static_cast<projection>(projectionType), static_cast<enMode>(stereoMode), kStitchingSoftware, nullptr);

				if (strVideoXml.length() < 1) {
					DEBUG_ERROR("INVALID XML");
					return FBCAPTURE_STATUS_METADATA_INVALID_XML;
				}

				md.setVideoXML(strVideoXml);
				
			// We want to generate additional file appending METADATA SUFFIX on input file name for 360 video by default			
				if (is360) {  
#if APPEND_METADATA_SUFFIX
					string injectedOutputFile(replaceString(outputfile, kMP4Ext, kMetadataExt));
					utils.inject_metadata(outputfile, injectedOutputFile, &md);
#else
					string injectedOutputFile(replaceString(outputfile, kMP4Ext, kMetadataExt));
					utils.inject_metadata(outputfile, injectedOutputFile, &md);
					remove(outputfile.c_str());
					rename(injectedOutputFile.c_str(), outputfile.c_str());
#endif
			}

			remove(videoFile_.c_str());  // Remove h264 video file
			remove(audioFile_.c_str());  // Remove aac audio file

			return status;
		}
	}
}
