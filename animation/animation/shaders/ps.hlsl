Texture2D tex;
SamplerState splr;

cbuffer UVOffset
{
	float2 xy_offset;
	float2 uv_offset;
};


float4 main(float2 tc : TexCoord) : SV_Target
{
	float4 dtex = tex.Sample(splr, tc);
	clip(dtex.a < 0.1f ? -1 : 1);
	return dtex;
}
