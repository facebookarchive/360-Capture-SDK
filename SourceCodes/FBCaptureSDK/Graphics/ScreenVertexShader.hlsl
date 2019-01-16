/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

struct VOut
{
	float4 position : SV_POSITION;
	float2 texcoord: TEXCOORD;	
};

VOut main(float4 position : POSITION, float2 texcoord : TEXCOORD)
{
  VOut output;
  output.position = position;
  output.texcoord = texcoord;
  output.texcoord.y = texcoord.y;
	return output;
}
