#include "PBRCommon.hlsli"

psInput main(vsInput input) 
{
    psInput output;
    float2 uv = input.uv * 2.f;
    float height = gHeight.SampleLevel(gSampler, uv, 0.f).x;
    height = height * 2.0 - 1.0;
    
    height *= gLocalCB.heightScale;
        
    float3 posW = mul(float4(input.pos, 0.f), gLocalCB.model).xyz;
    float3 posH = posW + input.normal * height;
    output.worldPosition = posH;
    
    float4 position = mul(float4(input.pos, 1.f), gLocalCB.model);
    position.xyz += +input.normal * height;
    
    output.modelPosition = position;
    position = mul(position, gPBRGCB.view);
    position = mul(position, gPBRGCB.proj);
    
    output.svPosition = position;
    
    output.uv = uv;
    
    float3 normal = mul(float4(input.normal, 0.f), gLocalCB.modelInvTranspose).xyz;
    float3 tangent = mul(float4(input.tangent, 0.f), gLocalCB.model).xyz;
    
    output.normal = normalize(normal);
    output.tangent = tangent;
    
    return output;
}