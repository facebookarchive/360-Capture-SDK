/****************************************************************************************************************

Filename	:	Log.cpp
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#ifdef _WIN32
#include <Shlobj.h>
#endif

#include "Log.h"

namespace FBCapture {
  namespace Common {

    const string EncoderLog::kLog = "[INFO]";
    const string EncoderLog::kError = "[ERROR]";
    EncoderLog* EncoderLog::kInstance = nullptr;
		
    EncoderLog& EncoderLog::instance() {
      if (kInstance == nullptr)
        kInstance = new EncoderLog();
      return *kInstance;
    }

    void EncoderLog::release() {
      delete EncoderLog::kInstance;
      EncoderLog::kInstance = nullptr;
    }

    EncoderLog::~EncoderLog() {
      output_.close();
    }

    EncoderLog::EncoderLog() {
      // Save logfile to %LOCALAPPDATA%\FBCapture\FBCaptureSDK.txt
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

      logFile += L"\\FBCaptureSDK.txt";
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
