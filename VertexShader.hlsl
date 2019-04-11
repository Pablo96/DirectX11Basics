cbuffer cbPerObject
{
	float4x4 WVP;
};

struct VS_OUTPUT
{
	float4 position: SV_POSITION;
	float3 normal : NORMAL;
};

VS_OUTPUT main(float3 inPos: POSITION, float3 inNormal : NORMAL)
{
	VS_OUTPUT output;

	output.position = mul(float4(inPos, 1), WVP);
	output.normal = inNormal;
	return output;
}