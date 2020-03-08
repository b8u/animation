//struct VSOut
//{
//	float2 tex : TexCoord;
//	float4 pos : SV_Position;
//};
//
//VSOut main(float2 pos : Position, float2 tex : TexCoord)
//{
//	VSOut vso;
//	vso.pos = pos;
//	vso.tex = tex;
//	return vso;
//}



float4 main( float2 pos : Position ) : SV_Position
{
	return float4(pos.x,pos.y,0.0f,1.0f);
}
