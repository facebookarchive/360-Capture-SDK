/****************************************************************************************************************

Filename	:	EncoderMain.cpp
Content		:	Encoder implementation for creating h264 format video with DX11 and Unity RenderTexture
Created		:	December 13, 2016
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/
#include "LibRTMP.h"
#include "AudioCapture.h"
#include "NVEncoder.h"
#include "AMDEncoder.h"
#include "FLVmuxer.h"
#include "MP4muxer.h"
#include "Log.h"
#include "ErrorCodes.h"
#include <sstream>
#include <shlobj.h>
#include <KnownFolders.h>
#include <ctime>

using namespace FBCapture::Video;
using namespace FBCapture::Audio;
using namespace FBCapture::Mux;
using namespace FBCapture::Common;

class EncoderMain {

public:
  EncoderMain();
  virtual ~EncoderMain();

public:
// Class instances for capture and encode
  NVEncoder*  nvEncoder;
  AMDEncoder* amdEncoder;
  FLVMuxer * flvMuxer;
  MP4Muxer * mp4Muxer;
  AudioCapture* audioCapture;
  AudioEncoder* audioEncoder;
  LibRTMP* rtmp;

  ID3D11Device* device;

  enum GraphicsCardType {
    NVIDIA,
    AMD,
    UNKNOWN
  };

  bool amdDevice;  // When it's amd
  bool nvidiaDevice;  // When it's nvidia
  bool isLive;  // Check if it's Live or VOD mode
  volatile bool initialized;  // Set true after checking gpu and intilizing all modules
  volatile bool isFlushed;  // Set true after flushing
  volatile bool initializedAudio;  // Set true after audio initialization
  volatile bool needUpdateTime;  // Set true when need to update Unix time for unique live video and audio file names
  volatile bool stopEncProcess;  // Stop encoding process when any critical encoding process is failed
  volatile bool isMuxed; // Set to be true after muxing
  volatile bool readyAudioEncoding; // Set to be true after starting encoding
  GraphicsCardType graphicsCardType; // GPU graphics card type

                                     // Strings for folder
  wstring liveFolder;
  wstring videoH264;
  wstring prevVideoH264;
  string audioWAV;
  string prevAudioWAV;
  string audioAAC;
  string prevAudioAAC;

  int c;

  const string wavExtension;
  const string aacExtension;
  const wstring h264Extension;
  const wstring flvExtension;

  time_t unixTime;

public:
  bool initialize();
  vector<wstring> splitString(wstring& str);
};

EncoderMain::EncoderMain() : nvEncoder(NULL), amdEncoder(NULL), flvMuxer(NULL), mp4Muxer(NULL), audioCapture(NULL), audioEncoder(NULL), rtmp(NULL),
device(NULL), amdDevice(false), nvidiaDevice(false), initialized(false), stopEncProcess(false), readyAudioEncoding(false), graphicsCardType(GraphicsCardType::UNKNOWN),
isFlushed(false), initializedAudio(false), isLive(false), unixTime(0), needUpdateTime(true), wavExtension(".wav"), aacExtension(".aac"), h264Extension(L".h264"), flvExtension(L".flv") {}

EncoderMain::~EncoderMain() {
  if (flvMuxer) {
    delete flvMuxer;
    flvMuxer = NULL;
  }

  if (mp4Muxer) {
    delete mp4Muxer;
    mp4Muxer = NULL;
  }

  if (audioEncoder) {
    delete audioEncoder;
    audioEncoder = NULL;
  }

  if (audioCapture) {
    delete audioCapture;
    audioCapture = NULL;
  }

  if (nvEncoder) {
    delete nvEncoder;
    nvEncoder = NULL;
  }

  if (amdEncoder) {
    delete amdEncoder;
    amdEncoder = NULL;
  }

  if (rtmp) {
    delete rtmp;
    rtmp = NULL;
  }

  RELEASE_LOG();
}

vector<wstring> EncoderMain::splitString(wstring& str) {
  vector<wstring> stringArr;
  wstringstream stringStream(str); // Turn the string into a stream.
  wstring tok;

  while (getline(stringStream, tok, L'.')) {
    stringArr.push_back(tok);
  }

  return stringArr;
}

bool EncoderMain::initialize() {
  if (graphicsCardType == GraphicsCardType::UNKNOWN) {
    DEBUG_ERROR("Unsupported graphics card. The SDK supports only nVidia and AMD GPUs");
    return false;
  }

  if (flvMuxer == NULL) {
    flvMuxer = new FLVMuxer();
  }

  if (mp4Muxer == NULL) {
    mp4Muxer = new MP4Muxer();
  }

  if (audioCapture == NULL) {
    audioCapture = new AudioCapture();
  }

  if (audioEncoder == NULL) {
    audioEncoder = new AudioEncoder();
  }

  if (rtmp == NULL) {
    rtmp = new LibRTMP();
  }

  // Using "%AppData%" folder for saving sliced live video files
  LPWSTR wszPath = NULL;
  HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &wszPath);
  if (SUCCEEDED(hr)) {
    liveFolder = wszPath;
    liveFolder += L"\\FBEncoder\\";
    if (!CreateDirectory(liveFolder.c_str(), NULL)) {
      if (ERROR_ALREADY_EXISTS == GetLastError()) {
        DEBUG_LOG("Folder is already existed");
      }
    }
  } else {
    liveFolder.clear();  // Just use root folder when it failed to create folder
  }

  unixTime = std::time(0);  // Using unix time for live video files' identity

  c = 0;

  return true;
}

EncoderMain encoderMain;

#define DllExport __declspec (dllexport)
extern "C" DllExport FBCAPTURE_STATUS startEncoding(const void* texturePtr, const TCHAR* fullSavePath, bool isLive, int bitrate, int fps, bool needFlipping) {
  FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

  if (encoderMain.stopEncProcess)
    return status;

  encoderMain.isLive = isLive;

  if (!encoderMain.initialized) {
    if (!encoderMain.initialize()) {
      DEBUG_ERROR("Failed on getting available graphics card");
      return FBCAPTURE_STATUS_ENCODE_INIT_FAILED;
    }

    if (encoderMain.nvidiaDevice) {  // Only nvidia encoder sdk needs to pass d3d device pointer got from Unity
      encoderMain.nvEncoder->setGraphicsDeviceD3D11(encoderMain.device);
    }

    encoderMain.initialized = true;
    encoderMain.readyAudioEncoding = true;
  }

  if (!encoderMain.isLive) {  // VOD mode
    encoderMain.videoH264 = fullSavePath;
  } else {  // Live mode
    encoderMain.videoH264 = encoderMain.liveFolder + to_wstring(encoderMain.unixTime) + encoderMain.h264Extension;
  }

  // Encoding with render texture
  if (texturePtr && encoderMain.nvidiaDevice) {
    status = encoderMain.nvEncoder->encodeMain(texturePtr, encoderMain.videoH264, bitrate, fps, needFlipping);
    if (status != FBCAPTURE_STATUS_OK) {
      encoderMain.stopEncProcess = true;
      return status;
    }
  } else if (texturePtr && encoderMain.amdDevice) {
    status = encoderMain.amdEncoder->encodeMain(texturePtr, encoderMain.videoH264, bitrate, fps, needFlipping);
    if (status != FBCAPTURE_STATUS_OK) {
      encoderMain.stopEncProcess = true;
      return status;
    }
  } else {
    DEBUG_LOG("It's invalid texture pointer: null");
  }

  TCHAR szMutexName[] = TEXT("mutex_write");
  TCHAR szSharedMemoryName[] = TEXT("Global\\Facebook360Capture_FileMappingObject");

  int		shmem_size = 16;  // 16byte
  HANDLE	shmem = INVALID_HANDLE_VALUE;
  HANDLE	mutex = INVALID_HANDLE_VALUE;

  mutex = CreateMutex(NULL, FALSE, szMutexName);

  shmem = CreateFileMapping(
    INVALID_HANDLE_VALUE,
    NULL,
    PAGE_READWRITE,
    0,
    shmem_size,
    szSharedMemoryName
  );

  char *buf = (char*)MapViewOfFile(shmem, FILE_MAP_ALL_ACCESS, 0, 0, shmem_size);

  // mutex lock
  WaitForSingleObject(mutex, INFINITE);

  // write shared memory
  memset(buf, encoderMain.c, shmem_size);
  encoderMain.c++;
  char msg[1024];
  sprintf(msg, "write shared memory...c=%d", encoderMain.c);
  DEBUG_LOG(msg);

  // mutex unlock
  ReleaseMutex(mutex);

  // release
  UnmapViewOfFile(buf);
  CloseHandle(shmem);
  ReleaseMutex(mutex);

  return status;
}

extern "C" DllExport FBCAPTURE_STATUS audioEncoding(/*const TCHAR* audioSource*/bool useRiftAudioEndpoint, bool silenceMode) {
  FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;
  if (!encoderMain.initialized || encoderMain.stopEncProcess || !encoderMain.readyAudioEncoding || encoderMain.videoH264.empty()) {
    return status;
  }

  if (!encoderMain.initializedAudio) {
    if (!encoderMain.isLive) {  // VOD mode
      wstring videoPath = encoderMain.videoH264;
      videoPath.erase(encoderMain.videoH264.length() - encoderMain.h264Extension.length(), encoderMain.h264Extension.length());

      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      string audioPath = stringTypeConversion.to_bytes(videoPath);

      encoderMain.audioWAV = audioPath + encoderMain.wavExtension;
      encoderMain.audioAAC = audioPath + encoderMain.aacExtension;
    } else {  // Live mode
      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      string folderPath = stringTypeConversion.to_bytes(encoderMain.liveFolder);
      encoderMain.audioWAV = folderPath + to_string(encoderMain.unixTime) + encoderMain.wavExtension;
      encoderMain.audioAAC = folderPath + to_string(encoderMain.unixTime) + encoderMain.aacExtension;
    }

    status = encoderMain.audioCapture->initializeAudio(encoderMain.audioWAV, useRiftAudioEndpoint);
    if (status == FBCAPTURE_STATUS_OK) {
      encoderMain.initializedAudio = true;
    } else {
      encoderMain.stopEncProcess = true;
      return status;
    }
  }

  // Capturing audio
  if (encoderMain.readyAudioEncoding) {
	  encoderMain.audioCapture->startAudioCapture(silenceMode);
  }

  return status;
}

