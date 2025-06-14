

struct Test
{
    float4 color;
};
struct psInput
{
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Test> gConstantBuffer : register(b0);

float4 main(psInput input) : SV_TARGET
{
    return gTexture.SampleLevel(gSampler, input.uv, 0.f);

}