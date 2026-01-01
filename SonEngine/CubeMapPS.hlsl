#include "CubeCommon.hlsli"

TextureCube gCubeMap : register(t0);
SamplerState gSampler : register(s0);

float4 main(psInput input) : SV_TARGET
{
    return gCubeMap.SampleLevel(gSampler, input.worldPosition, 0.f);

}