extern "C" DllExport FBCAPTURE_STATUS stopEncoding() {
  FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

  // Only we can flush buffers after encoding starts
  if (!encoderMain.initialized || encoderMain.stopEncProcess)
    return status;

  encoderMain.readyAudioEncoding = false;
  encoderMain.needUpdateTime = true;

  status = encoderMain.audioCapture->stopAudioCapture();
  if (status != FBCAPTURE_STATUS_OK) {
    encoderMain.stopEncProcess = true;
    return status;
  }

  // Flushing inputs
  if (encoderMain.nvidiaDevice) {
    status = encoderMain.nvEncoder->flushEncodedImages();
    if (status != FBCAPTURE_STATUS_OK) {
      encoderMain.stopEncProcess = true;
      return status;
    }
  } else if (encoderMain.amdDevice) {
    status = encoderMain.amdEncoder->flushEncodedImages();
    if (status != FBCAPTURE_STATUS_OK) {
      encoderMain.stopEncProcess = true;
      return status;
    }
  }

  if (encoderMain.needUpdateTime) {  // Update unix time for next frame video and audio
    encoderMain.unixTime = std::time(0);
    encoderMain.needUpdateTime = !encoderMain.needUpdateTime;
  }

  // Store previous file name to use in muxing
  encoderMain.prevAudioWAV = encoderMain.audioWAV;
  encoderMain.prevAudioAAC = encoderMain.audioAAC;
  encoderMain.prevVideoH264 = encoderMain.videoH264;

  // Update file names for next slice
  if (encoderMain.isLive) {
    wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
    string folderPath = stringTypeConversion.to_bytes(encoderMain.liveFolder);
    encoderMain.audioWAV = folderPath + to_string(encoderMain.unixTime) + encoderMain.wavExtension;
    encoderMain.audioAAC = folderPath + to_string(encoderMain.unixTime) + encoderMain.aacExtension;
    encoderMain.videoH264 = encoderMain.liveFolder + to_wstring(encoderMain.unixTime) + encoderMain.h264Extension;
  }

  encoderMain.readyAudioEncoding = true;
  encoderMain.initializedAudio = false;
  encoderMain.isFlushed = true;
  encoderMain.stopEncProcess = false;

  return status;
}

