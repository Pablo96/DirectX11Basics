float4 main(float2 pos: POSITION) : Sv_Position
{
	return float4(pos, 0, 1);
}