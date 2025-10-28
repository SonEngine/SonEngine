#include "PhongCommon.hlsli"

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PhongGlobalConstant> gPhongGCB : register(b1);


psInput main(vsInput input)
{
    psInput output;
    
    float4 position = float4(input.pos, 1.f);
    position = mul(position, gLocalCB.model);
    position = mul(position, gPhongGCB.view);
    position = mul(position, gPhongGCB.proj);
    
    output.svPosition = position;
    
    output.uv = input.uv;
    
    output.normal = input.normal;
    return output;
}