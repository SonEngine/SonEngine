#include "RT.hlsli"

Texture2D gAlbedo : register(t0);
SamplerState gSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    float4 t = gAlbedo.SampleLevel(gSampler, input.uv, 0.f);
    t.xyz = pow(t.xyz, 1 / 2.2f);
    return t;
}