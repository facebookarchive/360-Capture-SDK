Texture2D dtexture;
SamplerState sampleType;

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 textureColor = dtexture.Sample(sampleType, texcoord);
	return textureColor;
}
