# 360-Capture-SDK
This is a developer focused SDK that allows game and virtual reality developers to be able to easily and quickly integrate 360 photo/video capture capability into their applications.

# Core Features Supported
This SDK enables you to capture, record, and encode 360 photos and videos with the relevant metadata necessary for detection. 

For photo capture, it will save the captured 360 photo JPEG to a folder on disk with relevant photosphere metadata added.
For video capture, it will save the captured 360 video MP4 to a folder on disk (metadata coming soon, for now you can inject the metadata manually).

We record the default audio output from speakers and mux it with the 360 video captured on screen to create an output mp4.

# Game Engine Compatibility
Our SDK solution can be integrated into 
1. Unity
2. Unreal
3. Custom native engines

# Hardware Compatibility
We support both AMD and NVIDIA GPU hardware in terms of compatibility with this SDK. For details, please see the below detailed specs.

## Detailed Hardware Compatibility
* **NVidia**
    * Operating System (x64 only): Windows 8, 10, Server 2008 R2, Server 2012
    * GPU: NVIDIA Quadro, Tesla, GRID or GeForce products with Kepler, Maxwell and Pascal generation GPUs. 
    * NVidia Windows display driver: 375.95 or newer


* **AMD**
    * Operating System (x64 only): Windows 8, 10
    * GPU: AMD GPUs supporting following driver
    * AMD Windows display driver: AMD Radeon Software Crimson 17.1.1 or later

# Supported Resolutions for Capture Output
We've tested and found our SDK can capture 4K 360 videos in realtime. You can adjust the resolution capture settings for photos and videos to test how it works with your title.

# How to integrate

## Unity
We've provided a sample integration into Unity showing how you may use the SDK to capture and record 360 photos/videos with a simple scene.

Once you open the Unity sample project in Unity, you will be able to find “EncodePackage” and it includes everything we need for 360 and rectilinear capturing and encoding. To set up encode environment, you need to place the EncoderObject prefab in the scene where you want to use it as a spectator.
Then you can select option in ‘CaptureOption.cs’ to determine whether to capture in 360 or as a standard rectilinear.

The APIs in Unity Script layer are pretty simple regardless of what GPU you are using.

The plugin provides two functions in Unity script layer for video encoding and the functions will call the proper codec APIs in backend depending on GPUs. All functions needed for encoding are managed by the backend. 

Basically, the video is encoded with a RenderTexture and we need to attach the RenderTexture as a target texture on the camera of child object in the EncoderObject prefab. We can place the plugin position where we want to use as spectator. If we want to change video resolution, it aligns with “External Width” and “External Height” values in LiveSurroundCapture.cs and NonSurroundCapture.cs.

For Audio, we need to record consistently while game is running and we must call the audioEncoding function every frame to keep writing audio data into buffer.
```
private static extern void audioEncoding();
```
The startEncoding function is for passing RenderTexture to native encoder layer for encoding video. If we want to encode video with 30 fps, we need to set calling it by 30 fps as implemented in “SurroundCapture.cs” and “NonSurroundCapture.cs”.
```
private static extern void startEncoding(IntPtr texture, String path, bool isLive, bool needsFlipping);
```
The stopEncoding function is for flushing all inputs which were stacked in input buffers and it needs to be called once when we want to stop encoding
```
private static extern void stopEncoding();
```
The muxingData function is for muxing audio and video into mp4 and final video file is generated with the function. It should be called after stopEncoding function.
```
private static extern void muxingData();
```
The saveScreenShot is for taking screenshot. If you want to take a screenshot, just call this function anytime.
```
private static extern void saveScreenshot(IntPtr texture, string path, bool needsFlipping);
```
## Native Engine Integration
First we need to pass dx11 device pointer to dll and it will be used for setting the relevant encoding SDK for your hardware (AMD vs NVIDIA).
```
/**
	* Allocate ID3D11Device got from application to pass video encoder apis
	* - It needs to be called before startEncoding() as one part of initialization
	*/
	DllExport void SetGraphicsDevice(ID3D11Device* device);
```
As mentioned in above Unity part, we want to call AudioEncoding() function every frame by the same token.
```
/**
	* Capture audio
	* - It needs to be called every frame
	* - It will generate wav file
	*/
	DllExport void AudioEncoding();
```
This function is for capturing screen and we need to pass texture pointer(void pointer) based on 30 fps as we want to get 30 fps movie. The texture needs to be the content you want to encode as a video, so it should already be in the format of a 360 video (equirectangular or cubemap). See details below for texture formats and how to convert them.
```
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
	* @param needFlipping	  true if you want to flip pixels vertically
	*/
	DllExport void StartEncoding(const void* texturePtr, const TCHAR* fullSavePath, bool isLive, bool needFlipping);
```
It will stop encoding and create mp4 file by muxing h264 and aac file. We need to call this function when we want to stop encoding.
```
/**
	* Flushing all input video and audio datas and mux them into mp4
	* - It needs to be called when you want to finish encoding
	* - It will generate mp4 file
	*/
	DllExport void StopEncoding();
```
It will take screenshot and you should set file format as JPEG for the fullSavePath parameter. Note that metadata for 360 image recognition in FB feed is injected by default. So you can upload the image in FB as 360 image.
```
/**
	* Take Screenshot
	* - File format should be JPEG
	* @param texturePtr       ID3D11Texture2D that you want to capture and encode to jpeg
	* @param fullSavePath     image save folder path including file name
	* @param needFlipping	  true if you want to flip pixels vertically
	*/
	DllExport void SaveScreenShot(const void* texturePtr, const TCHAR* fullSavePath, bool needFlipping);
};
```
# Building FBCapture.dll source
Please refer to: https://github.com/facebook/360-Capture-SDK/releases for instructions on steps one needs to follow for building the dll on their own.

# Join the 360 Capture SDK community

Please use our issues page to let us know of any problems or feedback. If you are working on a project using the 360 Capture SDK, please reach out to cg439/cpgupta@fb.com for potential opportunities to feature your work as created by the SDK.

# License

360 Capture SDK is BSD-licensed. We also provide an additional patent grant.
