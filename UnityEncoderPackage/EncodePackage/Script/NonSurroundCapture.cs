using UnityEngine;
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
        private static extern FBCAPTURE_STATUS audioEncoding(bool useRiftAudioSource, bool silenceMode);
        [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern FBCAPTURE_STATUS stopEncoding();
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

        public delegate void OnErrorCallback(ErrorType error, FBCAPTURE_STATUS? captureStatus);

        public event OnErrorCallback OnError = delegate { };

        #endregion

        public static NonSurroundCapture singleton;

        private RenderTexture renderTexture;
        private RenderTexture outputTexture;

        public Shader flippingTextureShader;
        private Material flippingTextureMaterial;

        private bool encodingStart = false;
        private bool encodingStop = false;
        private bool needToStopEncoding = false;

        private int captureWidth = 2048;
        private int captureHeight = 1024;

        // Full path string of encoded moive
        private string videoFullPath;

        // Full path of screenshot image
        private string screenshotFullPath;

        // Event for managing thread
        private ManualResetEvent threadResume;
        private ManualResetEvent threadShutdown;
        private Thread flushThread;
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

        public bool releasedResources = true;

        private const int videoEncodingErrorCode = 100;
        private const int audioEncodingErrorCode = 200;
        private const int transcodingMuxingErrorCode = 300;
        private const int rtmpErrorCode = 400;

        void Awake() {
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
            encodingStart = false;
            encodingStop = false;
            pauseAudioCapture = false;

            flippingTextureMaterial = new Material(flippingTextureShader);
            flippingTextureMaterial.hideFlags = HideFlags.HideAndDontSave;

            releasedResources = true;
        }

        private void MuxingThread() {
            while (true) {
                threadResume.WaitOne(Timeout.Infinite);

                FBCAPTURE_STATUS status;

                status = muxingData();
                if (status != FBCAPTURE_STATUS.OK) {
                    Debug.Log("Failed on mxuing video and audio data. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                    OnError(ErrorType.MUX_FAILED, status);
                }

                if (isLiveStreaming) {
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
                        } else if (requestedFinalStream && encodingStop) {
                            Debug.Log("Stop live streaming and clean resources");
                            requestedFinalStream = false;
                            needToReleaseResources = true;
                        }
                    }

                    // Reset streaming resources and settings
                    if (needToReleaseResources) {
                        stopLiveStream();
                        resetResources();
                        releasedResources = true;
                    }
                } else {
                    resetResources();
                    releasedResources = true;
                }

                threadResume.Reset();
            }
        }

        private void AudioThread() {
            while (true) {
                if (needToStopEncoding) {
                    threadResume.Reset();
                } else if (encodingStart && !needToStopEncoding) {
                    audioEncoding(useRiftAudioSource: false, silenceMode: pauseAudioCapture);
                }
                Thread.Sleep(10);
            }
        }

        void Update() {
            FBCAPTURE_STATUS status;

            if (needToStopEncoding) {  // Stop encoding
                encodingStop = true;
            }

            if (encodingStart) {
                flushReady = false;
            }

            flushTimer += Time.deltaTime;
            fpsTimer += Time.deltaTime;

            if (fpsTimer >= frameDuration) {
                fpsTimer -= frameDuration;
                if (encodingStart) {
                    status = startEncoding(outputTexture.GetNativeTexturePtr(), videoFullPath, isLiveStreaming, videoBitrate, videoFPS, false);
                    if (status != FBCAPTURE_STATUS.OK) {
                        Debug.Log("Failed to start encoding. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                        OnError(ErrorType.ENCODE_FAILED_TO_START, status);
                    }

                    if (flushTimer > flushCycle && isLiveStreaming) {  // [Live] flush input buffers based on flush cycle value
                        flushCycle = streamingFlushCycle;  // Change flush cycle after first stream
                        flushTimer = 0.0f;
                        fpsTimer = 0.0f;
                        status = stopEncoding();
                        if (status != FBCAPTURE_STATUS.OK) {
                            Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                            OnError(ErrorType.FINALIZE_INPUT_FAILED, status);
                        }
                        flushReady = true;
                        needToSendFrame = true;
                        requestedFinalStream = false;
                    } else if (encodingStop && !isLiveStreaming) {  // [VOD] Flush input buffers when got stop input
                        status = stopEncoding();
                        if (status != FBCAPTURE_STATUS.OK) {
                            Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                            OnError(ErrorType.FINALIZE_INPUT_FAILED, status);
                        }
                        flushReady = true;
                    }
                }
            }

            // [Live] Flush final input buffers by force
            if (encodingStop && isLiveStreaming && !requestedFinalStream && !releasedResources) {
                flushTimer = 0.0f;
                fpsTimer = 0.0f;
                status = stopEncoding();
                if (status != FBCAPTURE_STATUS.OK) {
                    Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file for more information. [Error Type: " + status + "]");
                    OnError(ErrorType.FINALIZE_INPUT_FAILED, status);
                }
                flushReady = true;
                requestedFinalStream = true;
                needToSendFrame = true;
            }

            // Muxing
            if (flushReady && !isLiveStreaming) {  // [VOD] Push inputs and Stop encoding
                flushReady = false;
                encodingStart = false;
                threadResume.Set();
                Debug.LogFormat("[NonSurroundCapture] Stopped encoder. Saved {0}", videoFullPath.Remove(videoFullPath.Length - 4) + "mp4");
            } else if (flushReady && isLiveStreaming) {  // [Live] Restart encoding after flush
                flushReady = false;
                if (encodingStop) {
                    encodingStart = false;
                }
                threadResume.Set();
            }
        }

        public void StartEncodingVideo(int width, int height, int fps, int bitrate, string moviePathName = "") {
            if (releasedResources) {
                if (!string.IsNullOrEmpty(moviePathName)) {
                    videoFullPath = moviePathName;
                }

                videoBitrate = bitrate;
                videoFPS = fps;
                frameDuration = 1.0f / fps;
            }

            if (!encodingStart) {
                if (width > 0 && height > 0) {
                    if (isLiveStreaming) {
                        Debug.LogFormat("[NonSurroundCapture] Streaming started (w:{0}xh:{1}). Stream server URL: {2}", width, height, streamServerUrl);
                    } else {
                        Debug.LogFormat("[NonSurroundCapture] Starting encoder {0} x {1}: {2}", width, height, videoFullPath);
                    }
                } else {
                    Debug.LogFormat("Start Encoding is failed by invalid resolution: {0} x {1}", width, height);
                    OnError(ErrorType.ENCODE_INVALID_RESOLUTION, null);
                    return;
                }
            } else {
                Debug.Log("Encoding is already started");
                return;
            }

            if (threadResume == null) {
                threadResume = new ManualResetEvent(true);
            }
            if (threadShutdown == null) {
                threadShutdown = new ManualResetEvent(false);
            }
            if (flushThread == null) {
                flushThread = new Thread(MuxingThread);
                flushThread.Start();
            }
            if (audioThread == null) {
                audioThread = new Thread(AudioThread);
                audioThread.Start();
            }

            encodingStart = true;
            encodingStop = false;
            needToStopEncoding = false;
            needToSendFrame = false;
            releasedResources = false;

            flushTimer = 0.0f;
            fpsTimer = 0.0f;
            flushCycle = initialFlushCycle;
        }

        public void StopEncodingVideo() {
            needToStopEncoding = true;
        }

        // Take screenshot
        public void TakeScreenshot(int width, int height, string screenshotPathName = "") {
            if (encodingStart) {
                Debug.Log("Cannot take screenshot. Video capture encoding in progress.");
                return;
            }

            if (!string.IsNullOrEmpty(screenshotPathName)) {
                screenshotFullPath = screenshotPathName;
            }

            if (SetOutputSize(width, height)) {
                StartCoroutine(CaptureScreenshot(width, height));
            }
        }

        IEnumerator CaptureScreenshot(int width, int height) {
            FBCAPTURE_STATUS status;

            SetOutputSize(width, height);
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
            }

            renderTexture = new RenderTexture(width, height, 24, RenderTextureFormat.ARGB32, RenderTextureReadWrite.Default);
            sceneCamera.targetTexture = renderTexture;

            outputTexture = new RenderTexture(width, height, 0);
            outputTexture.hideFlags = HideFlags.HideAndDontSave;

            return true;
        }

        void OnDestroy() {
            Destroy(renderTexture);
            Destroy(outputTexture);
        }

        void OnApplicationQuit() {
            if (encodingStart) {
                stopEncoding();
                threadResume.Set();
                threadShutdown.Set();
            }

            if (flushThread != null) {
                flushThread.Abort();
            }

            if (audioThread != null) {
                audioThread.Abort();
            }

            if (isLiveStreaming) {
                stopLiveStream();
            }
            resetResources();
        }
    }
}
