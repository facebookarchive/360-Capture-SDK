/****************************************************************************************************************

Filename	:	EncoderInterface.h
Content		:	Encoder implementation for creating video
Created		:	May 18, 2017

****************************************************************************************************************/

#pragma once
#include <d3d11.h>

// How to load the DLL
/*
sample.cpp 

typedef EncoderInterface* (__cdecl *encoderFactory)();

int main(){
HINSTANCE dllHandle = ::LoadLibrary(TEXT("FBCapture.dll"));
if (!dllHandle) {
	cout << "Failed to load DLL";
	return -1;
}

// Get the function from the DLL
encoderFactory factory = reinterpret_cast<encoderFactory>(::GetProcAddress(dllHandle, "createEncoder"));
if (!factory) {
	cout << "Failed to load createEncoder from DLL";
	::FreeLibrary(dllHandle);
	return -1;
}

// Access to interfaces. Need to call functions based on guideline
EncoderInterface* instance = factory();

instance->SetGraphicsDevice(d3d11_DevicePtr);
instance->StartEncoding(...);
instance->AudioEncoding();
instance->StopEncoding();
instance->MuxingData();
instance->SaveScreenShot(...)

// Destroy explicitly
instance->destroy();
::FreeLibrary(dllHandle);
}
*/

class EncoderInterface {

public:
	/**
	* Allocate ID3D11Device got from application to pass video encoder apis
	* - It needs to be called before startEncoding() as one part of initialization
	*/
	virtual void SetGraphicsDevice(ID3D11Device* device) = 0;

	/**
	* Capture Screen and encode to h264
	* - It needs to be regularly called based on 30 fps
	*  ex)
	*      float fps = 1f / 30.0f;
	*      fpsTimer += Time.deltaTime;
	*      if(fpsTimer >= fps){ fpsTimer = 0.0f;  startEncoding(...); }
	*
	* @param texturePtr       ID3D11Texture2D to encode h264
	* @param fullSavePath     video save folder path including file name
	* @param isLive			  Select whether to be live or VOD. (It should be false now since live isn't support yet)
	* @param fps			  Set video fps and encourage setting it to 30 fps
	* @param needFlipping	  true if you want to flip pixels vertically
	*/
	virtual bool StartEncoding(const void* texturePtr, const TCHAR* fullSavePath, bool isLive, int fps, bool needFlipping) = 0;

	/**
	* Capture audio
	* - It needs to be called every frame
	* - It will generate wav file
	*/
	virtual bool AudioEncoding() = 0;

	/**
	* Flushing all input video and audio data
	* - It needs to be called when you want to finish encoding
	*/
	virtual bool StopEncoding() = 0;

	/**
	* Mux audio and video with audio transcoding(wav->aac)
	* - It needs to be called right after StopEncoding
	* - It will generate .mp4 which is final file format
	*/
	virtual bool MuxingData() = 0;

	/**
	* Take Screenshot
	* - File format should be JPEG
	* @param texturePtr       ID3D11Texture2D that you want to capture and encode to jpeg
	* @param fullSavePath     image save folder path including file name
	* @param needFlipping	  true if you want to flip pixels vertically
	*/
	virtual bool SaveScreenShot(const void* texturePtr, const TCHAR* fullSavePath, bool needFlipping) = 0;

	/**
	* Destroy instance created for accessing interfaces
	*/
	virtual void destroy() = 0;
};
