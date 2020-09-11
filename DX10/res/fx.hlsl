SamplerState s0;
Texture2D t0;

cbuffer cb0 : register(c0) {
	row_major float4x4 World;
	row_major float4x4 View;
	row_major float4x4 Proj;
};

cbuffer cb1 : register(c1) {
	float4 Diffuse;
}

struct VS_OUTPUT
{
    float4 pos  : SV_POSITION;
	float2 tex : TEX_COORD;
};


VS_OUTPUT VS( in float2 vPosition : POSITION )
{
	VS_OUTPUT Output;

    Output.pos = mul(mul(mul( float4( vPosition.x, vPosition.y, 0.f, 1.f), World), View), Proj);
	Output.tex = vPosition;

    return Output;
}


float4 PS(VS_OUTPUT input) : SV_Target
{
	return pow(t0.Sample(s0, input.tex) * Diffuse, 2.25f);
}
