#include "PhongCommon.hlsli"

Texture2D gAlbedo : register(t0);
SamplerState gSampler : register(s0);


float4 main(psInput input) : SV_TARGET
{
    float4 albedo = gAlbedo.Sample(gSampler, input.uv);
    albedo = pow(albedo, 1.0 / 2.2);
    return albedo;

}