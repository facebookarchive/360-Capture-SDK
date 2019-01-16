/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

Texture2D dtexture;
SamplerState sampleType;

float4 RGBAtoYUV444(float4 rgba) {

	float4 yuv444;

	// Y'CbCr Conversion Formula: https://en.wikipedia.org/wiki/YCbCr
	yuv444.r = 0.0f + 0.299f * rgba.r + 0.587f * rgba.g + 0.114f * rgba.b;
	yuv444.g = 0.5f - 0.169f * rgba.r - 0.331f * rgba.g + 0.500f * rgba.b;
	yuv444.b = 0.5f + 0.500f * rgba.r - 0.419f * rgba.g - 0.081f * rgba.b;
	yuv444.a = rgba.a;

	return yuv444;
}

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOOR) : SV_TARGET
{
	float4 textureColor = dtexture.Sample(sampleType, texcoord);

	return RGBAtoYUV444(textureColor);
}