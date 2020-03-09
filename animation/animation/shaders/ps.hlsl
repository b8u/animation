Texture2D tex;
SamplerState splr;

float4 main(float2 tc : TexCoord) : SV_Target
{
	//return float4(1.0f,1.0f,1.0f,1.0f);
	return tex.Sample(splr, tc);
	//return float4(tc.x, tc.y, 0.0f, 1.0f);
}
