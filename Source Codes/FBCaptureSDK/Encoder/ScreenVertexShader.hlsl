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
