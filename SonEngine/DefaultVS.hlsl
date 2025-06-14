cbuffer Test : register(b0)
{
    float4 color;
}
struct psInput
{
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
};

struct vsInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

psInput main(vsInput input)
{
    psInput output;
    
    output.svPosition = float4(input.pos, 1.f);
    output.uv = input.uv;
    return output;
}