extern "C" DllExport FBCAPTURE_STATUS muxingData() {
  FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

  if (!encoderMain.initialized || !encoderMain.isFlushed || encoderMain.stopEncProcess)
    return status;

  // Transcoding wav to aac
  status = encoderMain.audioEncoder->audioTranscoding(encoderMain.prevAudioWAV, encoderMain.prevAudioAAC);
  if (status != FBCAPTURE_STATUS_OK) {
    encoderMain.stopEncProcess = true;
    return status;
  }

  // Muxing
  if (encoderMain.isLive) {
    status = encoderMain.flvMuxer->muxingMedia(encoderMain.prevVideoH264, encoderMain.prevAudioAAC);
    if (status != FBCAPTURE_STATUS_OK) {
      encoderMain.stopEncProcess = true;
      return status;
    }
  } else {
    status = encoderMain.mp4Muxer->muxingMedia(encoderMain.prevVideoH264, encoderMain.prevAudioAAC);
    if (status != FBCAPTURE_STATUS_OK) {
      encoderMain.stopEncProcess = true;
      return status;
    }
  }

  encoderMain.isFlushed = false;
  encoderMain.isMuxed = true;

  return status;
}

extern "C" DllExport FBCAPTURE_STATUS saveScreenShot(const void* texturePtr, const TCHAR* fullSavePath, bool needFlipping) {
  FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

  if (!encoderMain.initialized) {
    encoderMain.initialize();

    if (encoderMain.nvidiaDevice) {  // Only nvidia encoder sdk needs to pass d3d device pointer got from Unity
      encoderMain.nvEncoder->setGraphicsDeviceD3D11(encoderMain.device);
    }
    encoderMain.initialized = true;
    encoderMain.readyAudioEncoding = true;
  }

  if (_tcslen(fullSavePath) == 0) {  //No input file path for screenshot
    DEBUG_ERROR("Didn't put screenshot file name");
    return FBCAPTURE_STATUS_NO_INPUT_FILE;
  }

  if (texturePtr && encoderMain.nvidiaDevice) {
    status = encoderMain.nvEncoder->saveScreenShot(texturePtr, fullSavePath, needFlipping);
    if (status != FBCAPTURE_STATUS_OK) {
      return status;
    }
  } else if (texturePtr && encoderMain.amdDevice) {
    status = encoderMain.amdEncoder->saveScreenShot(texturePtr, fullSavePath, needFlipping);
    if (status != FBCAPTURE_STATUS_OK) {
      return status;
    }
  } else if (texturePtr == NULL) {
    DEBUG_ERROR("Invalid render texture pointer(null)");
    return FBCAPTURE_STATUS_INVALID_TEXTURE_POINTER;
  }

  DEBUG_LOG("Screenshot has successfully finished");

  return status;
}

