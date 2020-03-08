//Texture2D tex;
//
//SamplerState splr;
//
//float4 main(float2 tc : TexCoord) : SV_Target
//{
//	return tex.Sample(splr, tc);
//}

float4 main() : SV_Target
{
	return float4(1.0f,1.0f,1.0f,1.0f);
}
