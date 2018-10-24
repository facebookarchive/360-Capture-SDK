/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Runtime.InteropServices;
using UnityEngine.Rendering;
using UnityEngine.VR;

namespace FBCapture {
  public class FBCaptureSDK : MonoBehaviour {

    #region CAPTURE SDK

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_reset();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
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
                                                  PROJECTION_TYPE projectionType,
                                                  STEREO_MODE stereoMode);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setVodCaptureSettings(
                                                  int width,
                                                  int height,
                                                  int frameRate,
                                                  int bitRate,
                                                  string fullSavePath,
                                                  bool is360,
                                                  bool verticalFlip,
                                                  bool horizontalFlip,
                                                  PROJECTION_TYPE projectionType,
                                                  STEREO_MODE stereoMode);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setPreviewCaptureSettings(
                                                  int width,
                                                  int height,
                                                  int frameRate,
                                                  bool is360,
                                                  bool verticalFlip,
                                                  bool horizontalFlip);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setScreenshotSettings(
                                                  int width,
                                                  int height,
                                                  string fullSavePath,
                                                  bool is360,
                                                  bool verticalFlip,
                                                  bool horizontalFlip);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setCameraOverlaySettings(
                                                  float widthPercentage,
                                                  UInt32 viewPortTopLeftX,
                                                  UInt32 viewPortTopLeftY);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_enumerateMicDevices();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern UInt32 fbc_getMicDevicesCount();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    [return: MarshalAs(UnmanagedType.LPStr)]
    private static extern string fbc_getMicDeviceName(UInt32 index);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setMicDevice(UInt32 index);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_unsetMicDevice();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setMicEnabledDuringCapture(bool enabled);
    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setAudioEnabledDuringCapture(bool enabled);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_enumerateCameraDevices();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern UInt32 fbc_getCameraDevicesCount();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    [return: MarshalAs(UnmanagedType.LPStr)]
    private static extern string fbc_getCameraDeviceName(UInt32 index);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setCameraDevice(UInt32 index);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_unsetCameraDevice();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setCameraEnabledDuringCapture(bool enabled);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_setMicAndAudioRenderDeviceByVRDeviceType(VRDEVICE_TYPE vrDevice);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_startLiveCapture();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_startVodCapture();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_startPreviewCapture();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_startScreenshot();
    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_captureTexture(IntPtr texturePtr);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_previewCapture(IntPtr texturePtr);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_previewCamera(IntPtr texturePtr);

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_getCaptureStatus();
    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_getScreenshotStatus();

    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern void fbc_stopCapture();
    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_getCaptureCapability();
    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern FBCAPTURE_STATUS fbc_saveScreenShot(IntPtr texturePtr);
    [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    private static extern GRAPHICS_CARD fbc_checkGPUManufacturer();


    private const int ERROR_VIDEO_ENCODING_CAUSE_ERRORS = 100;
    private const int ERROR_AUDIO_ENCODING_CAUSE_ERRORS = 200;
    private const int ERROR_TRANSCODING_MUXING_CAUSE_ERRORS = 300;
    private const int ERROR_RTMP_CAUSE_ERRORS = 400;
    private const int ERROR_GRAPHICS_CAPTURE_ERRORS = 500;
    private const int ERROR_CONFIGURATION_ERRORS = 600;
    private const int ERROR_SYSTEM_ERRORS = 700;
    private const int ERROR_ENCODING_CAPABILITY = 800;

    public enum FBCAPTURE_STATUS {
      // Common error codes
      OK = 0,
      FBCAPTURE_STATUS_ENCODE_IS_NOT_READY,
      FBCAPTURE_STATUS_NO_INPUT_FILE,
      FBCAPTURE_STATUS_FILE_READING_ERROR,
      FBCAPTURE_STATUS_OUTPUT_FILE_OPEN_FAILED,
      FBCAPTURE_STATUS_OUTPUT_FILE_CREATION_FAILED,
      FBCAPTURE_STATUS_DXGI_CREATING_FAILED,
      FBCAPTURE_STATUS_DEVICE_CREATING_FAILED,

      // Video/Image encoding specific error codes      
      FBCAPTURE_STATUS_ENCODE_INIT_FAILED = ERROR_VIDEO_ENCODING_CAUSE_ERRORS,
      FBCAPTURE_STATUS_ENCODE_SET_CONFIG_FAILED,
      FBCAPTURE_STATUS_ENCODER_CREATION_FAILED,
      FBCAPTURE_STATUS_INVALID_TEXTURE_POINTER,
      FBCAPTURE_STATUS_CONTEXT_CREATION_FAILED,
      FBCAPTURE_STATUS_TEXTURE_CREATION_FAILED,
      FBCAPTURE_STATUS_TEXTURE_RESOURCES_COPY_FAILED,
      FBCAPTURE_STATUS_IO_BUFFER_ALLOCATION_FAILED,
      FBCAPTURE_STATUS_ENCODE_PICTURE_FAILED,
      FBCAPTURE_STATUS_ENCODE_FLUSH_FAILED,
      FBCAPTURE_STATUS_MULTIPLE_ENCODING_SESSION,
      FBCAPTURE_STATUS_INVALID_TEXTURE_RESOLUTION,

      // WIC specific error codes
      FBCAPTURE_STATUS_WIC_SAVE_IMAGE_FAILED,

      // Audio encoding specific error codes
      FBCAPTURE_STATUS_AUDIO_DEVICE_ENUMERATION_FAILED = ERROR_AUDIO_ENCODING_CAUSE_ERRORS,
      FBCAPTURE_STATUS_AUDIO_CLIENT_INIT_FAILED,
      FBCAPTURE_STATUS_WRITING_WAV_HEADER_FAILED,
      FBCAPTURE_STATUS_RELEASING_WAV_FAILED,

      // Transcoding and muxing specific error codes
      FBCAPTURE_STATUS_MF_CREATION_FAILED = ERROR_TRANSCODING_MUXING_CAUSE_ERRORS,
      FBCAPTURE_STATUS_MF_INIT_FAILED,
      FBCAPTURE_STATUS_MF_CREATING_WAV_FORMAT_FAILED,
      FBCAPTURE_STATUS_MF_TOPOLOGY_CREATION_FAILED,
      FBCAPTURE_STATUS_MF_TOPOLOGY_SET_FAILED,
      FBCAPTURE_STATUS_MF_TRANSFORM_NODE_SET_FAILED,
      FBCAPTURE_STATUS_MF_MEDIA_CREATION_FAILED,
      FBCAPTURE_STATUS_MF_HANDLING_MEDIA_SESSION_FAILED,

      // WAMEDIA muxing specific error codes
      FBCAPTURE_STATUS_WAMDEIA_MUXING_FAILED,

      // More MF error codes
      FBCAPTURE_STATUS_MF_STARTUP_FAILED,
      FBCAPTURE_STATUS_MF_TRANSFORM_CREATION_FAILED,
      FBCAPTURE_STATUS_MF_SOURCE_READER_CREATION_FAILED,
      FBCAPTURE_STATUS_MF_STREAM_SELECTION_FAILED,
      FBCAPTURE_STATUS_MF_MEDIA_TYPE_CREATION_FAILED,
      FBCAPTURE_STATUS_MF_MEDIA_TYPE_CONFIGURATION_FAILED,
      FBCAPTURE_STATUS_MF_MEDIA_TYPE_SET_FAILED,
      FBCAPTURE_STATUS_MF_MEDIA_TYPE_GET_FAILED,
      FBCAPTURE_STATUS_MF_CREATE_WAV_FORMAT_FROM_MEDIA_TYPE_FAILED,
      FBCAPTURE_STATUS_MF_TRANSFORM_OUTPUT_STREAM_INFO_FAILED,
      FBCAPTURE_STATUS_MF_CREATE_MEMORY_BUFFER_FAILED,
      FBCAPTURE_STATUS_MF_CREATE_SAMPLE_FAILED,
      FBCAPTURE_STATUS_MF_SAMPLE_ADD_BUFFER_FAILED,
      FBCAPTURE_STATUS_MF_READ_SAMPLE_FAILED,
      FBCAPTURE_STATUS_MF_TRANSFORM_FAILED,
      FBCAPTURE_STATUS_MF_BUFFER_LOCK_FAILED,

      // RTMP specific error codes
      FBCAPTURE_STATUS_INVALID_FLV_HEADER = ERROR_RTMP_CAUSE_ERRORS,
      FBCAPTURE_STATUS_INVALID_STREAM_URL,
      FBCAPTURE_STATUS_RTMP_CONNECTION_FAILED,
      FBCAPTURE_STATUS_RTMP_DISCONNECTED,
      FBCAPTURE_STATUS_SENDING_RTMP_PACKET_FAILED,

      // Graphics capture error codes
      FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_INIT_FAILED = ERROR_GRAPHICS_CAPTURE_ERRORS,
      FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_INVALID_TEXTURE,
      FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_OPEN_SHARED_RESOURCE_FAILED,
      FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_MUTEX_ACQUIRE_FAILED,
      FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_ACQUIRE_ACQUIRE_SYNC_FAILED,
      FBCAPTURE_STATUS_GRAPHICS_DEVICE_CAPTURE_KEYED_ACQUIRE_RELASE_SYNC_FAILED,

      // Configuration error codes
      FBCAPTURE_STATUS_MIC_NOT_CONFIGURED = ERROR_CONFIGURATION_ERRORS,
      FBCAPTURE_STATUS_MIC_REQUIRES_ENUMERATION,
      FBCAPTURE_STATUS_MIC_DEVICE_NOT_SET,
      FBCAPTURE_STATUS_MIC_ENUMERATION_FAILED,
      FBCAPTURE_STATUS_MIC_SET_FAILED,
      FBCAPTURE_STATUS_MIC_UNSET_FAILED,
      FBCAPTURE_STATUS_MIC_INDEX_INVALID,
      FBCAPTURE_STATUS_CAMERA_NOT_CONFIGURED,
      FBCAPTURE_STATUS_CAMERA_REQUIRES_ENUMERATION,
      FBCAPTURE_STATUS_CAMERA_DEVICE_NOT_SET,
      FBCAPTURE_STATUS_CAMERA_ENUMERATION_FAILED,
      FBCAPTURE_STATUS_CAMERA_SET_FAILED,
      FBCAPTURE_STATUS_CAMERA_UNSET_FAILED,
      FBCAPTURE_STATUS_CAMERA_INDEX_INVALID,
      FBCAPTURE_STATUS_LIVE_CAPTURE_SETTINGS_NOT_CONFIGURED,
      FBCAPTURE_STATUS_VOD_CAPTURE_SETTINGS_NOT_CONFIGURED,
      FBCAPTURE_STATUS_PREVIEW_CAPTURE_SETTINGS_NOT_CONFIGURED,

      // System error codes
      FBCAPTURE_STATUS_SYSTEM_INITIALIZE_FAILED = ERROR_SYSTEM_ERRORS,
      FBCAPTURE_STATUS_SYSTEM_ENCODING_TEXTURE_CREATION_FAILED,
      FBCAPTURE_STATUS_SYSTEM_PREVIEW_TEXTURE_CREATION_FAILED,
      FBCAPTURE_STATUS_SYSTEM_ENCODING_TEXTURE_FORMAT_CREATION_FAILED,
      FBCAPTURE_STATUS_SYSTEM_SCREENSHOT_TEXTURE_FORMAT_CREATION_FAILED,
      FBCAPTURE_STATUS_SYSTEM_CAPTURE_IN_PROGRESS,
      FBCAPTURE_STATUS_SYSTEM_CAPTURE_NOT_IN_PROGRESS,
      FBCAPTURE_STATUS_SYSTEM_CAPTURE_TEXTURE_NOT_RECEIVED,
      FBCAPTURE_STATUS_SYSTEM_CAMERA_OVERLAY_FAILED,
      FBCAPTURE_STATUS_SYSTEM_CAPTURE_PREVIEW_FAILED,
      FBCAPTURE_STATUS_SYSTEM_CAPTURE_PREVIEW_NOT_IN_PROGRESS,

      // Encoding capability error codes
      FBCAPTURE_STATUS_UNSUPPORTED_ENCODING_ENVIRONMENT = ERROR_ENCODING_CAPABILITY,
      FBCAPTURE_STATUS_UNSUPPORTED_GRAPHICS_CARD_DRIVER_VERSION,
      FBCAPTURE_STATUS_UNSUPPORTED_GRAPHICS_CARD,
      FBCAPTURE_STATUS_UNSUPPORTED_OS_VERSION,
      FBCAPTURE_STATUS_UNSUPPORTED_OS_PROCESSOR,
    }

    public enum VRDEVICE_TYPE {
      UNKNOWN,
      OCULUS_RIFT,
      HTC_VIVE,
    }

    public enum STEREO_MODE {
      SM_NONE,
      SM_TOP_BOTTOM,
      SM_LEFT_RIGHT
    }

    public enum PROJECTION_TYPE {
      NONE,
      EQUIRECT,
      CUBEMAP
    }

    public enum GRAPHICS_CARD {
      NVIDIA,
      AMD,
      UNSUPPORTED_DEVICE
    }

    public enum CAPTURE_MODE {
      _360_CAPTURE,
      NON_360_CAPTURE,
    }
    public enum CAPTURE_TEXTURE_FORMAT {
      RGB_CAPTURE,
      RGBD_CAPTURE,
    }

    public enum CAPTURE_TYPE {
      NONE,
      LIVE,
      VOD,
      PREVIEW,
      SCREENSHOT
    }

    public enum VIDEO_CAPTURE_TYPE {
      VOD,
      LIVE,
    }

    public enum RESOLUTION_PRESET {
      CUSTOM,
      _720P,
      _1080P,
      _4K,
    }

    public enum CAPTURE_STATUS {
      STARTING,
      RUNNING,
      STOPPED
    }

    public enum CAPTURE_ERROR {
      UNSUPPORTED_SPEC,
      VOD_FAILED_TO_START,
      LIVE_FAILED_TO_START,
      PREVIEW_FAILED_TO_START,
      SCREENSHOT_FAILED_TO_START,
      INVALID_STREAM_URI,
      TEXTURE_ENCODE_FAILED,
      SCREENSHOT_FAILED,
      CAPTURE_ALREADY_IN_PROGRESS,
    }
    #endregion

    [Header("360 Capture Camera")]
    [Tooltip("Reference to camera that renders the cubemap")]
    public Camera cubemapCamera;
    [Tooltip("Reference to camera that renders the depth cubemap")]
    public Camera depthCubemapCamera;
    [Header("Regular Capture Camera")]
    private Camera regularCamera;

    public static FBCaptureSDK singleton;

    [Header("Capture Options")]
    public CAPTURE_MODE captureMode = CAPTURE_MODE._360_CAPTURE;
    public CAPTURE_TEXTURE_FORMAT captureTextureFormat = CAPTURE_TEXTURE_FORMAT.RGB_CAPTURE;
    public PROJECTION_TYPE projectionType = PROJECTION_TYPE.EQUIRECT;
    public VIDEO_CAPTURE_TYPE videoCaptureType = VIDEO_CAPTURE_TYPE.VOD;

    [Header("Capture Hotkeys")]
    public KeyCode startScreenShot = KeyCode.F1;
    public KeyCode startEncoding = KeyCode.F2;
    public KeyCode stopEncoding = KeyCode.F3;

    /// <summary>
    /// Encoding Setting Variables
    /// </summary>
    //    Live Video
    [Header("Live Video Settings")]
    public RESOLUTION_PRESET liveVideoPreset = RESOLUTION_PRESET.CUSTOM;
    public Int32 liveVideoWidth = 1920;
    public Int32 liveVideoHeight = 1080;
    public Int32 liveVideoFrameRate = 30;
    public Int32 liveVideoBitRate = 5000000;
    public string liveStreamUrl = "";  // You can get test live stream key on "https://www.facebook.com/live/create". 
                                       // ex. rtmp://rtmp-api-dev.facebook.com:80/rtmp/xxStreamKeyxx
    private const float encodingInitialFlushCycle = 5f;  // Video initial flush cycle
    private const float encodingSecondaryFlushCycle = 5f;  // Video flush cycle
                                                           //    Vod
    [Header("VOD Video Settings")]
    public RESOLUTION_PRESET vodVideoPreset = RESOLUTION_PRESET.CUSTOM;
    public Int32 vodVideoWidth = 4096;
    public Int32 vodVideoHeight = 2048;
    public Int32 vodVideoFrameRate = 30;
    public Int32 vodVideoBitRate = 5000000;
    [Tooltip("Save path for recorded video including file name. File format should be mp4 or h264")]
    public string fullVodSavePath = "";  // Save path for recorded video including file name (c://xxx.mp4)

    //    Screenshot
    [Header("Screenshot Settings")]
    public RESOLUTION_PRESET screenshotPreset = RESOLUTION_PRESET.CUSTOM;
    public Int32 screenShotWidth = 4096;
    public Int32 screenShotHeight = 2048;
    [Tooltip("Save path for screenshot including file name. File format should be jpg")]
    public string fullScreenshotSavePath = "";  // Save path for screenshot including file name (c://xxx.jpg) 

    //    Preview Video
    [Header("Preview Video Settings")]
    public RESOLUTION_PRESET previewVideoPreset = RESOLUTION_PRESET.CUSTOM;
    public Int32 previewVideoWidth = 4096;
    public Int32 previewVideoHeight = 2048;
    public Int32 previewVideoFrameRate = 30;
    public Int32 previewVideoBitRate = 5000000;

    private Int32 outputTextureWidth = 0;
    private Int32 outputTextureHeight = 0;

    //    Callback for error handling
    public delegate void OnStatusCallback(CAPTURE_ERROR error, FBCAPTURE_STATUS? captureStatus);
    public event OnStatusCallback OnError = delegate { };

    // Private Members
    private bool captureStarted = false;
    private CAPTURE_TYPE captureStartedType = CAPTURE_TYPE.NONE;

    //Set ture if you want to mute Audio
    public bool enabledAudioCapture = true;
    public bool enabledMicCapture = true;

    private RenderTexture cubemapTexture = null;
    private RenderTexture cubemapRenderTarget = null;
    private RenderTexture depthCubemapTexture = null;
    private RenderTexture equirectTexture = null;
    private RenderTexture depthEquirectTexture = null;
    private RenderTexture outputTexture = null;

    private Material equirectMaterial;
    private Material cubemapMaterial;
    private Int32 cubemapSize = 1024;

    [Header("Shader Settings")]
    public Shader equirectShader;
    public Shader cubemapShader;

    private VRDEVICE_TYPE attachedHMD;
    private bool updateCubemap = true;
    private bool includeCameraRotation = true;
    private bool screenshotStarted = false;

    [Tooltip("Offset spherical coordinates (shift equirect)")]
    private Vector2 sphereOffset = new Vector2(0, 1);
    [Tooltip("Scale spherical coordinates (flip equirect, usually just 1 or -1)")]
    private Vector2 sphereScale = new Vector2(1, -1);

    void Awake() {

      if (singleton != null) {
        Debug.LogError("There are multiple instances of FBCaptureSDK.");
        return;
      }
      singleton = this;

      captureStarted = false;
      screenshotStarted = false;
      captureStartedType = CAPTURE_TYPE.NONE;
      regularCamera = GetComponent<Camera>();
      regularCamera.enabled = false;
      depthCubemapCamera.enabled = false;
      cubemapCamera.enabled = false;

      OnError += CaptureStatusLog;

      // Live video preset
      if (liveVideoPreset == RESOLUTION_PRESET._720P) {
        liveVideoWidth = 1280;
        liveVideoHeight = 720;
        liveVideoBitRate = 2000000;
      } else if (liveVideoPreset == RESOLUTION_PRESET._1080P) {
        liveVideoWidth = 1920;
        liveVideoHeight = 1080;
        liveVideoBitRate = 4000000;
      } else if (liveVideoPreset == RESOLUTION_PRESET._4K) {
        liveVideoWidth = 4096;
        liveVideoHeight = 2048;
        liveVideoBitRate = 10000000;
      }

      // VOD video preset
      if (vodVideoPreset == RESOLUTION_PRESET._720P) {
        vodVideoWidth = 1280;
        vodVideoHeight = 720;
        vodVideoBitRate = 2000000;
      } else if (vodVideoPreset == RESOLUTION_PRESET._1080P) {
        vodVideoWidth = 1920;
        vodVideoHeight = 1080;
        vodVideoBitRate = 4000000;
      } else if (vodVideoPreset == RESOLUTION_PRESET._4K) {
        vodVideoWidth = 4096;
        vodVideoHeight = 2048;
        vodVideoBitRate = 10000000;
      }

      // Screenshot video preset
      if (screenshotPreset == RESOLUTION_PRESET._720P) {
        screenShotWidth = 1280;
        screenShotHeight = 720;
      } else if (screenshotPreset == RESOLUTION_PRESET._1080P) {
        screenShotWidth = 1920;
        screenShotHeight = 1080;
      } else if (screenshotPreset == RESOLUTION_PRESET._4K) {
        screenShotWidth = 4096;
        screenShotHeight = 2048;
      }

      // Preview video preset
      if (previewVideoPreset == RESOLUTION_PRESET._720P) {
        previewVideoWidth = 1280;
        previewVideoHeight = 720;
        previewVideoBitRate = 2000000;
      } else if (previewVideoPreset == RESOLUTION_PRESET._1080P) {
        previewVideoWidth = 1920;
        previewVideoHeight = 1080;
        previewVideoBitRate = 4000000;
      } else if (previewVideoPreset == RESOLUTION_PRESET._4K) {
        previewVideoWidth = 4096;
        previewVideoHeight = 2048;
        previewVideoBitRate = 10000000;
      }

      // Retrieve attached VR devie for sound and microphone capture in VR
      // If expected VR device is not attached, it will capture default audio device
      string vrDeviceName = UnityEngine.XR.XRDevice.model.ToLower();
      if (vrDeviceName.Contains("rift")) {
        attachedHMD = VRDEVICE_TYPE.OCULUS_RIFT;
      } else if (vrDeviceName.Contains("vive")) {
        attachedHMD = VRDEVICE_TYPE.HTC_VIVE;
      } else {
        attachedHMD = VRDEVICE_TYPE.UNKNOWN;
      }

      // Check hardware capability for screenshot
      FBCAPTURE_STATUS status;
      status = fbc_getCaptureCapability();
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.SCREENSHOT_FAILED_TO_START, status);
      }
    }

    void Update() {

      // Input interface for start/stop encoding and screenshot
      if (Input.GetKeyDown(startEncoding)) {
        if (videoCaptureType == VIDEO_CAPTURE_TYPE.LIVE) {
          StartLiveStreaming(liveStreamUrl);
        } else if (videoCaptureType == VIDEO_CAPTURE_TYPE.VOD) {
          StartVodCapture();
        }
      } else if (Input.GetKeyDown(stopEncoding)) {
        StopCapture();
      } else if (Input.GetKeyDown(startScreenShot)) {
        SaveScreenShot(screenShotWidth, screenShotHeight);
      }

      if (!captureStarted && !screenshotStarted) return;

      if (updateCubemap && captureMode == CAPTURE_MODE._360_CAPTURE) {
        // Render rgb cubemap
        if (cubemapCamera && cubemapTexture) {
          cubemapCamera.transform.position = transform.position;
          cubemapCamera.RenderToCubemap(cubemapTexture);
        }

        if (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE && depthCubemapCamera && depthCubemapTexture) {
          // Render depth cubemap
          depthCubemapCamera.transform.position = transform.position;
          depthCubemapCamera.RenderToCubemap(depthCubemapTexture);
        }

        StartCoroutine(SurroundScreenBufferBlit());
      } else {
        regularCamera.Render();
      }
    }

    /// <summary>
    /// Configuration for Live Streaming 
    /// </summary>
    /// <param name="streamUrl">live stream key</param>
    public bool StartLiveStreaming(string streamUrl) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS.OK;

      regularCamera.enabled = true;
      depthCubemapCamera.enabled = true;
      cubemapCamera.enabled = true;

      if (captureStarted || fbc_getCaptureStatus() != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.CAPTURE_ALREADY_IN_PROGRESS, null);
        return false;
      }

      if (string.IsNullOrEmpty(streamUrl)) {
        OnError(CAPTURE_ERROR.INVALID_STREAM_URI, null);
        return false;
      }

      if ((projectionType != PROJECTION_TYPE.EQUIRECT && captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE) ||
          (captureMode == CAPTURE_MODE.NON_360_CAPTURE && captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE)) {
        Debug.Log("We only support RGB-D capture with equirect projection type and suruound capture mode");
        captureTextureFormat = CAPTURE_TEXTURE_FORMAT.RGB_CAPTURE;
      }

      if (captureMode == CAPTURE_MODE._360_CAPTURE && projectionType == PROJECTION_TYPE.NONE) {
        Debug.Log("ProjectionType should be set for 360 capture. " +
                  "We want to set type to equirect for generating texture properly");
        projectionType = PROJECTION_TYPE.EQUIRECT;
      } else if (captureMode == CAPTURE_MODE.NON_360_CAPTURE) {  // Non 360 capture doesn't have projection type
        projectionType = PROJECTION_TYPE.NONE;
      }

      // Live video preset
      if (liveVideoPreset == RESOLUTION_PRESET._720P) {
        liveVideoWidth = 1280;
        liveVideoHeight = 720;
        liveVideoBitRate = 2000000;
      } else if (liveVideoPreset == RESOLUTION_PRESET._1080P) {
        liveVideoWidth = 1920;
        liveVideoHeight = 1080;
        liveVideoBitRate = 4000000;
      } else if (liveVideoPreset == RESOLUTION_PRESET._4K) {
        liveVideoWidth = 4096;
        liveVideoHeight = 2048;
        liveVideoBitRate = 10000000;
      }

      // Check hardware capability for live video encoding
      status = fbc_getCaptureCapability();
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.UNSUPPORTED_SPEC, status);
        return false;
      }

      // MAX video encoding resolution
      // AMD:     4096 x 2048
      // NVIDIA:  4096 x 4096
      if (GRAPHICS_CARD.AMD == fbc_checkGPUManufacturer() &&
          (liveVideoWidth > 4096 ||
          (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE ? liveVideoHeight * 2 : liveVideoHeight) > 2048)) {
        Debug.Log("Max video encoding resolution on AMD is 4096 x 2048");
        OnError(CAPTURE_ERROR.UNSUPPORTED_SPEC, null);
        return false;
      } else if (GRAPHICS_CARD.NVIDIA == fbc_checkGPUManufacturer() &&
                 (liveVideoWidth > 4096 ||
                 (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE ? liveVideoHeight * 2 : liveVideoHeight) > 4096)) {
        Debug.Log("Max video encoding resolution on NVIDIA is 4096 x 4096");
        OnError(CAPTURE_ERROR.UNSUPPORTED_SPEC, null);
        return false;
      } else if (GRAPHICS_CARD.UNSUPPORTED_DEVICE == fbc_checkGPUManufacturer()) {
        Debug.Log("Unsupported gpu device or you missed to call fbc_getCaptureCapability supporting gpu device check");
        OnError(CAPTURE_ERROR.UNSUPPORTED_SPEC, null);
        return false;
      }

      // Create RenderTextures which will be used for live video encoding
      CreateRenderTextures(liveVideoWidth, liveVideoHeight);

      // Video Encoding and Live Configuration Settings
      status = fbc_setLiveCaptureSettings(
                      width: outputTextureWidth,
                      height: outputTextureHeight,
                      frameRate: liveVideoFrameRate,
                      bitRate: liveVideoBitRate,
                      flushCycleStart: encodingInitialFlushCycle,
                      flushCycleAfter: encodingSecondaryFlushCycle,
                      streamUrl: streamUrl,
                      is360: captureMode == CAPTURE_MODE._360_CAPTURE ? true : false,
                      verticalFlip: false,
                      horizontalFlip: false,
                      projectionType: projectionType,
                      stereoMode: STEREO_MODE.SM_NONE);
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.LIVE_FAILED_TO_START, status);
        return false;
      }

      // Pick attached audio device resources for audio capture
      status = fbc_setMicAndAudioRenderDeviceByVRDeviceType(attachedHMD);
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.LIVE_FAILED_TO_START, status);
        return false;
      }

      // Make enable audio output capture(ex. speaker)
      status = fbc_setAudioEnabledDuringCapture(enabledAudioCapture);
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.LIVE_FAILED_TO_START, status);
        return false;
      }

      // Make enable audio input capture(ex. microphone)
      status = fbc_setMicEnabledDuringCapture(enabledMicCapture);
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.LIVE_FAILED_TO_START, status);
        return false;
      }

      status = fbc_startLiveCapture();
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.LIVE_FAILED_TO_START, status);
        return false;
      }

      OnCaptureStarted(CAPTURE_TYPE.LIVE);
      return true;
    }

    /// <summary>
    /// Configuration for Video Recording
    /// </summary>
    public bool StartVodCapture() {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS.OK;

      regularCamera.enabled = false;
      depthCubemapCamera.enabled = false;
      cubemapCamera.enabled = false;

      if (captureStarted || fbc_getCaptureStatus() != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.CAPTURE_ALREADY_IN_PROGRESS, null);
        return false;
      }

      if ((projectionType != PROJECTION_TYPE.EQUIRECT && captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE) ||
          (captureMode == CAPTURE_MODE.NON_360_CAPTURE && captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE)) {
        Debug.Log("We only support RGB-D capture with equirect projection type and 360 capture mode");
        captureTextureFormat = CAPTURE_TEXTURE_FORMAT.RGB_CAPTURE;
      }

      if (captureMode == CAPTURE_MODE._360_CAPTURE && projectionType == PROJECTION_TYPE.NONE) {
        Debug.Log("ProjectionType should be set for 360 capture. " +
                  "We want to set type to equirect for generating texture properly");
        projectionType = PROJECTION_TYPE.EQUIRECT;
      } else if (captureMode == CAPTURE_MODE.NON_360_CAPTURE) {  // Non 360 capture doesn't have projection type
        projectionType = PROJECTION_TYPE.NONE;
      }

      // VOD video preset
      if (vodVideoPreset == RESOLUTION_PRESET._720P) {
        vodVideoWidth = 1280;
        vodVideoHeight = 720;
        vodVideoBitRate = 2000000;
      } else if (vodVideoPreset == RESOLUTION_PRESET._1080P) {
        vodVideoWidth = 1920;
        vodVideoHeight = 1080;
        vodVideoBitRate = 4000000;
      } else if (vodVideoPreset == RESOLUTION_PRESET._4K) {
        vodVideoWidth = 4096;
        vodVideoHeight = 2048;
        vodVideoBitRate = 10000000;
      }

      // Check hardware capability for video encoding
      status = fbc_getCaptureCapability();
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.VOD_FAILED_TO_START, status);
        return false;
      }

      // MAX video encoding resolution
      // AMD:     4096 x 2048
      // NVIDIA:  4096 x 4096
      if (GRAPHICS_CARD.AMD == fbc_checkGPUManufacturer() &&
          (vodVideoWidth > 4096 ||
          (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE ? vodVideoHeight * 2 : vodVideoHeight) > 2048)) {
        Debug.Log("Max video encoding resolution on AMD is 4096 x 2048");
        OnError(CAPTURE_ERROR.UNSUPPORTED_SPEC, null);
        return false;
      } else if (GRAPHICS_CARD.NVIDIA == fbc_checkGPUManufacturer() &&
                 (vodVideoWidth > 4096 ||
                 (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE ? vodVideoHeight * 2 : vodVideoHeight) > 4096)) {
        Debug.Log("Max video encoding resolution on NVIDIA is 4096 x 4096");
        OnError(CAPTURE_ERROR.UNSUPPORTED_SPEC, null);
        return false;
      } else if (GRAPHICS_CARD.UNSUPPORTED_DEVICE == fbc_checkGPUManufacturer()) {
        Debug.Log("Unsupported gpu device or you missed to call fbc_getCaptureCapability supporting gpu device check");
        OnError(CAPTURE_ERROR.UNSUPPORTED_SPEC, null);
        return false;
      }

      // Create RenderTextures which will be used for video encoding
      CreateRenderTextures(vodVideoWidth, vodVideoHeight);

      // If we haven't set the save path, we want to use project folder and timestamped file name by default
      string savePath;
      if (string.IsNullOrEmpty(fullVodSavePath)) {
        savePath = string.Format("{0}/movie_{1}x{2}_{3}.mp4",
            Directory.GetCurrentDirectory(),
            outputTextureWidth, outputTextureHeight,
            DateTime.Now.ToString("yyyy-MM-dd hh_mm_ss"));
      } else {
        savePath = fullVodSavePath;
      }

      // Video Encoding Configuration Settings
      status = fbc_setVodCaptureSettings(
                    width: outputTextureWidth,
                    height: outputTextureHeight,
                    frameRate: vodVideoFrameRate,
                    bitRate: vodVideoBitRate,
                    fullSavePath: savePath,
                    is360: captureMode == CAPTURE_MODE._360_CAPTURE ? true : false,
                    verticalFlip: false,
                    horizontalFlip: captureMode == CAPTURE_MODE._360_CAPTURE ? true : false,
                    projectionType: projectionType,
                    stereoMode: STEREO_MODE.SM_NONE);
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.VOD_FAILED_TO_START, status);
        return false;
      }

      // Pick attached audio device resources for audio capture
      status = fbc_setMicAndAudioRenderDeviceByVRDeviceType(attachedHMD);
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.VOD_FAILED_TO_START, status);
        return false;
      }

      // Make enable audio output capture(ex. speaker)
      status = fbc_setAudioEnabledDuringCapture(enabledAudioCapture);
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.VOD_FAILED_TO_START, status);
        return false;
      }

      // Make enable audio input capture(ex. microphone)
      status = fbc_setMicEnabledDuringCapture(enabledMicCapture);
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.VOD_FAILED_TO_START, status);
        return false;
      }

      // Start VOD capture
      status = fbc_startVodCapture();
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.VOD_FAILED_TO_START, status);
        return false;
      }

      OnCaptureStarted(CAPTURE_TYPE.VOD);
      return true;
    }

    /// <summary>
    /// Configuration for Screenshot
    /// </summary>
    /// <param name="width">  Screenshot resolution - width </param>
    /// <param name="height"> Screenshot resolution - height </param>
    public bool SaveScreenShot(int width, int height) {
      FBCAPTURE_STATUS status;

      regularCamera.enabled = false;
      depthCubemapCamera.enabled = false;
      cubemapCamera.enabled = false;

      // Check current screenshot status.
      // It should return FBCAPTURE_STATUS.OK when it's not in progress
      status = fbc_getScreenshotStatus();
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.CAPTURE_ALREADY_IN_PROGRESS, null);
        return false;
      }

      if ((projectionType != PROJECTION_TYPE.EQUIRECT && captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE) ||
          (captureMode == CAPTURE_MODE.NON_360_CAPTURE && captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE)) {
        Debug.Log("We only support RGB-D capture with equirect projection type and suruound capture mode");
        captureTextureFormat = CAPTURE_TEXTURE_FORMAT.RGB_CAPTURE;
      }

      if (captureMode == CAPTURE_MODE._360_CAPTURE && projectionType == PROJECTION_TYPE.NONE) {
        Debug.Log("ProjectionType should be set for 360 capture. " +
                  "We want to set type to equirect for generating texture properly");
        projectionType = PROJECTION_TYPE.EQUIRECT;
      }

      // Screenshot video preset
      if (screenshotPreset == RESOLUTION_PRESET._720P) {
        screenShotWidth = 1280;
        screenShotHeight = 720;
      } else if (screenshotPreset == RESOLUTION_PRESET._1080P) {
        screenShotWidth = 1920;
        screenShotHeight = 1080;
      } else if (screenshotPreset == RESOLUTION_PRESET._4K) {
        screenShotWidth = 3840;
        screenShotHeight = 2160;
      }

      // In the sample, we only allow to use same resolution with video encoding WHEN video encoding is started
      if (captureStarted && screenShotWidth != outputTextureWidth && screenShotHeight != outputTextureHeight) {
        screenShotWidth = outputTextureWidth;
        screenShotHeight = outputTextureHeight;
      }

      // Create RenderTextures which will be used for screenshot
      CreateRenderTextures(screenShotWidth, screenShotHeight);

      // If we haven't set the save path, we want to use project folder and timestamped file name by default
      string savePath;
      if (string.IsNullOrEmpty(fullScreenshotSavePath)) {
        savePath = string.Format("{0}/movie_{1}x{2}_{3}.jpg",
            Directory.GetCurrentDirectory(),
            outputTextureWidth, outputTextureHeight,
            DateTime.Now.ToString("yyyy-MM-dd hh_mm_ss"));
      } else {
        savePath = fullScreenshotSavePath;
      }

      // Screenshot Configuration Settings in FBCapture SDK 
      status = fbc_setScreenshotSettings(
                      width: outputTextureWidth,
                      height: outputTextureHeight,
                      fullSavePath: savePath,
                      is360: captureMode == CAPTURE_MODE._360_CAPTURE ? true : false,
                      verticalFlip: false,
                      horizontalFlip: captureMode == CAPTURE_MODE._360_CAPTURE ? true : false);

      // Start ScreenShot
      status = fbc_startScreenshot();
      if (status != FBCAPTURE_STATUS.OK) {
        OnError(CAPTURE_ERROR.SCREENSHOT_FAILED_TO_START, status);
        return false;
      }

      screenshotStarted = true;

      if (captureMode == CAPTURE_MODE.NON_360_CAPTURE) {
        regularCamera.Render();
      }

      Debug.Log("Screenshot started");

      return true;
    }

    /// <summary>
    /// Capture Stop Routine with Unity resource release
    /// </summary>
    public void StopCapture() {
      if (captureStarted) {

        fbc_stopCapture();

        // Release textures & material
        if (equirectTexture) {
          Destroy(equirectTexture);
          equirectTexture = null;
        }

        if (depthEquirectTexture) {
          Destroy(depthEquirectTexture);
          depthEquirectTexture = null;
        }

        if (cubemapTexture) {
          Destroy(cubemapTexture);
          cubemapTexture = null;
        }

        if (depthCubemapTexture) {
          Destroy(depthCubemapTexture);
          depthCubemapTexture = null;
        }

        if (equirectMaterial) {
          Destroy(equirectMaterial);
          equirectMaterial = null;
        }

        captureStarted = false;

        regularCamera.enabled = false;
        depthCubemapCamera.enabled = false;
        cubemapCamera.enabled = false;

        captureStartedType = CAPTURE_TYPE.NONE;

        Debug.Log("Capture stopped");
      }
    }

    IEnumerator SurroundScreenBufferBlit() {

      yield return null;

      if (captureStarted || screenshotStarted) {

        yield return new WaitForEndOfFrame();

        regularCamera.targetTexture = outputTexture;

        if (projectionType == PROJECTION_TYPE.EQUIRECT) {
          // convert to equirectangular
          equirectMaterial.SetTexture("_CubeTex", cubemapTexture);
          equirectMaterial.SetVector("_SphereScale", sphereScale);
          equirectMaterial.SetVector("_SphereOffset", sphereOffset);

          if (includeCameraRotation) {
            // cubemaps are always rendered along axes, so we do rotation by rotating the cubemap lookup
            equirectMaterial.SetMatrix("_CubeTransform", Matrix4x4.TRS(Vector3.zero, transform.rotation, Vector3.one));
          } else {
            equirectMaterial.SetMatrix("_CubeTransform", Matrix4x4.identity);
          }

          if (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE) {
            // equirect RGB texture copy
            Graphics.Blit(null, equirectTexture, equirectMaterial);
            Graphics.CopyTexture(equirectTexture, 0, 0, 0, 0, outputTextureWidth, outputTextureHeight / 2,
                                 outputTexture, 0, 0, 0, outputTextureHeight / 2);

            // equirect depth texture copy
            equirectMaterial.SetTexture("_CubeTex", depthCubemapTexture);
            Graphics.Blit(null, depthEquirectTexture, equirectMaterial);
            Graphics.CopyTexture(depthEquirectTexture, 0, 0, 0, 0, outputTextureWidth, outputTextureHeight / 2,
                                 outputTexture, 0, 0, 0, 0);
          } else {
            // equirect RGB texture copy
            Graphics.Blit(null, equirectTexture, equirectMaterial);
            Graphics.CopyTexture(equirectTexture, 0, 0, 0, 0, outputTextureWidth, outputTextureHeight,
                                 outputTexture, 0, 0, 0, 0);
          }
        } else if (projectionType == PROJECTION_TYPE.CUBEMAP) {
          cubemapMaterial.SetTexture("_CubeTex", cubemapTexture);
          cubemapMaterial.SetVector("_SphereScale", sphereScale);
          cubemapMaterial.SetVector("_SphereOffset", sphereOffset);

          if (includeCameraRotation) {
            // cubemaps are always rendered along axes, so we do rotation by rotating the cubemap lookup
            cubemapMaterial.SetMatrix("_CubeTransform", Matrix4x4.TRS(Vector3.zero, transform.rotation, Vector3.one));
          } else {
            cubemapMaterial.SetMatrix("_CubeTransform", Matrix4x4.identity);
          }

          cubemapMaterial.SetPass(0);

          Graphics.SetRenderTarget(cubemapRenderTarget);

          float s = 1.0f / 3.0f;
          RenderCubeFace(CubemapFace.PositiveX, 0.0f, 0.5f, s, 0.5f);
          RenderCubeFace(CubemapFace.NegativeX, s, 0.5f, s, 0.5f);
          RenderCubeFace(CubemapFace.PositiveY, s * 2.0f, 0.5f, s, 0.5f);

          RenderCubeFace(CubemapFace.NegativeY, 0.0f, 0.0f, s, 0.5f);
          RenderCubeFace(CubemapFace.PositiveZ, s, 0.0f, s, 0.5f);
          RenderCubeFace(CubemapFace.NegativeZ, s * 2.0f, 0.0f, s, 0.5f);

          Graphics.SetRenderTarget(null);
          Graphics.Blit(cubemapRenderTarget, outputTexture);
        }

        // Pass captured texture for video or screenshot
        updateTextureToNative();
      }
    }

    IEnumerator NonSurroundScreenBufferBlit() {
      yield return new WaitForEndOfFrame();
      
      // Pass captured texture for video or screenshot
      updateTextureToNative();
    }

    private void updateTextureToNative() {
      FBCAPTURE_STATUS status;
      if (outputTexture && captureStarted) {
        status = fbc_captureTexture(outputTexture.GetNativeTexturePtr()); // Passing render texture to FBCaptureSDK
        if (status != FBCAPTURE_STATUS.OK) {
          OnError(CAPTURE_ERROR.TEXTURE_ENCODE_FAILED, status);
          StopCapture();
          return;
        }
      }

      if (screenshotStarted && outputTexture) {
        screenshotStarted = false;
        status = fbc_saveScreenShot(outputTexture.GetNativeTexturePtr()); // Passing render texture to FBCaptureSDK
        if (status != FBCAPTURE_STATUS.OK) {
          OnError(CAPTURE_ERROR.SCREENSHOT_FAILED, status);
          return;
        }
      }
    }

    private void RenderCubeFace(CubemapFace face, float x, float y, float w, float h) {
      // Texture coordinates for displaying each cube map face
      Vector3[] faceTexCoords =
      {
                // +x
                new Vector3(1, 1, 1),
                new Vector3(1, -1, 1),
                new Vector3(1, -1, -1),
                new Vector3(1, 1, -1),
                // -x
                new Vector3(-1, 1, -1),
                new Vector3(-1, -1, -1),
                new Vector3(-1, -1, 1),
                new Vector3(-1, 1, 1),

                // -y
                new Vector3(-1, -1, 1),
                new Vector3(-1, -1, -1),
                new Vector3(1, -1, -1),
                new Vector3(1, -1, 1),
                // +y // flipped with -y for fb live
                new Vector3(-1, 1, -1),
                new Vector3(-1, 1, 1),
                new Vector3(1, 1, 1),
                new Vector3(1, 1, -1),

                // +z
                new Vector3(-1, 1, 1),
                new Vector3(-1, -1, 1),
                new Vector3(1, -1, 1),
                new Vector3(1, 1, 1),
                // -z
                new Vector3(1, 1, -1),
                new Vector3(1, -1, -1),
                new Vector3(-1, -1, -1),
                new Vector3(-1, 1, -1),
            };

      GL.PushMatrix();
      GL.LoadOrtho();
      GL.LoadIdentity();

      int i = (int)face;

      GL.Begin(GL.QUADS);
      GL.TexCoord(faceTexCoords[i * 4]);
      GL.Vertex3(x, y, 0);
      GL.TexCoord(faceTexCoords[i * 4 + 1]);
      GL.Vertex3(x, y + h, 0);
      GL.TexCoord(faceTexCoords[i * 4 + 2]);
      GL.Vertex3(x + w, y + h, 0);
      GL.TexCoord(faceTexCoords[i * 4 + 3]);
      GL.Vertex3(x + w, y, 0);
      GL.End();

      GL.PopMatrix();
    }

    private void OnCaptureStarted(CAPTURE_TYPE captureType) {
      captureStarted = true;
      captureStartedType = captureType;
      Debug.Log("Capture started");
    }

    private void CaptureStatusLog(CAPTURE_ERROR error, FBCAPTURE_STATUS? captureStatus) {
      Debug.Log("Capture SDK Error Occured of type: " + error + " [Error code: " + captureStatus + " ] \n" +
                "Please check FBCaptureSDK.txt log file for more information");
    }

    /// <summary>
    /// Create the RenderTexture for encoding texture
    /// </summary>
    /// <param name="width">  Encoding Resolution - width </param>
    /// <param name="height"> Encoding Resolution - height </param>
    private void CreateRenderTextures(int width, int height) {
      if (captureStarted) {
        Debug.Log("Capture is already started. You can't resize texture and generate new texture");
        return;
      }

      if (outputTextureWidth != width ||
          outputTextureHeight != (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE ? height * 2 : height)) {
        Debug.Log("Texture size was changed. Need to create new render texture");

        if (outputTexture) {
          Destroy(outputTexture);
          outputTexture = null;
        }

        if (equirectTexture) {
          Destroy(equirectTexture);
          equirectTexture = null;
        }

        if (depthEquirectTexture) {
          Destroy(depthEquirectTexture);
          depthEquirectTexture = null;
        }
      }

      outputTextureWidth = width;
      outputTextureHeight = captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE ? height * 2 : height;

      if (outputTexture == null) {
        outputTexture = new RenderTexture(outputTextureWidth,
                                          outputTextureHeight,
                                          24,
                                          RenderTextureFormat.ARGB32);
        regularCamera.targetTexture = outputTexture;
        outputTexture.Create();
        outputTexture.hideFlags = HideFlags.HideAndDontSave;
      }

      if (captureMode == CAPTURE_MODE._360_CAPTURE) {
        if (projectionType == PROJECTION_TYPE.EQUIRECT) {
          if (equirectTexture == null) {
            equirectTexture = new RenderTexture(width, height, 0, RenderTextureFormat.ARGB32);
            equirectTexture.Create();
            equirectTexture.hideFlags = HideFlags.HideAndDontSave;
          }

          if (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE) {
            if (depthEquirectTexture == null) {
              depthEquirectTexture = new RenderTexture(width, height, 0, RenderTextureFormat.ARGB32);
              depthEquirectTexture.Create();
              depthEquirectTexture.hideFlags = HideFlags.HideAndDontSave;
            }
          }

          // Create equirect material
          equirectMaterial = CreateMaterial(equirectShader, equirectMaterial);
        } else if (projectionType == PROJECTION_TYPE.CUBEMAP) {
          if (cubemapRenderTarget == null) {
            cubemapRenderTarget = new RenderTexture(width, height, 0, RenderTextureFormat.ARGB32);
            cubemapRenderTarget.Create();
            cubemapRenderTarget.hideFlags = HideFlags.HideAndDontSave;
            cubemapMaterial = CreateMaterial(cubemapShader, cubemapMaterial);
          }
        }

        // Create cubemap render texture
        if (cubemapTexture == null) {
          cubemapTexture = new RenderTexture(cubemapSize, cubemapSize, 0);
          cubemapTexture.hideFlags = HideFlags.HideAndDontSave;

        }
#if UNITY_5_4_OR_NEWER
        cubemapTexture.dimension = UnityEngine.Rendering.TextureDimension.Cube;
#else
        cubemapTexture.isCubemap = true;        
#endif

        if (captureTextureFormat == CAPTURE_TEXTURE_FORMAT.RGBD_CAPTURE) {
          if (depthCubemapTexture == null) {
            depthCubemapTexture = new RenderTexture(cubemapSize, cubemapSize, 0);
            depthCubemapTexture.hideFlags = HideFlags.HideAndDontSave;
          }
#if UNITY_5_4_OR_NEWER
          depthCubemapTexture.dimension = UnityEngine.Rendering.TextureDimension.Cube;
#else
          depthCubemapTexture.isCubemap = true;
#endif
        }
      }
    }

    /// <summary>
    /// Create materials which will be used for equirect and cubemap generation
    /// </summary>
    /// <param name="s"> shader code </param>
    /// <param name="m2Create"> material </param>
    /// <returns></returns>
    protected Material CreateMaterial(Shader s, Material m2Create) {
      if (!s) {
        Debug.Log("Missing shader in " + ToString());
        return null;
      }

      if (m2Create && (m2Create.shader == s) && (s.isSupported))
        return m2Create;

      if (!s.isSupported) {
        return null;
      }

      m2Create = new Material(s);
      m2Create.hideFlags = HideFlags.DontSave;

      return m2Create;
    }

    void OnRenderImage(RenderTexture src, RenderTexture dst) {
      if (captureMode == CAPTURE_MODE.NON_360_CAPTURE && outputTexture) {        
        Graphics.Blit(src, dst);
        Graphics.Blit(null, outputTexture);
        StartCoroutine(NonSurroundScreenBufferBlit());
      }
    }

    void OnDestroy() {
      StopCapture();
    }

    void OnApplicationQuit() {
      StopCapture();
    }

    public bool IsCaptureInProgress() {
      return captureStarted;
    }

    public UInt32 GetMicDevicesCount() {
      fbc_enumerateMicDevices();
      return fbc_getMicDevicesCount();
    }

    public string GetMicDeviceName(UInt32 index) {
      return fbc_getMicDeviceName(index);
    }

    public void SetMicDevice(UInt32 index) {
      fbc_setMicDevice(index);
    }

    public void UnsetMicDevice() {
      fbc_unsetMicDevice();
    }

    public UInt32 GetCameraDevicesCount() {
      fbc_enumerateCameraDevices();
      return fbc_getCameraDevicesCount();
    }

    public string GetCameraDeviceName(UInt32 index) {
      return fbc_getCameraDeviceName(index);
    }

    public void SetCameraDevice(UInt32 index) {
      fbc_setCameraDevice(index);
    }

    public void UnsetCameraDevice() {
      fbc_unsetCameraDevice();
    }

    public void SetCameraEnabledDuringCapture(bool enabled) {
      fbc_setCameraEnabledDuringCapture(enabled);
    }

    public void SetCameraOverlaySettings(float widthPercentage, UInt32 viewPortTopLeftX, UInt32 viewPortTopLeftY) {
      fbc_setCameraOverlaySettings(widthPercentage, viewPortTopLeftX, viewPortTopLeftY);
    }

  }
}
