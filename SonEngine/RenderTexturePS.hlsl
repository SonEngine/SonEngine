#include "RT.hlsli"

Texture2D gAlbedo : register(t0);
SamplerState gSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    return gAlbedo.SampleLevel(gSampler, input.uv, 0.f);
}