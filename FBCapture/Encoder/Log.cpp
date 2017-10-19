/****************************************************************************************************************

Filename	:	Log.cpp
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#include "Log.h"

namespace FBCapture {
  namespace Common {

    const string EncoderLog::kLog = "[LOG]";
    const string EncoderLog::kError = "[ERROR]";
    const char* EncoderLog::kLogFile = "FBCaptureSDK.txt";
    EncoderLog* EncoderLog::kInstance = nullptr;
    mutex EncoderLog::mutex_;


    EncoderLog& EncoderLog::instance() {
      lock_guard<mutex> lock(mutex_);
      if (kInstance == nullptr)
        kInstance = new EncoderLog();
      return *kInstance;
    }

    void EncoderLog::release() {
      lock_guard<mutex> lock(EncoderLog::mutex_);
      delete EncoderLog::kInstance;
      EncoderLog::kInstance = nullptr;
    }

    EncoderLog::~EncoderLog() {
      output_.close();
    }

    EncoderLog::EncoderLog() {
      output_.open(kLogFile, ios_base::out);
      if (!output_.good()) {
        throw runtime_error("Initialization is failed");
      }
    }

    void EncoderLog::log(const string& log, const string& logType) {
      lock_guard<mutex> lock(mutex_);
      logWriter(log, logType);
    }

    void EncoderLog::log(const string& log, const std::string& var, const string& logType) {
      lock_guard<mutex> lock(mutex_);
      logWriter(log, var, logType);
    }

    void EncoderLog::logWriter(const std::string& log, const std::string& logType) {
      output_ << getCurrentTime() << logType << ": " << log << endl;
    }

    void EncoderLog::logWriter(const std::string& log, const std::string& var, const std::string& logType) {
      output_ << getCurrentTime() << logType << ": " << log << ": " << var << endl;
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
