/****************************************************************************************************************

Filename	:	EncoderMain.cpp
Content		:	Encoder implementation for creating h264 format video with DX11 and Unity RenderTexture
Created		:	December 13, 2016
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/
#include "AudioCapture.h"
#include "NVEncoder.h"
#include "AMDEncoder.h"
#include "Muxing.h"
#include "Log.h"
#include <sstream>
#include <shlobj.h>
#include <KnownFolders.h>
#include <ctime>

using namespace FBCapture::Video;
using namespace FBCapture::Audio;
using namespace FBCapture::Mux;
using namespace FBCapture::Log;

class EncoderMain {

public:
	EncoderMain();
	virtual ~EncoderMain();

private:
	void deleteFiles();

public:
	// Class instances for capture and encode
	NVEncoder*  nvEncoder;
	AMDEncoder* amdEncoder;
	Muxing * muxing;
	AudioCapture* audioCapture;
	AudioEncoder* audioEncoder;

	ID3D11Device* device;

	bool amdDevice;  // When it's amd
	bool nvidiaDevice;  // When it's nvidia    
	bool checkedBackend;  // Set true after checking gpu
	bool isFlushed;  // Set true after flushing 	
	bool initializedAudio;  // Set true after audio initialization
	bool isLive;  // Check if it's Live or VOD mode
	bool needUpdateTime;  // Set true when need to update Unix time for unique live video and audio file names  
	bool stopEncProcess;  // Stop encoding process when any critical encoding process is failed

	// Strings for folder
	wstring liveFolder;		
	wstring videoH264;
	wstring prevVideoH264;
	string audioWAV;		
	string prevAudioWAV;
	string audioAAC;
	string prevAudioAAC;

	time_t unixTime;

public:
	bool checkGraphicsCard();
	vector<wstring> splitString(wstring& str);
};

EncoderMain::EncoderMain() : nvEncoder(NULL), amdEncoder(NULL), muxing(NULL), audioCapture(NULL), audioEncoder(NULL),
device(NULL), amdDevice(false), nvidiaDevice(false), checkedBackend(false), stopEncProcess(false),
isFlushed(false), initializedAudio(false), isLive(false), unixTime(0), needUpdateTime(true){ }

EncoderMain::~EncoderMain()
{
	if (muxing) {
		delete muxing;
		muxing = NULL;
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

	//if (isLive) {  // delete remained files
	//	deleteFiles();
	//}

	RELEASE_LOG();
}

void EncoderMain::deleteFiles()
{
	std::wstring folder = liveFolder;
	std::wstring pathWithfiles = folder + L"*.*";

	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFileW(pathWithfiles.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// If we have no error, loop through the files in this dir 
		BOOL bContinue = TRUE;
		int counter = 0;
		do
		{
			if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				pathWithfiles = folder + data.cFileName;
				if (!DeleteFileW(pathWithfiles.c_str())) {
					FindClose(hFind);
					DEBUG_ERROR("Failed to delete files");
					return;
				}
				++counter;
				bContinue = FindNextFile(hFind, &data);
			}
		} while (bContinue);
		FindClose(hFind); // Free the dir 
	}
}

vector<wstring> EncoderMain::splitString(wstring& str) 
{
	vector<wstring> stringArr;
	wstringstream stringStream(str); // Turn the string into a stream.
	wstring tok;

	while (getline(stringStream, tok, L'.')) {
		stringArr.push_back(tok);
	}

	return stringArr;
}

bool EncoderMain::checkGraphicsCard() 
{	
	int nvidiaVenderID = 4318;  // NVIDIA Vendor ID: 0x10DE	
	int amdVenderID1 = 4098;	// AMD Vendor ID: 0x1002
	int amdVenderID2 = 4130;	// AMD Vendor ID: 0x1022

	IDXGIAdapter1 * adapter;
	std::vector <IDXGIAdapter1*> adapters;
	IDXGIFactory1* factory = NULL;
	DXGI_ADAPTER_DESC1 adapterDescription;

	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory))) {
		DEBUG_ERROR("Failed to create DXGI factory object");
		return false;
	}

	for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapter->GetDesc1(&adapterDescription);
		if (adapterDescription.VendorId == nvidiaVenderID) {
			nvidiaDevice = true;
			if (nvEncoder == NULL) {
				nvEncoder = new NVEncoder();
			}
		}
		else if (adapterDescription.VendorId == amdVenderID1 || adapterDescription.VendorId == amdVenderID2) {
			amdDevice = true;
			if (amdEncoder == NULL) {
				amdEncoder = new AMDEncoder();
			}
		}	
	}
	
	if(!nvidiaDevice && !amdDevice) {
		DEBUG_ERROR("Unsupported graphics card. The SDK supports only nVidia and AMD GPUs");
		return false;
	}

	if (muxing == NULL) {
		muxing = new Muxing();
	}

	if (audioCapture == NULL) {
		audioCapture = new AudioCapture();
	}

	if (audioEncoder == NULL) {
		audioEncoder = new AudioEncoder();
	}

	if (factory) {
		factory->Release();
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
	}
	else {
		liveFolder.clear();  // Just use root folder when it failed to create folder
	}

	unixTime = std::time(0);  // Using unix time for live video files' identity		

	return true;
}

EncoderMain encoderMain;

#define DllExport __declspec (dllexport)
extern "C"	DllExport void startEncoding(const void* texturePtr, const TCHAR* fullSavePath, bool isLive, int fps, bool needFlipping)
{	
	if (encoderMain.stopEncProcess)
		return;

	encoderMain.isLive = isLive;

	if (!encoderMain.checkedBackend) {
		if (!encoderMain.checkGraphicsCard()) {
			DEBUG_ERROR("Failed on getting available graphics card");
			return;
		}

		if (encoderMain.nvidiaDevice) {  // Only nvidia encoder sdk needs to pass d3d device pointer got from Unity
			encoderMain.nvEncoder->setGraphicsDeviceD3D11(encoderMain.device);
		}

		encoderMain.checkedBackend = true;
	}
	
	if (!encoderMain.isLive) {  // VOD mode
		encoderMain.videoH264 = fullSavePath;
	}		
	else {  // Live mode
		encoderMain.videoH264 = encoderMain.liveFolder + to_wstring(encoderMain.unixTime) + L".h264";
	}		

	// Encoding with render texture
	if (texturePtr && encoderMain.nvidiaDevice) {
		if (!encoderMain.nvEncoder->encodeMain(texturePtr, encoderMain.videoH264, fps, needFlipping)) {
			encoderMain.stopEncProcess = true;  
			return;
		}
	}
	else if (texturePtr && encoderMain.amdDevice) {
		if (!encoderMain.amdEncoder->encodeMain(texturePtr, encoderMain.videoH264, fps, needFlipping)) {
			encoderMain.stopEncProcess = true; 
			return;
		}
	}	
}

extern "C"	DllExport void audioEncoding()
{
	if (!encoderMain.checkedBackend || encoderMain.stopEncProcess)
		return;	

	if (!encoderMain.initializedAudio && !encoderMain.isLive) {  // VOD mode
		encoderMain.audioWAV = "encodedAudio.wav";
		encoderMain.audioAAC = "encodedAudio.aac";
		
		if (encoderMain.audioCapture->initializeAudio(encoderMain.audioWAV)) {
			encoderMain.initializedAudio = true;
		}
		else {
			encoderMain.stopEncProcess = true;
		}
	}
	else if (!encoderMain.initializedAudio && encoderMain.isLive) {  // Live mode		
		wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
		string folderPath = stringTypeConversion.to_bytes(encoderMain.liveFolder);
		encoderMain.audioWAV = folderPath + to_string(encoderMain.unixTime) + ".wav";		
		encoderMain.audioAAC = folderPath + to_string(encoderMain.unixTime) + ".aac";

		if (encoderMain.audioCapture->initializeAudio(encoderMain.audioWAV)) {
			encoderMain.initializedAudio = true;
		}
		else {
			encoderMain.stopEncProcess = true; 
			return;
		}
	}	

	if(encoderMain.initializedAudio){  // Capturing audio
		encoderMain.audioCapture->startAudioCapture();
	}	
}

extern "C"	DllExport void stopEncoding()
{	
	if (!encoderMain.checkedBackend || encoderMain.stopEncProcess) // Only we can flush buffers after encoding starts
		return;

	encoderMain.needUpdateTime = true;

	encoderMain.audioCapture->stopAudioCapture();

	// Flushing inputs
	if (encoderMain.nvidiaDevice) {
		if (!encoderMain.nvEncoder->flushEncodedImages()) {
			encoderMain.stopEncProcess = true;
			return;
		}
	}
	else if (encoderMain.amdDevice) {
		if (!encoderMain.amdEncoder->flushEncodedImages()) {
			encoderMain.stopEncProcess = true;
			return;
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
		encoderMain.audioWAV = folderPath + to_string(encoderMain.unixTime) + ".wav";
		encoderMain.audioAAC = folderPath + to_string(encoderMain.unixTime) + ".aac";
		encoderMain.videoH264 = encoderMain.liveFolder + to_wstring(encoderMain.unixTime) + L".h264";
	}

	encoderMain.initializedAudio = false;
	encoderMain.isFlushed = true;
}

extern "C"	DllExport void muxingData()
{
	if (!encoderMain.checkedBackend || !encoderMain.isFlushed || encoderMain.stopEncProcess)
		return;

	// Transcoding wav to aac
	if (encoderMain.audioEncoder->audioTranscoding(encoderMain.prevAudioWAV, encoderMain.prevAudioAAC) < 0) {
		encoderMain.stopEncProcess = true;
		return;
	}

	// Muxing
	if (encoderMain.muxing->muxingMedia(encoderMain.prevVideoH264, encoderMain.prevAudioAAC, encoderMain.isLive) < 0) {
		encoderMain.stopEncProcess = true;
		return;
	}

	encoderMain.isFlushed = false;
}

extern "C" DllExport void saveScreenShot(const void* texturePtr, const TCHAR* fullSavePath, bool needFlipping)
{
	if (!encoderMain.checkedBackend) {
		encoderMain.checkGraphicsCard();

		if (encoderMain.nvidiaDevice) {  // Only nvidia encoder sdk needs to pass d3d device pointer got from Unity
			encoderMain.nvEncoder->setGraphicsDeviceD3D11(encoderMain.device);
		}
		encoderMain.checkedBackend = true;
	}

	if (_tcslen(fullSavePath) == 0) {  //No input file path for screenshot
		DEBUG_ERROR("Didn't put screenshot file name");
		return;
	}

	if (texturePtr && encoderMain.nvidiaDevice) {
		encoderMain.nvEncoder->saveScreenShot(texturePtr, fullSavePath, needFlipping);
	}
	else if (texturePtr && encoderMain.amdDevice) {
		encoderMain.amdEncoder->saveScreenShot(texturePtr, fullSavePath, needFlipping);
	}

	DEBUG_LOG("Screenshot has successfully finished");
}

// TODO: T16054461 Will reactivate when live is ready
//extern "C" DllExport void startLiveStream(TCHAR* streamSeverURL, TCHAR* streamKey)
//{
//	wstring outputFile;
//	if (encoderMain.libRTMP && encoderMain.isLive) {
//		vector<wstring> pathArr = encoderMain.splitString(encoderMain.prevVideoH264);
//		outputFile.clear();
//		for (int i = 0; i < pathArr.size() - 1; ++i)
//			outputFile += pathArr[i];
//		outputFile += L".flv"; // Change to live file format
//
//		encoderMain.libRTMP->connectRTMPWithFlv(streamSeverURL, streamKey, outputFile.c_str());
//	}	
//	else {
//		DEBUG_LOG("Need to enable live streaming option in Unity script");
//	}
//}
//
//extern "C" DllExport void stopLiveStream()
//{
//	if (encoderMain.libRTMP) {
//		encoderMain.libRTMP->close();
//	}
//}

extern "C" DllExport void releaseEncoderResources()
{
	encoderMain.~EncoderMain();
}


extern "C" DllExport void UnitySetGraphicsDevice(void* device, int deviceType, int eventType)
{
	// D3D11 device, remember device pointer and device type.
	// The pointer we get is ID3D11Device.
	if (deviceType == kGfxRendererD3D11 && device != NULL) {
		encoderMain.device = (ID3D11Device*)device;
	}
}

