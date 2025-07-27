struct LocalConstant
{
    row_major matrix model;
};

struct GlobalConstant
{
    row_major matrix view;
    row_major matrix proj;
};

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<GlobalConstant> gGlobalCB : register(b1);

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
    
    float4 position = float4(input.pos, 1.f);

    position = mul(position, gLocalCB.model);
    
    output.svPosition = position;
    output.uv = input.uv;
    return output;
}