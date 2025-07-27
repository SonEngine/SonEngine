struct psInput
{
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D yTex : register(t0); // R8_UNORM
Texture2D uvTex : register(t1); // R8G8_UNORM

SamplerState gSampler : register(s0);

float4 main(psInput input) : SV_TARGET
{
    float y = yTex.Sample(gSampler, input.uv).r;
    float2 uvVal = uvTex.Sample(gSampler, input.uv).rg;
    
    float u = uvVal.x - 0.5;
    float v = uvVal.y - 0.5;

    float r = y + 1.402 * v;
    float g = y - 0.344136 * u - 0.714136 * v;
    float b = y + 1.772 * u;

    return float4(r, g, b, 1.0);

}