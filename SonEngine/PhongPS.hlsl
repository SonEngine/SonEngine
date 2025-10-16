#include "PhongCommon.hlsli"

Texture2D gBricksTexture : register(t0);
SamplerState gSampler : register(s0);


float4 main(psInput input) : SV_TARGET
{
    float color = input.fog /3.f;
    
    return gBricksTexture.Sample(gSampler, input.uv);
    //return float4(1.f,1.f,1.f,1.f);
    //return float4(color, color, color, 1.0f);
}