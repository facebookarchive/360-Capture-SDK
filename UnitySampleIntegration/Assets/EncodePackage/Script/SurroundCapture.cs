using UnityEngine;
using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;
using System.Threading;


namespace FBCapture
{
    [RequireComponent(typeof(Camera))]
    public class SurroundCapture : MonoBehaviour
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

        public static SurroundCapture singleton;

        [Tooltip("Reference to camera that renders the scene")]
        public Camera sceneCamera;        

        [Tooltip("Offset spherical coordinates (shift equirect)")]
        public Vector2 sphereOffset = Vector2.zero;
        [Tooltip("Scale spherical coordinates (flip equirect, usually just 1 or -1)")]
        public Vector2 sphereScale = Vector2.one;

        public enum EncodingFormat {
            Cubemap,
            Equirect,
        }

        [Header("Surround Capture Format")]
        public EncodingFormat encodingFormat = EncodingFormat.Equirect;

        [Header("Cubemap Size")]
        public int cubemapSize = 1024;
        private int captureWidth = 2560;
        private int captureHeight = 1440;

        private int lastWidth = 0, lastHeight = 0;
                
        private RenderTexture cubemapTex;
        private RenderTexture outputTex;  // equirect or cubemap ends up here
        private RenderTexture externalTex;

        [Header("Surround Capture Shaders")]
        public Shader convertShader;
        public Shader outputCubemapShader;
        public Shader downSampleShader;

        private bool encodingStart = false;
        private bool encodingStop = false;
        private bool needToStopEncoding = false;        
      
        // Full path string of encoded moive         
        private string videoFullPath;

        // Full path of screenshot image        
        private string screenshotFullPath;

        // Live stream sever url
        private string streamServerUrl;
        
        // Live stream key
        private string streamKey;

        // Rotate camera for cubemap lookup
        private bool includeCameraRotation = false;

        // materials for equirect and cubemap
        private Material convertMaterial;
        private Material downSampleMaterial;
        private Material outputCubemapMaterial;

        // It sets video FPS
        protected int videoFPS = 30;
        protected float fps = 1f / 30.0f;
        protected float fpsTimer = 0.0f;

        // Event for managing thread
        protected ManualResetEvent threadResume;
        protected ManualResetEvent threadShutdown;
        protected Thread flushThread;
        protected Thread audioThread;


        protected bool flushReady = false;
        protected float flushTimer = 0.0f;
        protected float flushCycle = 5.0f;
        protected bool initialized = false;
        protected bool liveStreaming;
        public bool isLiveStreaming { get { return this.liveStreaming; } set { liveStreaming = value; } }           

        void Awake()
        {
#if (UNITY_ANDROID && !UNITY_EDITOR) || UNITY_STANDALONE_OSX || UNITY_EDITOR_OSX
        Destroy(gameObject);
        return;
#endif
            if (singleton != null) {
                DestroyImmediate(gameObject);
                return;
            }
            singleton = this;
        }

        void Start()
        {
            // create cubemap render texture
            cubemapTex = new RenderTexture(cubemapSize, cubemapSize, 0);
#if UNITY_5_4_OR_NEWER
            cubemapTex.dimension = UnityEngine.Rendering.TextureDimension.Cube;
#else
        cubemapTex.isCubemap = true;
#endif
            cubemapTex.hideFlags = HideFlags.HideAndDontSave;

            // create render texture for equirectangular image
            SetOutputSize(captureWidth, captureHeight);

            // create materials
            convertMaterial = CreateMaterial(convertShader, convertMaterial);
            downSampleMaterial = CreateMaterial(downSampleShader, downSampleMaterial);
            outputCubemapMaterial = CreateMaterial(outputCubemapShader, outputCubemapMaterial);
            initialized = true;

            encodingStart = false;
            encodingStop = false;        
        }

