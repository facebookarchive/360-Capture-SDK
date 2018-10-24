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

namespace FBCapture {
	public class DisplayDepth : MonoBehaviour {
		public Shader depthShader;
		private Material depthMaterial;
		private Camera depthCamera;

		private void Start() {
			depthCamera = GetComponent<Camera>();
      depthCamera.enabled = false;
      depthCamera.depthTextureMode = DepthTextureMode.Depth;
			depthMaterial = CreateMaterial(depthShader, depthMaterial);
		}

		private void OnRenderImage(RenderTexture src, RenderTexture dst) {
      Graphics.Blit(src, dst, depthMaterial);
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
