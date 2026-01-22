#include "PBRCommon.hlsli"

psInput main(vsInput input) 
{
    psInput output;
    
    float2 uv = mul(float4(input.uv, 0, 1), gLocalCB.texTransform).xy;
    output.uv = uv;
    
    float height = gHeight.SampleLevel(gWrapLinearSampler, uv, 0.f).x;
    height = height * 2.0 - 1.0;
    height *= gLocalCB.heightScale;
    
    float4 position = float4(input.pos + input.normal * height, 1.f);
    position = mul(position, gLocalCB.model);
       
    output.modelPosition = position;
    position = mul(position, gPBRGCB.view);
    position = mul(position, gPBRGCB.proj);
    
    output.svPosition = position;
        
    float3 normal = mul(float4(input.normal, 0.f), gLocalCB.modelInvTranspose).xyz;
    //float3 normal = mul(float4(input.normal, 0.f), gLocalCB.model).xyz;
    float3 tangent = mul(float4(input.tangent, 0.f), gLocalCB.model).xyz;
    
    output.normal = normalize(normal);
    output.tangent = normalize(tangent);
    
    return output;
}