struct VS_OUTPUT
{
	float4 position: SV_POSITION;
	float3 normal : NORMAL;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
struct Light
{
	float3 dir;
	float4 ambient;
	float4 diffuse;
};

cbuffer cbPerFrame
{
	Light light;
};

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	input.normal = normalize(input.normal);

    float4 diffuse = float4(0, 1, 0, 1);
	float3 finalColor;

	finalColor = diffuse * light.ambient;
	finalColor += saturate(dot(light.dir, input.normal) * light.diffuse * diffuse);

	return float4(finalColor, diffuse.a);
}