// read from Unity depth texture, convert to RGBD inverse depth

Shader "Custom/ConvertDepth" {
	Properties{
		_MinDepth("Minimum depth", Float) = 1.0
	}

	SubShader {
		ZTest Always Cull Off ZWrite Off Fog{ Mode Off }

		Tags{ "RenderType" = "Opaque" }

		Pass {
		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#include "UnityCG.cginc"

		sampler2D _CameraDepthTexture;
		float _Scale;
		float _MinDepth;

		struct v2f {
			float4 pos : SV_POSITION;
			float2 texcoord : TEXCOORD0;
		};

		//Vertex Shader
		v2f vert(appdata_base v) {
			v2f o;
			o.pos = UnityObjectToClipPos(v.vertex);
			o.texcoord = v.texcoord;
			return o;
		}

		//Fragment Shader
		float4 frag(v2f i) : COLOR
		{
			float depth = SAMPLE_DEPTH_TEXTURE_LOD(_CameraDepthTexture, float4(i.texcoord, 0, 0));
			depth = LinearEyeDepth(depth);
			float intensity = (1.0f / depth) / (1.0f / _MinDepth);
			return intensity;
		}

		ENDCG
		}
	}
	FallBack "Diffuse"
}