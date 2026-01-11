#include "PBRCommon.hlsli"

psInput main(vsInput input) 
{
    psInput output;
    
    float4 position = float4(input.pos, 1.f);
    output.worldPosition = mul(float4(input.pos, 0.f), gLocalCB.model).xyz;
    
    position = mul(position, gLocalCB.model);
    output.modelPosition = position;
    position = mul(position, gPBRGCB.view);
    position = mul(position, gPBRGCB.proj);
    
    output.svPosition = position;
    
    output.uv = input.uv;
    
    float3 normal = mul(float4(input.normal, 0.f), gLocalCB.modelInvTranspose).xyz;
    float3 tangent = mul(float4(input.tangent, 0.f), gLocalCB.modelInvTranspose).xyz;
    
    output.normal = normalize(normal);
    output.tangent = tangent;
    
    return output;
}