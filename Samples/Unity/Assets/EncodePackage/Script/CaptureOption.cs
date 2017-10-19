using UnityEngine;
using System.Collections;
using System.IO;
using System;


namespace FBCapture {
    public class CaptureOption : MonoBehaviour {
        [Header("Capture Option")]
        public bool doSurroundCaptureOption;

        [Header("Live Option")]
        public bool liveStreamingOption;
        public string streamUrlOption;
        public string streamKeyOption;

        [Header("Capture Hotkeys")]
        public KeyCode screenShotKey = KeyCode.None;
        public KeyCode encodingStartShotKey = KeyCode.None;
        public KeyCode encodingStopShotKey = KeyCode.None;

        [Header("Image and Video Size")]
        public int screenShotWidth = 2048;
        public int screenShotHeight = 1024;
        public int videoWidth = 2560;
        public int videoHeight = 1440;

        [Header("Encoding Options")]
        public int fps = 30;
        public int bitrate = 5000000;

        private SurroundCapture surroundCapture = null;
        private NonSurroundCapture nonSurroundCapture = null;

        [Header("Output Path")]
        public string outputPath;  // Path where created files will be saved

        private bool currDoSurroundCapture;
        private bool doSurroundCapture {
            set {
                if (currDoSurroundCapture == value)
                    return;

                if (!surroundCapture.releasedResources || !nonSurroundCapture.releasedResources) {
                    Debug.Log("Cannot change capture option while capture encoding is happening.");
                    doSurroundCaptureOption = currDoSurroundCapture;
                    return;
                }

                currDoSurroundCapture = value;
                surroundCapture.enabled = currDoSurroundCapture;
                nonSurroundCapture.enabled = !currDoSurroundCapture;
                Debug.LogFormat("DoSurroundCapture {0}", currDoSurroundCapture ? "enabled" : "disabled");
            }
        }

        private bool currLiveStreaming;
        private bool liveStreaming {
            set {
                if (currLiveStreaming == value)
                    return;

                if (!surroundCapture.releasedResources || !nonSurroundCapture.releasedResources) {
                    Debug.Log("Cannot change capture option while capture encoding is happening.");
                    liveStreamingOption = currLiveStreaming;
                    return;
                }

                currLiveStreaming = value;
                surroundCapture.isLiveStreaming = currDoSurroundCapture && currLiveStreaming;
                nonSurroundCapture.enabled = !currDoSurroundCapture && currLiveStreaming;
                liveStreamServerUrl = streamKeyOption;
                Debug.LogFormat("LiveStreaming {0}", currLiveStreaming ? "enabled" : "disabled");
            }
        }

        private string currLiveStreamServerUrl;
        private string currStreamKey;
        private string liveStreamServerUrl {
            set {
                if (!currLiveStreaming || currStreamKey == value)
                    return;

                if (!surroundCapture.releasedResources || !nonSurroundCapture.releasedResources) {
                    Debug.Log("Cannot change capture option while capture encoding is happening.");
                    streamKeyOption = currStreamKey;
                    return;
                }

                currStreamKey = value;
                string streamServerUrl = StreamServerURL();
                if (streamServerUrl == currLiveStreamServerUrl)
                    return;

                currLiveStreamServerUrl = streamServerUrl;
                if (currDoSurroundCapture)
                    surroundCapture.streamServerUrl = currLiveStreamServerUrl;
                else
                    nonSurroundCapture.streamServerUrl = currLiveStreamServerUrl;
                Debug.LogFormat("LiveStreamServerUrl: {0}", currLiveStreamServerUrl);
            }
        }

        void Start() {
            if (string.IsNullOrEmpty(outputPath)) {
                outputPath = System.IO.Path.Combine(Directory.GetCurrentDirectory(), "Gallery");
                // create the directory
                if (!Directory.Exists(outputPath)) {
                    Directory.CreateDirectory(outputPath);
                }
            }

            surroundCapture = GetComponent<SurroundCapture>();
            nonSurroundCapture = GetComponent<NonSurroundCapture>();

            doSurroundCapture = doSurroundCaptureOption;
            liveStreaming = liveStreamingOption;
            liveStreamServerUrl = streamKeyOption;
        }

        void Update() {
            // Check in real time if capture option is changed if there is no encoding session happening at the moment
            doSurroundCapture = doSurroundCaptureOption;
            liveStreaming = liveStreamingOption;
            liveStreamServerUrl = streamKeyOption;

            // 360 screen capturing
            if (Input.GetKeyDown(screenShotKey) && currDoSurroundCapture) {
                surroundCapture.TakeScreenshot(screenShotWidth, screenShotHeight, ScreenShotName(screenShotWidth, screenShotHeight));
            } else if (Input.GetKeyDown(encodingStartShotKey) && currDoSurroundCapture) {
                surroundCapture.StartEncodingVideo(videoWidth, videoHeight, fps, bitrate, MovieName(videoWidth, videoHeight));
            } else if (Input.GetKeyDown(encodingStopShotKey) && currDoSurroundCapture) {
                surroundCapture.StopEncodingVideo();
            }

            // 2D screen capturing
            if (Input.GetKeyDown(screenShotKey) && !currDoSurroundCapture) {
                nonSurroundCapture.TakeScreenshot(screenShotWidth, screenShotHeight, ScreenShotName(screenShotWidth, screenShotHeight));
            } else if (Input.GetKeyDown(encodingStartShotKey) && !currDoSurroundCapture) {
                nonSurroundCapture.StartEncodingVideo(videoWidth, videoHeight, fps, bitrate, MovieName(videoWidth, videoHeight));
            } else if (Input.GetKeyDown(encodingStopShotKey) && !currDoSurroundCapture) {
                nonSurroundCapture.StopEncodingVideo();
            }
        }

        string StreamServerURL() {
            if (streamUrlOption.EndsWith("/")) {
                streamUrlOption = streamUrlOption.Remove(streamUrlOption.Length - 1);
            }
            return streamUrlOption + '/' + streamKeyOption;
        }

        string MovieName(int width, int height) {
            return string.Format("{0}/movie_{1}x{2}_{3}.h264",
                                outputPath,
                                width, height,
                                DateTime.Now.ToString("yyyy-MM-dd hh_mm_ss"));
        }

        string ScreenShotName(int width, int height) {
            return string.Format("{0}/screenshot_{1}x{2}_{3}.jpg",
                                outputPath,
                                width, height,
                                DateTime.Now.ToString("yyyy-MM-dd hh_mm_ss"));
        }
    }
}
