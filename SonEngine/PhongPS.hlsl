#include "PhongCommon.hlsli"

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);


float4 main(psInput input) : SV_TARGET
{
    //float color = input.fog /3.f;
    float4 color = gTexture.Sample(gSampler, input.uv);
    //color = pow(color, 1.0 / 2.2);
    return color;
    //return float4(1.f,1.f,1.f,1.f);
    //return float4(color, color, color, 1.0f);
}