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
        [DllImport("HWEncoder", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool startEncoding(IntPtr texture, string path, bool isLive, int fps, bool needFlipping);
        [DllImport("HWEncoder", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool audioEncoding();
        [DllImport("HWEncoder", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool stopEncoding();
        [DllImport("HWEncoder", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool muxingData();
        [DllImport("HWEncoder", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern bool saveScreenShot(IntPtr texture, string path, bool needFlipping);

        public static NonSurroundCapture singleton;

        public RenderTexture renderTexture;

        private bool encodingStart = false;
        private bool encodingStop = false;
        private bool needToStopEncoding = false;

        private int lastWidth = 0, lastHeight = 0;

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
        }

        void Start()
        {
            encodingStart = false;
            encodingStop = false;            
        }

        private void MuxingThread()
        {
            while (true)
            {
                threadResume.WaitOne(Timeout.Infinite);
                muxingData();
                threadResume.Reset();
            }
        }

        private void AudioThread()
        {
            while (true)
            {
                if (needToStopEncoding)
                {
                    threadResume.Reset();
                }
                if (encodingStart && !needToStopEncoding)
                {
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
                fpsTimer = 0.0f;
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
                //SetOutputSize(width, height);
                Debug.LogFormat("[NonSurroundCapture] Starting encoder {0} x {1}: {2}", width, height, videoFullPath);
            }
            else {
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

            StartCoroutine(CaptureScreenshot(width, height));
        }

        IEnumerator CaptureScreenshot(int width, int height)
        {
            // yield a frame to re-render into the rendertexture
            yield return new WaitForEndOfFrame();

            Debug.LogFormat("[NonSurroundCapture] Saved {0} x {1} screenshot: {2}", width, height, screenshotFullPath);
            if(!saveScreenShot(renderTexture.GetNativeTexturePtr(), screenshotFullPath, true)) {
                Debug.Log("Failed on taking screenshot. Please check FBCaptureSDK.log file");
            }
        }

        void SetOutputSize(int width, int height)
        {
            if (width == lastWidth && height == lastHeight) {
                return;
            }
            else {
                lastWidth = width;
                lastHeight = height;
            }

            renderTexture = new RenderTexture(width, height, 0);
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
