struct VSOut
{
	float2 tex : TexCoord;
	float4 pos : SV_Position;
};

VSOut main(float2 pos : Position, float2 tex : TexCoord)
{
	VSOut vsOut;
	vsOut.pos = float4(pos.x,pos.y,0.0f,1.0f);
	vsOut.tex = tex;
	return vsOut;
}
