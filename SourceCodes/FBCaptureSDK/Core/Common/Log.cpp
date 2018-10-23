/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "Log.h"
#include <atlstr.h>

namespace FBCapture {
  namespace Common {

    const string EncoderLog::kLog = "[INFO]";
    const string EncoderLog::kError = "[ERROR]";
    std::unique_ptr<EncoderLog> EncoderLog::kInstance = {};
		
    EncoderLog& EncoderLog::instance() {
      if (kInstance == nullptr) {
        kInstance = std::make_unique<EncoderLog>();
      }
      return *kInstance;
    }   

    EncoderLog::~EncoderLog() {
      output_.close(); 
    }

    EncoderLog::EncoderLog() {
      // Save logfile to %LOCALAPPDATA%\FBCapture\FBCaptureSDK_processname_timestamp.txt
      PWSTR localAppPath = nullptr;
      HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &localAppPath);
      if (FAILED(hr)) {
        throw runtime_error("Unable to locate LocalAppData");
      }
      std::wstring logFile = localAppPath;
      CoTaskMemFree(localAppPath);

      // Creates "\FBCapture" subdirectory if it doesn't exist
      logFile += L"\\FBCapture";
      if (_wmkdir(logFile.c_str()) != 0 && errno != EEXIST) {
        throw runtime_error("Unable to create FBCapture subdirectory in LocalAppData: " + std::to_string(errno));
      }

      // Create unique log file name with timestamp and process name
      TCHAR szFileName[MAX_PATH + 1] = {};
      wstring processName = {};

      if (GetModuleFileName(nullptr, szFileName, MAX_PATH + 1)) {
        processName = PathFindFileNameW(szFileName);
        wstring extention = L".exe";
        wstring::size_type sizeType = processName.find(extention);

        if (sizeType != wstring::npos) processName.erase(sizeType, extention.length());
        logFile += L"\\FBCaptureSDK_" + processName;
      } else {
        logFile += L"\\FBCaptureSDK";
      }

      static wchar_t timestamp[100] = {};      
      time_t now = time(0);

      if (wcsftime(timestamp, sizeof(timestamp), L"%Y%m%d_%H%M%S", localtime(&now))) {
        logFile = logFile + L"_" + timestamp + L".txt";
      } else {
        logFile += L".txt";
      }
      output_.open(logFile.c_str(), ios_base::out);
      if (!output_.good()) {
        throw runtime_error("Initialization is failed");
      }
    }

    void EncoderLog::log(const string& log, const string& logType) {
      logWriter(log, logType);
    }

    void EncoderLog::log(const string& log, const std::string& var, const string& logType) {
      logWriter(log, var, logType);
    }

    void EncoderLog::logWriter(const std::string& log, const std::string& logType) {
      output_ << getCurrentTime() << logType << " " << log << endl;
    }

    void EncoderLog::logWriter(const std::string& log, const std::string& var, const std::string& logType) {
      output_ << getCurrentTime() << logType << " " << log << ": " << var << endl;
    }

    string EncoderLog::getCurrentTime() {
      auto now = std::chrono::system_clock::now();
      auto time = std::chrono::system_clock::to_time_t(now);

      std::stringstream timeStamp;
      timeStamp << std::put_time(std::localtime(&time), "[ %Y-%m-%d %X ]");
      return timeStamp.str();
    }
  }
}
