using UnityEngine;
using System.Collections;
using System.IO;
using System;


namespace FBCapture
{
    public class CaptureOption : MonoBehaviour
    {      
        [Header("Capture Option")]
        public bool doSurroundCapture;

        [Header("Capture Hotkeys")]
        public KeyCode screenShotKey = KeyCode.None;
        public KeyCode encodingStartShotKey = KeyCode.None;  
        public KeyCode encodingStopShotKey = KeyCode.None;  

        [Header("Image and Video Size")]
        public int screenShotWidth = 2048;
        public int screenShotHeight = 1024;
        public int videoWidth = 2560;
        public int videoHeight = 1440;
               
        private SurroundCapture surroundCapture = null;
        private NonSurroundCapture nonSurroundCapture = null;
        
        public string outputPath;  // Path where created files will be saved 
        private bool liveStreaming = false; // Set false by force because not fully implemented
        
        void Start()
        {            
            if (string.IsNullOrEmpty(outputPath)) {
                outputPath = System.IO.Path.Combine(Directory.GetCurrentDirectory(), "Gallery");
                // create the directory
                if (!Directory.Exists(outputPath)) {
                    Directory.CreateDirectory(outputPath);
                }
            }

            surroundCapture = GetComponent<SurroundCapture>();
            nonSurroundCapture = GetComponent<NonSurroundCapture>();

            if (doSurroundCapture) {               
                surroundCapture.enabled = true;
                nonSurroundCapture.enabled = false;
                surroundCapture.isLiveStreaming = liveStreaming;
            }
            else {
                nonSurroundCapture.enabled = true;
                surroundCapture.enabled = false;
                nonSurroundCapture.isLiveStreaming = liveStreaming;
            }
        }

        void Update()
        {
            // 360 screen capturing
            if (Input.GetKeyDown(screenShotKey) && doSurroundCapture) {
                surroundCapture.TakeScreenshot(screenShotWidth, screenShotHeight, ScreenShotName(screenShotWidth, screenShotHeight));
            }

            else if (Input.GetKeyDown(encodingStartShotKey) && doSurroundCapture) {
                surroundCapture.StartEncodingVideo(videoWidth, videoHeight, MovieName(videoWidth, videoHeight));
            }

            else if (Input.GetKeyDown(encodingStopShotKey) && doSurroundCapture) {
                surroundCapture.StopEncodingVideo();
            }

            // 2D screen capturing
            if (Input.GetKeyDown(screenShotKey) && !doSurroundCapture) {
                nonSurroundCapture.TakeScreenshot(screenShotWidth, screenShotHeight, ScreenShotName(screenShotWidth, screenShotHeight));
            }

            else if (Input.GetKeyDown(encodingStartShotKey) && !doSurroundCapture) {
                nonSurroundCapture.StartEncodingVideo(videoWidth, videoHeight, MovieName(videoWidth, videoHeight));
            }

            else if (Input.GetKeyDown(encodingStopShotKey) && !doSurroundCapture) {
                nonSurroundCapture.StopEncodingVideo();
            }
        }

        string MovieName(int width, int height)
        {
            return string.Format("{0}/movie_{1}x{2}_{3}.h264",
                                outputPath,
                                width, height,
                                DateTime.Now.ToString("yyyy-MM-dd hh_mm_ss"));
        }

        string ScreenShotName(int width, int height)
        {            
            return string.Format("{0}/screenshot_{1}x{2}_{3}.jpg",
                                outputPath,
                                width, height,
                                DateTime.Now.ToString("yyyy-MM-dd hh_mm_ss"));
        }
    }
}
