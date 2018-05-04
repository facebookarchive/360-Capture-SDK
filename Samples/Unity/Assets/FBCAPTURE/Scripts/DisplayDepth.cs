using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace FBCapture {
    public class DisplayDepth : MonoBehaviour {
        public Shader depthShader;
        private Material depthMaterial;

        void Start() {
            var cam = GetComponent<Camera>();
            cam.depthTextureMode = DepthTextureMode.Depth;
            depthMaterial = CreateMaterial(depthShader, depthMaterial);
        }

        void OnRenderImage(RenderTexture src, RenderTexture dest) {
            Graphics.Blit(src, dest, depthMaterial);
        }

        /// <summary>
        /// Create materials which will be used for depth generation
        /// </summary>
        /// <param name="s"> shader code </param>
        /// <param name="m2Create"> material </param>
        /// <returns> material </returns>
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
    }
}
