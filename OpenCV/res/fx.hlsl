Texture2D t0;
SamplerState s0;


cbuffer cb0 : register(c0)
{
	row_major float4x4 Proj;
	row_major float4x4 View;
};

cbuffer cb1 : register(c1) {
	row_major float4x4 World;
	row_major float4x4 Mat;
	float4 Diffuse;
	float4x3 _reserved_;
};

struct VS_OUTPUT
{
	float4 Position   : SV_POSITION;
	float2 TexCoord : TEX_COORD;
	float4 Diffuse : DIFFUSE;
};


VS_OUTPUT VS(in float2 Pos : POSITION)
{
	VS_OUTPUT Output;
	Output.Position =
		mul(
			mul(
				mul(float4(Pos, 0.f, 1.f),
					World),
				View),
			Proj);
	Output.TexCoord = mul(float4(Pos, 0.f, 1.f), Mat).xy;
	Output.Diffuse = Diffuse;
	return Output;
}

float4 PS(VS_OUTPUT vs) : SV_TARGET{
	float4 color = t0.Sample(s0, vs.TexCoord);

	color.r = pow(color.r, 2.25f);
	color.g = pow(color.g, 2.25f);
	color.b = pow(color.b, 2.25f);

	return color * vs.Diffuse;
}


