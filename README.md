# FBCAPTURE SDK 2.2 DOCUMENTATION

## Release Note

* RGB-D Capture Support
    * We're starting to support RGB-D capture for euqirectangular videos. You will be able to see recorded RGB-D video with RGB-D video player. 
* Metadata Injection for recorded 360 video. 
    * Another file named “xxxx_injected.mp4” will be generated for metadata injected video file. You can upload 360 video to FB or Youtube without any additional work.
* More simple APIs and integration process
    * All threads for encoding and screenshot are now managed in FBCapture SDK
    * Single APIs will handle start and stop encoding
* Webcam Capture Support with overlay
    * You're able to capture webcam and overlay it on the captured video
* Windows 7 Support (SP1 or greater)
* 32 Bit Support
* Bug fixes, performance improvement and memory optimizations
    * Improved many parts of performance including async texture encoding, memory, and stability on live and vod we got on FBCapture SDK 1.0

## Compatibility

 1. Hardware AND Software Compatibility

* NVidia
    * GPU: NVIDIA Quadro, Tesla, GRID or GeForce products with Kepler, Maxwell and Pascal generation GPUs.
    * NVidia Windows display driver: 375.95 or newer
* AMD
    * GPU: AMD GPUs supporting following driver
    * AMD Windows display driver: AMD Radeon Software Crimson 17.1.1 or later
* OS
    *  Windows 7 SP1 or Greater

## Download

1. Unity Full Sample Project Download: 
https://github.com/facebook/360-Capture-SDK/tree/master/Samples/Unity

2. FBCapture SDK 2.0 Unity Package Download: 
https://github.com/facebook/360-Capture-SDK/blob/master/FBCaptureSDK.unitypackage

3. FBCapture SDK 2.0 Full Source Code:
https://github.com/facebook/360-Capture-SDK/releases/tag/v2.01

## FBCAPTURE SDK 2.0 Unity Integration Guide

1. **Integration Guide**
   1) DLL copy
       1) Download attached DLLs in this page and copy them into plugin folders in the Sample Project
   2) Unity package import
       1) Import FBCapture SDK Unity package into your Unity project
       2)  Drag and drop FBCapture prefab into your scene
       3) That's it. Now you're ready to go for video and image capture 

2. **Folder Structure**
    1. Scripts
         - DisplayDepth.cs: Rendering depth texture with the ConvertDepth material 
         - FBCaptureSDK:* Main script calling FBCAPTURE SDK’s APIs 
     2.  Shaders
         - ConvertDepth: Reading from Unity Depth texture and converting to RGB-D inverse depth
         - CubemapDisplay: Generating cubemap texture
         - CubemapToEquirect:* Generating equirect texture from cubemap texture
      3. Prefab
          - FBCapture: Capturing video and screenshot for 360, non-360 and RGB-D.
 It’s everything you need to put in the scene for capturing video and screenshot. 

3. **FBCapture Prefab and Interface on Unity Inspector**
FBCapture prefab handles all encoding and screenshot sessions with “FBCapture.dll”. 

      * *360 Capture Camera*
         * The cameras will be used for generating cubemap, equirect and depth textures for 360 capture
      * *Capture Options*
      * *Capture Mode:* You're able to select capture mode for 360 capture and non 360 capture
      * *Capture Texture Format*: You're able to select texture format for RGB and RGB-D captures
      * *Projection Type*: You're able to select projection type for 360 (EQUIRECT, CUBEMAP) capture
      * *Video Capture Type*: You're able to select video capture type for VOD and Live streaming
      * *Capture Hotkeys*
         * Hotkeys to start or stop video encoding and screenshot
      * *Live Video Settings*
        * Live Video Preset
           * *CUSTOM*: Custom resolution and bit rate
           * *720P*: 1280 x 720, 2 megabit per second
           * *1080P*: 1920 x 1080, 4 megabit per second
           * *4K*: 4096 x 2048, 10 megabit per second
        * *Live Video Width*: Set video resolution for width
        * *Live Video Height*: Set video resolution for height
        * *Live Video Frame Rate*: Set live video FPS
        * *Live Video Bit Rate*: Set live video bit rate
        * *Live Stream Url: Set live stream key for streaming server*
     * *VOD Video Settings*
        * Vod Video Preset
           * *CUSTOM*: Custom resolution and bit rate
           * *720P*: 1280 x 720, 2 megabit per second
           * *1080P*: 1920 x 1080, 4 megabit per second
          * *4K*: 4096 x 2048, 10 megabit per second
       * VOD *Video Width*: Set video resolution for width
       * VOD *Video Height*: Set video resolution for height
       * VOD *Video Frame Rate*: Set vod video FPS
       * VOD *Video Bit Rate*: Set vod video bit rate
       * *Full Vod Save Path*: Set video save path for recorded video including file name (file format should be mp4 or h264)
   * *Screenshot Settings*
      * Screenshot Preset
          * *CUSTOM*: Custom resolution
          * *720P*: 1280 x 720
          * *1080P*: 1920 x 1080
          * *4K*: 4096 x 2048
     * *Screenshot Width*: Set screenshot resolution for width
     * *Screenshot Height*: Set screenshot resolution for height
   * Preview Video Settings
       * Preview Video Preset
           * *CUSTOM*: Custom resolution and bit rate
           * *720P*: 1280 x 720, 2 megabit per second
           * *1080P*: 1920 x 1080, 4 megabit per second
           * *4K*: 4096 x 2048, 10 megabit per second
       * *Preview Video Width*: Set preview video resolution for width 
       * *Preview Video Height*: Set preview video resolution for height
       * *Preview Video Frame Rate*: Set preview video FPS
       * *Preview Video Bit Rate*: Set preview video bit rate
       * TODO: The function is implemented in FBCapture SDK, but the Unity sample doesn’t include this feature. We will update this feature in Unity sample later. 
   * Shader Settings
       * The shaders will generate cubemap, equirect textures with Surround Capture Cameras. 

