#include "TextCommon.hlsli"

Texture2D gText : register(t0);
SamplerState gSampler : register(s0);


float4 main(psInput input) : SV_TARGET
{
    float4 text = gText.Sample(gSampler, input.uv);
    text = pow(text, 1.0 / 2.2);
    return text;

}