#include "PhongCommon.hlsli"

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PhongGlobalConstant> gPhongGCB : register(b1);


psInput main(vsInput input)
{
    psInput output;
    
    float4 position = float4(input.pos, 1.f);
    position = mul(position, gLocalCB.model);
    output.worldPosition = position;
    position = mul(position, gPhongGCB.view);
    position = mul(position, gPhongGCB.proj);
    
    output.svPosition = position;
    
    output.uv = input.uv;
    
    float3 normal = mul(float4(input.normal, 0.f), gLocalCB.modelInvTranspose).xyz;
    normal = normalize(normal);
    output.normal = normal;
    
    return output;
}