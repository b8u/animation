cbuffer UVOffset
{
	float2 xy_offset;
	float2 uv_offset;
};

struct VSOut
{
	float2 tex : TexCoord;
	float4 pos : SV_Position;
};

VSOut main(float2 pos : Position, float2 tex : TexCoord)
{
	VSOut vsOut;
	vsOut.pos = float4(pos.x, pos.y, 0.0f, 1.0f);
	vsOut.pos += float4(xy_offset, 0.0f, 0.0f);
	vsOut.tex = tex + uv_offset;
	return vsOut;
}