extern "C" DllExport FBCAPTURE_STATUS startLiveStream(TCHAR* streamUrl) {
  FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

  if (!encoderMain.initialized || !encoderMain.isMuxed || encoderMain.stopEncProcess || !encoderMain.isLive)
    return status;

  if (encoderMain.rtmp && encoderMain.isLive) {
    wstring liveVideoFile = encoderMain.prevVideoH264;
    liveVideoFile.erase(encoderMain.prevVideoH264.length() - encoderMain.h264Extension.length(), encoderMain.h264Extension.length());
    liveVideoFile += encoderMain.flvExtension;

    if (!liveVideoFile.empty() && (status = encoderMain.rtmp->connectRTMPWithFlv(streamUrl, liveVideoFile.c_str())) != FBCAPTURE_STATUS_OK) {
      return status;
    }
  } else {
    DEBUG_LOG("Need to enable live streaming option in Unity script");
  }

  return status;
}

extern "C" DllExport void stopLiveStream() {
  if (encoderMain.rtmp) {
    encoderMain.rtmp->close();
  }
}

extern "C" DllExport void resetResources() {
  encoderMain.initialized = false;
  encoderMain.stopEncProcess = false;
  encoderMain.readyAudioEncoding = false;
  encoderMain.isFlushed = false;
  encoderMain.initializedAudio = false;
  encoderMain.isLive = false;
  encoderMain.unixTime = 0;
  encoderMain.needUpdateTime = true;
}

extern "C" DllExport int getGraphicsCardType() {
  return encoderMain.graphicsCardType;
}

extern "C" DllExport void UnitySetGraphicsDevice(void* device, int deviceType, int eventType) {
  int nvidiaVenderID = 4318;  // NVIDIA Vendor ID: 0x10DE
  int amdVenderID1 = 4098;  // AMD Vendor ID: 0x1002
  int amdVenderID2 = 4130;  // AMD Vendor ID: 0x1022

                            // D3D11 device, remember device pointer and device type.
                            // The pointer we get is ID3D11Device.
  if (deviceType == kGfxRendererD3D11 && device != NULL) {
    encoderMain.device = (ID3D11Device*)device;
  }

  IDXGIAdapter1 * adapter;
  std::vector <IDXGIAdapter1*> adapters;
  IDXGIFactory1* factory = NULL;
  DXGI_ADAPTER_DESC1 adapterDescription;

  if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory))) {
    DEBUG_ERROR("Failed to create DXGI factory object");
    return;
  }

  for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    adapter->GetDesc1(&adapterDescription);
    if (adapterDescription.VendorId == nvidiaVenderID) {
      encoderMain.nvidiaDevice = true;
      encoderMain.graphicsCardType = encoderMain.NVIDIA;
      if (encoderMain.nvEncoder == NULL) {
        encoderMain.nvEncoder = new NVEncoder();
      }
    } else if (adapterDescription.VendorId == amdVenderID1 || adapterDescription.VendorId == amdVenderID2) {
      encoderMain.amdDevice = true;
      encoderMain.graphicsCardType = encoderMain.AMD;
      if (encoderMain.amdEncoder == NULL) {
        encoderMain.amdEncoder = new AMDEncoder();
      }
    }
  }

  if (factory) {
    factory->Release();
    factory = NULL;
  }
}
