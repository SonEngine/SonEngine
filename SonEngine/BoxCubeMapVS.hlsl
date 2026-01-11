#include "BoxCubeMap.hlsli"

psInput main(vsInput input)
{
    psInput output;
    
    float4 position = float4(input.pos, 1.f);
    //output.worldPosition = input.pos;
    output.worldPosition = mul(float4(input.pos, 0.f), gLocalCB.model).xyz;
    
    position = mul(position, gLocalCB.model);
    output.modelPosition = position;
    position = mul(position, gPhongGCB.view);
    position = mul(position, gPhongGCB.proj);
    
    output.svPosition = position;
    
    output.uv = input.uv;
    
    float3 normal = mul(float4(input.normal, 0.f), gLocalCB.modelInvTranspose).xyz;
    normal = normalize(normal);
    output.normal = normal;
    
    return output;
}