        private void MuxingThread()
        {
            while (true) {
                threadResume.WaitOne(Timeout.Infinite);
                if (!muxingData()) {
                    Debug.Log("Failed on mxuing video and audio data. Please check FBCaptureSDK.log file");                    
                }
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
                    if(!audioEncoding()) {
                        Debug.Log("Failed on audio encoding. Please check FBCaptureSDK.log file");
                    }
                }
                Thread.Sleep(10);
            }
        }


        protected Material CreateMaterial(Shader s, Material m2Create)
        {
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

        void OnDestroy()
        {
            DestroyImmediate(cubemapTex);
            DestroyImmediate(outputTex);
            DestroyImmediate(externalTex);      
            DestroyImmediate(convertMaterial);
            DestroyImmediate(downSampleMaterial);
            DestroyImmediate(outputCubemapMaterial);
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
                    if (sceneCamera) {
                        sceneCamera.transform.position = transform.position;
                        sceneCamera.RenderToCubemap(cubemapTex);  // render cubemap
                    }

                    if(!startEncoding(externalTex.GetNativeTexturePtr(), videoFullPath, liveStreaming, videoFPS, false)) {
                        Debug.Log("Failed to start encoding. Please check FBCaptureSDK.log file");
                    }

                    if (flushTimer > flushCycle && liveStreaming) {  // [Live] flush input buffers based on flush cycle value
                        flushTimer = 0.0f;
                        if(!stopEncoding()) {
                            Debug.Log("Failed to finalize inputs. Please check FBCaptureSDK.log file");
                        }
                        flushReady = true;
                    }
                    else if (encodingStop && !liveStreaming)  {  // flush input buffers when got stop input                                                
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
                    if (encodingStop) {
                        encodingStart = false;
                    }
                    threadResume.Set();
                }
            }
        }

        // Start video encoding    
        public void StartEncodingVideo(int width, int height, string moviePathName = "")
        {
            if (!string.IsNullOrEmpty(moviePathName)) {
                videoFullPath = moviePathName;
            }

            if (!encodingStart) {
                SetOutputSize(width, height);
                Debug.LogFormat("[SurroundCapture] Starting encoder {0} x {1}: {2}", width, height, videoFullPath);
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

        // Stop video encoding
        public void StopEncodingVideo()
        {            
            Debug.Log("Stop Encoding");
            needToStopEncoding = true;            
        }

        IEnumerator CaptureScreenshot(int width, int height) {
            SetOutputSize(width, height);
            // yield a frame to re-render into the rendertexture
            yield return new WaitForEndOfFrame();

            Debug.LogFormat("[SurroundCapture] Saved {0} x {1} screenshot: {2}", width, height, screenshotFullPath);
            if(!saveScreenShot(externalTex.GetNativeTexturePtr(), screenshotFullPath, false)) {
                Debug.Log("Failed on taking screenshot. Please check FBCaptureSDK.log file");
            }         
        }

        // Take screenshot
        public void TakeScreenshot(int width, int height, string screenshotPathName = "")
        {
            if (!string.IsNullOrEmpty(screenshotPathName)) {
                screenshotFullPath = screenshotPathName;
            }

            if (!encodingStart) {
                if (sceneCamera) {
                    sceneCamera.transform.position = transform.position;
                    sceneCamera.RenderToCubemap(cubemapTex);  // render cubemap
                }
            }
            
             StartCoroutine(CaptureScreenshot(width, height));            
        }

        void SetOutputSize(int width, int height) {

            if (width == lastWidth && height == lastHeight) {
                return;
            }
            else {
                lastWidth = width;
                lastHeight = height;
            }

            if (outputTex != null) {
                Destroy(outputTex);
            }

            outputTex = new RenderTexture(width, height, 0);
            outputTex.hideFlags = HideFlags.HideAndDontSave;

            if (externalTex != null) {
                Destroy(externalTex);
            }

            externalTex = new RenderTexture(width, height, 0);
            externalTex.hideFlags = HideFlags.HideAndDontSave;
        }

        void RenderCubeFace(CubemapFace face, float x, float y, float w, float h)
        {
            // texture coordinates for displaying each cube map face
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
            GL.TexCoord(faceTexCoords[i * 4]); GL.Vertex3(x, y, 0);
            GL.TexCoord(faceTexCoords[i * 4 + 1]); GL.Vertex3(x, y + h, 0);
            GL.TexCoord(faceTexCoords[i * 4 + 2]); GL.Vertex3(x + w, y + h, 0);
            GL.TexCoord(faceTexCoords[i * 4 + 3]); GL.Vertex3(x + w, y, 0);
            GL.End();

            GL.PopMatrix();
        }

        void SetMaterialParameters(Material material)
        {
            // convert to equirectangular
            material.SetTexture("_CubeTex", cubemapTex);
            material.SetVector("_SphereScale", sphereScale);
            material.SetVector("_SphereOffset", sphereOffset);

            if (includeCameraRotation) {  // cubemaps are always rendered along axes, so we do rotation by rotating the cubemap lookup                
                material.SetMatrix("_CubeTransform", Matrix4x4.TRS(Vector3.zero, transform.rotation, Vector3.one));
            }
            else {
                material.SetMatrix("_CubeTransform", Matrix4x4.identity);
            }
        }

        void DisplayCubeMap(RenderTexture dest)
        {
            SetMaterialParameters(outputCubemapMaterial);
            outputCubemapMaterial.SetPass(0);

            Graphics.SetRenderTarget(outputTex);

            float s = 1.0f / 3.0f;
            RenderCubeFace(CubemapFace.PositiveX, 0.0f, 0.5f, s, 0.5f);
            RenderCubeFace(CubemapFace.NegativeX, s, 0.5f, s, 0.5f);
            RenderCubeFace(CubemapFace.PositiveY, s * 2.0f, 0.5f, s, 0.5f);

            RenderCubeFace(CubemapFace.NegativeY, 0.0f, 0.0f, s, 0.5f);
            RenderCubeFace(CubemapFace.PositiveZ, s, 0.0f, s, 0.5f);
            RenderCubeFace(CubemapFace.NegativeZ, s * 2.0f, 0.0f, s, 0.5f);

            Graphics.SetRenderTarget(null);
            Graphics.Blit(outputTex, externalTex);            
            Debug.Log("DisplayCubeMap");
        }

        void DisplayEquirect(RenderTexture dest)
        {
            SetMaterialParameters(convertMaterial);
            Graphics.Blit(null, externalTex, convertMaterial);            
        }

        void OnRenderImage(RenderTexture src, RenderTexture dest)
        {
            if (!initialized) {
                Graphics.Blit(src, dest);
                return;
            }

            if (encodingFormat == EncodingFormat.Cubemap) {
                DisplayCubeMap(dest);
            }
            else {
                DisplayEquirect(dest);
            }
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