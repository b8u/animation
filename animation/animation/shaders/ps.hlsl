Texture2D tex;
SamplerState splr;

float4 main(float2 tc : TexCoord) : SV_Target
{
	float4 dtex = tex.Sample(splr, tc);
	clip(dtex.a < 0.1f ? -1 : 1);
	//return float4(1.0f,1.0f,1.0f,1.0f);
	return dtex;
	//return float4(tc.x, tc.y, 0.0f, 1.0f);
}
