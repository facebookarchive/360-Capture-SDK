/****************************************************************************************************************

Filename	:	Log.h
Content		:
Created		:	Feb 28, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/

#pragma once

#ifdef _WIN32
#include <Shlobj.h>
#endif

#include <comdef.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include "Common.h"


using namespace std;
#define DEBUG_LOG(message) \
	FBCapture::Common::EncoderLog::instance().log(message, FBCapture::Common::EncoderLog::kLog)

#define DEBUG_LOG_VAR(message, vars) \
	FBCapture::Common::EncoderLog::instance().log(message, vars, FBCapture::Common::EncoderLog::kLog)

#define DEBUG_ERROR(message) \
	FBCapture::Common::EncoderLog::instance().log(message, FBCapture::Common::EncoderLog::kError)

#define DEBUG_ERROR_VAR(message, vars) \
	FBCapture::Common::EncoderLog::instance().log(message, vars, FBCapture::Common::EncoderLog::kError)

#define RELEASE_LOG() \
	FBCapture::Common::EncoderLog::instance().release();

#define HRESULT_ERROR(hr) \
  to_string(hr)

#define DEBUG_HRESULT_ERROR(message, hr)                                       \
  DEBUG_ERROR_VAR(message, HRESULT_ERROR(hr))

namespace FBCapture {
  namespace Common {

    class EncoderLog {

    public:
      EncoderLog();
      virtual ~EncoderLog();
      static const std::string kLog;
      static const std::string kError;

      static EncoderLog& instance();

      void log(const std::string& log, const std::string& logType);
      void log(const std::string& log, const std::string& var, const std::string& logType);

    private:
      //static EncoderLog* kInstance;
      static std::unique_ptr<EncoderLog> kInstance;
      std::ofstream output_ = {};

      void logWriter(const std::string& log, const std::string& logType);
      void logWriter(const std::string& log, const std::string& var, const std::string& inLogLevel);
      string getCurrentTime();
    };
  }
}