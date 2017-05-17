using UnityEngine;
using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;
using System.Threading;


namespace FBCapture
{
    [RequireComponent(typeof(Camera))]
    public class NonSurroundCapture : MonoBehaviour
    {
        [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool startEncoding(IntPtr texture, string path, bool isLive, int fps, bool needFlipping);
        [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool audioEncoding();
        [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool stopEncoding();
        [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool muxingData();
        [DllImport("FBCapture", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool saveScreenShot(IntPtr texture, string path, bool needFlipping);

        public static NonSurroundCapture singleton;

        private RenderTexture renderTexture;

        private bool encodingStart = false;
        private bool encodingStop = false;
        private bool needToStopEncoding = false;

        private int lastWidth = 0, lastHeight = 0;
        private int captureWidth = 2048;
        private int captureHeight = 1024;

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

        // Event for managing thread
        private ManualResetEvent threadResume;
        private ManualResetEvent threadShutdown;
        private Thread flushThread;
        private Thread audioThread;

        [Tooltip("Reference to camera that renders the scene")]
        public Camera sceneCamera;

        // It sets video FPS
        int videoFPS = 30;
        float fps = 1f / 30.0f;
        float fpsTimer = 0.0f;

        private bool flushReady = false;
        private float flushTimer = 0.0f;
        private float flushCycle = 5.0f;        
        private bool liveStreaming;
        public bool isLiveStreaming { get { return this.liveStreaming; } set { liveStreaming = value; } }

        void Awake()
        {
#if (UNITY_ANDROID && !UNITY_EDITOR) || UNITY_STANDALONE_OSX || UNITY_EDITOR_OSX
                Destroy(gameObject);
                return;
#endif
            if (singleton != null)
            {
                DestroyImmediate(gameObject);
                return;
            }
            singleton = this;

            SetOutputSize(captureWidth, captureHeight);            
        }

        void Start()
        {
            encodingStart = false;
            encodingStop = false;            
        }

        private void MuxingThread()
        {
            while (true) {
                threadResume.WaitOne(Timeout.Infinite);
                muxingData();
                threadResume.Reset();
            }
        }

        private void AudioThread()
        {
            while (true) {
                if (needToStopEncoding) {
                    threadResume.Reset();
                }
                else if (encodingStart && !needToStopEncoding) {
                    audioEncoding();
                }
                Thread.Sleep(10);
            }
        }

        void Update()
        {
            if (needToStopEncoding) {  // Stop encoding 
                encodingStop = true;
            }

            if (encodingStart) {
                flushReady = false;
            }

            flushTimer += Time.deltaTime;
            fpsTimer += Time.deltaTime;

            if (fpsTimer >= fps) {
                fpsTimer -= fps;
                if (encodingStart) {
                    if (!startEncoding(renderTexture.GetNativeTexturePtr(), videoFullPath, liveStreaming, videoFPS, true)) {
                        Debug.Log("Failed to start encoding. Please check FBCaptureSDK.log file");
                    }

                    if (flushTimer > flushCycle && liveStreaming) {  // [Live] flush input buffers based on flush cycle value
                        flushTimer = 0.0f;
                        if (!stopEncoding()) {
                            Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file");
                        }
                        flushReady = true;
                    }
                    else if (encodingStop && !liveStreaming) {  // flush input buffers when got stop input                                                
                        if (!stopEncoding()) {
                            Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file");
                        }
                        flushReady = true;
                    }
                }

                // Muxing
                if (flushReady && !liveStreaming) {  // Flush inputs and Stop encoding
                    flushReady = false;
                    encodingStart = false;
                    threadResume.Set();
                }
                else if (flushReady && liveStreaming) {  // Restart encoding after flush
                    flushReady = false;
                    if (encodingStop)
                    {
                        encodingStart = false;
                    }
                    threadResume.Set();
                }
            }
        }
      
        public void StartEncodingVideo(int width, int height, string moviePathName = "")
        {            
            if (!string.IsNullOrEmpty(moviePathName)) {
                videoFullPath = moviePathName;
            }

            if (!encodingStart) {
                if (SetOutputSize(width, height)) {                    
                    Debug.LogFormat("[NonSurroundCapture] Starting encoder {0} x {1}: {2}", width, height, videoFullPath);                    
                }
                else {
                    Debug.LogFormat("Start Encoding is failed by invalid resolution: {0} x {1}", width, height);
                    return;
                }
            }
            else {
                Debug.Log("Encoding is already started");
                return;
            }

            encodingStart = true;
            encodingStop = false;
            needToStopEncoding = false;            

            flushTimer = 0.0f;
            fpsTimer = 0.0f;

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
        }

        public void StopEncodingVideo()
        {            
            Debug.Log("Stop Encoding");
            needToStopEncoding = true;
        }       

        // Take screenshot
        public void TakeScreenshot(int width, int height, string screenshotPathName = "")
        {
            if (!string.IsNullOrEmpty(screenshotPathName)) {
                screenshotFullPath = screenshotPathName;
            }

            if (SetOutputSize(width, height)) {
                StartCoroutine(CaptureScreenshot(width, height));
            }
        }

        IEnumerator CaptureScreenshot(int width, int height)
        {
            SetOutputSize(width, height);
            // yield a frame to re-render into the rendertexture
            yield return new WaitForEndOfFrame();
                        
            if (!saveScreenShot(renderTexture.GetNativeTexturePtr(), screenshotFullPath, true)) {
                Debug.Log("Failed on taking screenshot. Please check FBCaptureSDK.log file");
            }

            Debug.LogFormat("[NonSurroundCapture] Saved {0} x {1} screenshot: {2}", width, height, screenshotFullPath);
        }

        bool SetOutputSize(int width, int height)
        {
            if (width == 0 || height == 0) {
                Debug.Log("The width and height shouldn't be zero");
                return false;
            }
            else if (!CheckPowerOfTwo(width, height)) {
                Debug.Log("The width and height should be power of two in Non SurrondCapture");
                return false;
            }

            renderTexture = new RenderTexture(width, height, 24, RenderTextureFormat.ARGB32, RenderTextureReadWrite.Default);            
            sceneCamera.targetTexture = renderTexture;

            return true;
        }

        bool CheckPowerOfTwo(int width, int height)
        {
            return (width & (width - 1)) == 0 && (height & (height -1)) == 0;
        }

        void OnDestroy()
        {
            DestroyImmediate(renderTexture);
        }

            void OnApplicationQuit()
        {
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
        }
    }
}
