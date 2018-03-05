using UnityEngine;
using UnityEngine.VR;
using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;
using System.Threading;


namespace FBCapture {
    [RequireComponent(typeof(Camera))]
    public class NonSurroundCapture : MonoBehaviour {
				[DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
				private static extern FBCAPTURE_STATUS startEncoding(IntPtr texture, string path, bool isLive, int bitrate, int fps, bool needFlipping);
				[DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
				private static extern FBCAPTURE_STATUS audioEncoding(bool useVRAudioResources, bool silenceMode, VRDeviceType vrDevice, string useMicIMMDeviceId);
				[DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
				private static extern FBCAPTURE_STATUS stopEncoding(bool forceStop);
				[DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
				private static extern FBCAPTURE_STATUS muxingData();
				[DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
				private static extern FBCAPTURE_STATUS startLiveStream(string streamUrl);
				[DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
				private static extern void stopLiveStream();
				[DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
				private static extern void resetResources();
				[DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
				private static extern FBCAPTURE_STATUS saveScreenShot(IntPtr texture, string path, bool needFlipping);

				#region CAPUTRE SDK

				public enum ErrorType {
								MUX_FAILED,
								INVALID_STREAM_URI,
								STREAM_FAILED_TO_START,
								STREAM_FAILURE,
								ENCODE_FAILED_TO_START,
								FINALIZE_INPUT_FAILED,
								ENCODE_INVALID_RESOLUTION,
								SCREENSHOT_FAILED,
						}

						public enum FBCAPTURE_STATUS {
								// Common
								OK = 0,
								ENCODE_IS_NOT_READY,
								NO_INPUT_FILE,
								FILE_READING_ERROR,
								OUTPUT_FILE_OPEN_FAILED,

								// Video/Image encoding specific errors
								UNSUPPORTED_GRAPHICS_CARD_DRIVER_VERSION = videoEncodingErrorCode,
								ENCODE_INIT_FAILED,
								ENCODE_SET_CONFIG_FAILED,
								ENCODER_CREATION_FAILED,
								INVALID_TEXTURE_POINTER,
								CONTEXT_CREATION_FAILED,
								TEXTURE_CREATION_FAILED,
								TEXTURE_RESOURCES_COPY_FAILED,
								IO_BUFFER_ALLOCATION_FAILED,
								ENCODE_PICTURE_FAILED,
								ENCODE_FLUSH_FAILED,

								// WIC specific error
								WIC_SAVE_IMAGE_FAILED,

								// Audio encoding specific errors
								AUDIO_DEVICE_ENUMERATION_FAILED = audioEncodingErrorCode,
								AUDIO_CLIENT_INIT_FAILED,
								WRITTING_WAV_HEADER_FAILED,
								RELEASING_WAV_FAILED,

								// Transcoding and muxing specific errors
								MF_CREATION_FAILED = transcodingMuxingErrorCode,
								MF_INIT_FAILED,
								MF_CREATING_WAV_FORMAT_FAILED,
								MF_TOPOLOGY_CREATION_FAILED,
								MF_TOPOLOGY_SET_FAILED,
								MF_TRANSFORM_NODE_SET_FAILED,
								MF_MEDIA_CREATION_FAILED,
								MF_HANDLING_MEDIA_SESSION_FAILED,

								// WAMEDIA muxing specific errors
								WAMDEIA_MUXING_FAILED,

								// RTMP specific errors
								INVALID_FLV_HEADER = rtmpErrorCode,
								INVALID_STREAM_URL,
								RTMP_CONNECTION_FAILED,
								RTMP_DISCONNECTED,
								SENDING_RTMP_PACKET_FAILED,
						}

						private enum VRDeviceType
						{
								UNKNOWN,
								OCULUS_RIFT,
								HTC_VIVE,
						}

						public delegate void OnErrorCallback(ErrorType error, FBCAPTURE_STATUS? captureStatus);

						public event OnErrorCallback OnError = delegate { };

						#endregion

        public static NonSurroundCapture singleton;

        private RenderTexture renderTexture;
        private RenderTexture outputTexture;

        public Shader flippingTextureShader;
        private Material flippingTextureMaterial;

        private volatile bool encodingStarted = false;
        private volatile bool needToStopEncoding = false;
        private volatile bool terminateThread = false;
        private volatile bool needAudioEncoding = false;
        private bool encodingStopped = false;

        private int lastWidth = 0, lastHeight = 0;
        private int captureWidth = 2048;
        private int captureHeight = 1024;

        [HideInInspector]
        public bool releasedResources = true;

        // video file name
        private string videoName;

        // Screenshot file name: Format should be jpg!!
        private string screenshotName;

        // Path where files will be saved
        private string saveFolder;

        // Full path string of encoded moive
        private string videoFullPath;

        // Full path of screenshot image
        private string screenshotFullPath;
    
        private Thread muxingThread;
        private Thread audioThread;

        [Tooltip("Reference to camera that renders the scene")]
        public Camera sceneCamera;

        // It sets video FPS
        int videoFPS = 30;
        float frameDuration = 1f / 30f;
        float fpsTimer = 0f;

        // Encoding bitrate
        int videoBitrate = 5000000;

        private bool flushReady = false;
        private bool needToSendFrame = false;
        private bool requestedFinalStream = false;
        private float flushTimer = 0f;

        const float initialFlushCycle = 5f;
        const float streamingFlushCycle = 5f;
        private float flushCycle = initialFlushCycle;

        //Set true if you want to go live
        public bool isLiveStreaming { get; set; }
        // Live stream sever url
        public string streamServerUrl { get; set; }

        //Set ture if you want to mute Audio
        public bool pauseAudioCapture { get; set; }

        private const int videoEncodingErrorCode = 100;
        private const int audioEncodingErrorCode = 200;
        private const int transcodingMuxingErrorCode = 300;
        private const int rtmpErrorCode = 400;

        static readonly AutoResetEvent muxingThreadManager = new AutoResetEvent(false);

        VRDeviceType attachedHMD;

        void Awake() {
            releasedResources = true;
#if (UNITY_ANDROID && !UNITY_EDITOR) || UNITY_STANDALONE_OSX || UNITY_EDITOR_OSX
                Destroy(gameObject);
                return;
#endif
            if (singleton != null) {
                DestroyImmediate(gameObject);
                return;
            }
            singleton = this;

            SetOutputSize(captureWidth, captureHeight);
        }

        void Start() {
            encodingStarted = false;
            encodingStopped = false;
            pauseAudioCapture = false;

            flippingTextureMaterial = new Material(flippingTextureShader);
            flippingTextureMaterial.hideFlags = HideFlags.HideAndDontSave;
            
            terminateThread = false;

            // Since Spaces only support two HMD at the moment, we want to check only two devices now.
            string vrDeviceName = VRDevice.model.ToLower();
            if (vrDeviceName.Contains("rift")) {
                attachedHMD = VRDeviceType.OCULUS_RIFT;
            } else if (vrDeviceName.Contains("vive")) {
                attachedHMD = VRDeviceType.HTC_VIVE;
            } else {
                attachedHMD = VRDeviceType.UNKNOWN;
            }
        }

        private void MuxingThread() {
            while (!terminateThread) {

                muxingThreadManager.WaitOne();

                FBCAPTURE_STATUS status;
                status = muxingData();
                if (status != FBCAPTURE_STATUS.OK) {
                    Debug.Log("Failed on mxuing video and audio data. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                    OnError(ErrorType.MUX_FAILED, status);
                    encodingStarted = false;
                } else if (isLiveStreaming) {
                    bool needToReleaseResources = false;
                    if (string.IsNullOrEmpty(streamServerUrl)) {
                        Debug.Log("You should have live stream server url.");
                        OnError(ErrorType.INVALID_STREAM_URI, null);
                    } else if (needToSendFrame) {
                        status = startLiveStream(streamServerUrl);
                        if (status != FBCAPTURE_STATUS.OK) {
                            Debug.Log("Failed on streaming. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                            OnError(ErrorType.STREAM_FAILED_TO_START, status);
                            needToReleaseResources = true;
                        } else if (requestedFinalStream && encodingStopped) {
                            Debug.Log("Stop live streaming and clean resources");
                            needToReleaseResources = true;
                        }
                    }

                    // Reset streaming resources and settings
                    if (needToReleaseResources) {
                        stopLiveStream();
                        resetResources();
                        needToReleaseResources = false;
                        releasedResources = true;
                        encodingStarted = false;
                    }
                } else {
                    resetResources();
                    Debug.Log("muxing is done for record mode");
                }
            }
            Debug.Log("muxing thread is terminated");
        }

        private void AudioThread() {
            FBCAPTURE_STATUS status;
            while (!terminateThread) {
                if (needAudioEncoding) {
                    // useVRAudioResources should be true in Rift or Vive.
                    status = audioEncoding(useVRAudioResources: false, silenceMode: pauseAudioCapture, vrDevice: attachedHMD, useMicIMMDeviceId: null);
                    if (status != FBCAPTURE_STATUS.OK) {
                        Debug.Log("Failed on audio encoding. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                        needAudioEncoding = false;
                    }
                }
                Thread.Sleep(10);
            }

            Debug.Log("audio thread is terminated");
        }

        void Update() {
            FBCAPTURE_STATUS status;

            if (needToStopEncoding) {  // Stop encoding
                encodingStopped = true;
            }

            if (encodingStarted) {
                flushReady = false;
            }

            flushTimer += Time.deltaTime;
            fpsTimer += Time.deltaTime;

            if (fpsTimer >= frameDuration) {
                fpsTimer -= frameDuration;
                if (encodingStarted) {
                    status = startEncoding(outputTexture.GetNativeTexturePtr(), videoFullPath, isLiveStreaming, videoBitrate, videoFPS, false);
                    if (status != FBCAPTURE_STATUS.OK) {
                        Debug.Log("Failed to start encoding. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                        OnError(ErrorType.ENCODE_FAILED_TO_START, status);
                        encodingStarted = false;
                        return;
                    }

                    if (flushTimer > flushCycle && isLiveStreaming) {  // [Live] flush input buffers based on flush cycle value
                        flushCycle = streamingFlushCycle;  // Change flush cycle after first stream
                        flushTimer = 0.0f;
                        fpsTimer = 0.0f;
                        status = stopEncoding(forceStop: false);
                        if (status != FBCAPTURE_STATUS.OK) {
                            Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                            OnError(ErrorType.FINALIZE_INPUT_FAILED, status);
                            encodingStarted = false;
                            return;
                        }
                        flushReady = true;
                        requestedFinalStream = false;
                    } else if (encodingStopped && !isLiveStreaming) {  // [VOD] Flush input buffers when got stop input
                        needAudioEncoding = false;
                        status = stopEncoding(forceStop: true);
                        if (status != FBCAPTURE_STATUS.OK) {
                            Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                            OnError(ErrorType.FINALIZE_INPUT_FAILED, status);
                            encodingStarted = false;
                            return;
                        }
                        flushReady = true;
                    }
                }
            }
           
            // [Live] Flush final input buffers by force
            if (encodingStopped && isLiveStreaming && !requestedFinalStream) {
                flushTimer = 0.0f;
                fpsTimer = 0.0f;
                needAudioEncoding = false;
                status = stopEncoding(forceStop: true);
                if (status != FBCAPTURE_STATUS.OK) {
                    Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                    OnError(ErrorType.FINALIZE_INPUT_FAILED, status);
                    encodingStarted = false;
                    return;
                }
                flushReady = true;
                requestedFinalStream = true;
            }

            // Muxing
            if (flushReady && !isLiveStreaming) {  // [VOD] Push inputs and Stop encoding
                flushReady = false;
                encodingStarted = false;
                muxingThreadManager.Set();
            } else if (flushReady && isLiveStreaming) {  // [Live] Restart encoding after flush
                flushReady = false;
                muxingThreadManager.Set();
                needToSendFrame = true;
                if (encodingStopped) {
                    encodingStarted = false;
                }
            }
        }

        public void StartEncodingVideo(int width, int height, string moviePathName = "") {
            if (!string.IsNullOrEmpty(moviePathName)) {
                videoFullPath = moviePathName;
            }

            if (encodingStarted) {
                Debug.Log("Encoding is already started");
                return;
            } else {
                if (width > 0 && height > 0) {
                    Debug.LogFormat("[NonSurroundCapture] Starting encoder {0} x {1}: {2}", width, height, videoFullPath);
                } else {
                    Debug.LogFormat("Start Encoding is failed by invalid resolution: {0} x {1}", width, height);
                    OnError(ErrorType.ENCODE_INVALID_RESOLUTION, null);
                    return;
                }
            }

            SetOutputSize(width, height);

            if (muxingThread == null) {
                muxingThread = new Thread(MuxingThread);
                muxingThread.Start();
            }

            if (audioThread == null) {
                audioThread = new Thread(AudioThread);
                audioThread.Start();
            }

            encodingStarted = true;
            encodingStopped = false;
            needToStopEncoding = false;
            needToSendFrame = false;
            releasedResources = false;
            requestedFinalStream = false;
            needAudioEncoding = true;

            flushTimer = 0.0f;
            fpsTimer = 0.0f;
            flushCycle = initialFlushCycle;
        }

        public void StopEncodingVideo() {
            Debug.Log("Stop Encoding");
            needToStopEncoding = true;
        }

        // Take screenshot
        public void TakeScreenshot(int width, int height, string screenshotPathName = "") {
            if (!string.IsNullOrEmpty(screenshotPathName)) {
                screenshotFullPath = screenshotPathName;
            }

            if (SetOutputSize(width, height)) {
                StartCoroutine(CaptureScreenshot(width, height));
            }
        }

        IEnumerator CaptureScreenshot(int width, int height) {
            FBCAPTURE_STATUS status;

            // yield a frame to re-render into the rendertexture
            yield return new WaitForEndOfFrame();

            status = saveScreenShot(outputTexture.GetNativeTexturePtr(), screenshotFullPath, false);

            if (status != FBCAPTURE_STATUS.OK) {
                Debug.Log("Failed on taking screenshot. Please check FBCaptureSDK.log file");
                OnError(ErrorType.SCREENSHOT_FAILED, status);
            }

            Debug.LogFormat("[NonSurroundCapture] Saved {0} x {1} screenshot: {2}", width, height, screenshotFullPath);
        }

        void OnRenderImage(RenderTexture src, RenderTexture dest) {
            Graphics.Blit(renderTexture, outputTexture, flippingTextureMaterial);

        }

        bool SetOutputSize(int width, int height) {
            if (width == 0 || height == 0) {
                Debug.Log("The width and height shouldn't be zero");
                return false;
            } else if (width == lastWidth && height == lastHeight) {
                return true;
            } else {
                lastWidth = width;
                lastHeight = height;
            }

            renderTexture = new RenderTexture(lastWidth, lastHeight, 24, RenderTextureFormat.ARGB32, RenderTextureReadWrite.Default);
            sceneCamera.targetTexture = renderTexture;

            outputTexture = new RenderTexture(lastWidth, lastHeight, 0);
            outputTexture.hideFlags = HideFlags.HideAndDontSave;

            return true;
        }

        void OnDestroy() {
            Destroy(renderTexture);
            Destroy(outputTexture);
      
            terminateThread = true;
            muxingThreadManager.Set();

            if (muxingThread != null) {
                muxingThread.Join();
                muxingThread = null;
            }

            if (audioThread != null) {
                audioThread.Join();
                audioThread = null;
            }
        }

        void OnApplicationQuit() {
            if (encodingStarted) {
                stopEncoding(forceStop: true);
            }

            if (isLiveStreaming) {
                stopLiveStream();
            }

            resetResources();
        }
    }
}
