/****************************************************************************************************************

Filename	:	Log.h
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/

#pragma once
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>
#include <ctime>
#include <sstream>
#include <iomanip>

using namespace std;
#define DEBUG_LOG(message) \
	EncoderLog::instance().log(message, EncoderLog::kLog)

#define DEBUG_LOG_VAR(message, vars) \
	EncoderLog::instance().log(message, vars, EncoderLog::kLog)

#define DEBUG_ERROR(message) \
	EncoderLog::instance().log(message, EncoderLog::kError)

#define DEBUG_ERROR_VAR(message, vars) \
	EncoderLog::instance().log(message, vars, EncoderLog::kError)

#define RELEASE_LOG() \
	EncoderLog::instance().release();

namespace FBCapture {
  namespace Common {

    class EncoderLog {
    private:
      EncoderLog();
      virtual ~EncoderLog();
      EncoderLog(const EncoderLog&);
      static std::mutex mutex_;

    public:
      static const std::string kLog;
      static const std::string kError;

      static EncoderLog& instance();

      void log(const std::string& log, const std::string& logType);
      void log(const std::string& log, const std::string& var, const std::string& logType);
      void release();

    private:
      static EncoderLog* kInstance;
      static const char* kLogFile;
      std::ofstream output_;

      void logWriter(const std::string& log, const std::string& logType);
      void logWriter(const std::string& log, const std::string& var, const std::string& inLogLevel);
      string getCurrentTime();
    };
  }
}
