#include "DepthOnlyCommon.hlsli"

psInput main(vsInput input)
{
    psInput output;
    
    float2 uv = mul(float4(input.uv, 0, 1), gLocalCB.texTransform).xy; 
    float height = gHeight.SampleLevel(gWrapLinearSampler, uv, 0.f).x;
    height = height * 2.0 - 1.0;
    height *= gLocalCB.heightScale;
    
    float4 position = float4(input.pos + input.normal * height, 1.f);
    position = mul(position, gLocalCB.model);
       

    position = mul(position, gPBRGCB.view);
    position = mul(position, gPBRGCB.proj);
    
    output.svPosition = position;
    
    return output;
}