**4. FBCapture SDK LOG SAVE PATH**

LOG file will be saved in "%LOCALAPPDATA%\FBCapture" and the log file will be named with process name and timestamp to have unique log file per process. (ex. %LOCALAPPDATA%\FBCapture\FBCaptureSDK_processname_timestamp.txt)
If you want to change save path or naming convention, please refer to Log.cpp file. 

**5. FBCapture SDK Calling Conventions (Sample Unity C# Integration Code)**

   **1. Set up Session** 
APIs needed to be called once at start encoding

  (1) Check capabilities

private static extern FBCAPTURE_STATUS fbc_getCaptureCapability();

It will check hardware and software capabilities. Currently it checks windows version(Win 7 SP1 or greater), Graphics driver version(Nvidia: 375.95 or newer, AMD: Radeon Software Crimson 17.1.1 or later) and Graphics manufacturer(AMD, NVIDIA). If your specs doesn't meet SDK's minimum specs, it will return failure reason and won't start encoding. 

 (2) Set configurations depending on capture types

// Live configurations
private static extern FBCAPTURE_STATUS fbc_setLiveCaptureSettings(
                                                      int width,
                                                      int height,
                                                      int frameRate,
                                                      int bitRate,
                                                      float flushCycleStart,
                                                      float flushCycleAfter,
                                                      string streamUrl,
                                                      bool is360,
                                                      bool verticalFlip,
                                                      bool horizontalFlip,
                                                      PROJECTIONTYPE projectionType,
                                                      STEREO_MODE stereoMode);

// VOD configurations                                                                                                            
private static extern FBCAPTURE_STATUS fbc_setVodCaptureSettings(
                                                      int width,
                                                      int height,
                                                      int frameRate,
                                                      int bitRate,
                                                      string fullSavePath,
                                                      bool is360,
                                                      bool verticalFlip,
                                                      bool horizontalFlip,
                                                      PROJECTIONTYPE projectionType,
                                                      STEREO_MODE stereoMode);

// Screenshot Configurations
private static extern FBCAPTURE_STATUS fbc_setScreenshotSettings(
                                                      int width,
                                                      int height,
                                                      string fullSavePath,
                                                      bool is360,
                                                      bool verticalFlip,
                                                      bool horizontalFlip);

// Preview configurations
private static extern FBCAPTURE_STATUS fbc_setPreviewCaptureSettings(
                                                      int width,
                                                      int height,
                                                      int frameRate,
                                                      bool is360,
                                                      bool verticalFlip,
                                                      bool horizontalFlip);

                                                      

It will configure encoding or screenshot properties.  In the Unity sample, we can easily set them on inspector of Unity editor. 

  (3) Set VR audio device in used

private static extern FBCAPTURE_STATUS 
fbc_setMicAndAudioRenderDeviceByVRDeviceType(VRDeviceType vrDevice);

It will set VR audio device that FBCapture SDK will capture for output(headphone) and input(microphone).

   (4) Enable audio capture

private static extern FBCAPTURE_STATUS 
fbc_setAudioEnabledDuringCapture(bool enabled);

It will enable audio capture during video capture. If you don't want to capture audio(input/output), then you need to pass 
'false'. 

   (5) Start capture depending on capture types

// Start Live
private static extern FBCAPTURE_STATUS fbc_startLiveCapture();

// Start VOD
private static extern FBCAPTURE_STATUS fbc_startVodCapture();

// Start Screenshot
private static extern FBCAPTURE_STATUS fbc_startScreenshot();

// Start Preview 
private static extern FBCAPTURE_STATUS fbc_startPreviewCapture();

It will start encoding or screenshot session with creating resources and separate threads needed for session in dll. 

   **2. Pass rendered Textures for encoding**
   - API needed to be called every frame - Video
private static extern FBCAPTURE_STATUS fbc_captureTexture(IntPtr texturePtr);

It will pass rendered textures to FBCapture SDK and video will be generated based on the textures. 

   - API needed to be called once after creating rendered texture - Screenshot 
private static extern FBCAPTURE_STATUS fbc_saveScreenShot(IntPtr texturePtr);

It will pass rendered texture to FBCapture SDK and screenshot will be generated based on the texture.

**3. Stop Encoding**
API needed to be called once at stop

private static extern void fbc_stopCapture();

It will flush input textures stacked on buffer and create video file with audio and video muxing. And then metadata for projection type, stereo mode and stitching software name will be injected for 360 VOD video, but only stitching software name's metadata will be injected for non 360 VOD video. At last, it will release all created resources and cleanup threads. 
