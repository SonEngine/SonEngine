#include "DepthOnlyCommon.hlsli"

ConstantBuffer<SkinnedLocalConstant> gBoneTransformCB : register(b2);

psInput main(skinnedVsInput input)
{
    psInput output;
    
    float2 uv = mul(float4(input.uv, 0, 1), gLocalCB.texTransform).xy;
    float height = gHeight.SampleLevel(gWrapLinearSampler, uv, 0.f).x;
    height = height * 2.0 - 1.0;
    height *= gLocalCB.heightScale;
    
    uint indices[8] =
    {
        input.blendIndices0.x,
        input.blendIndices0.y,
        input.blendIndices0.z,
        input.blendIndices0.w,
        input.blendIndices1.x,
        input.blendIndices1.y,
        input.blendIndices1.z,
        input.blendIndices1.w
    };
    float weights[8] =
    {
        input.blendWeight0.x,
        input.blendWeight0.y,
        input.blendWeight0.z,
        input.blendWeight0.w,
        input.blendWeight1.x,
        input.blendWeight1.y,
        input.blendWeight1.z,
        input.blendWeight1.w
    };
    
    float3 posL = 0.f;
    for (uint i = 0; i < 8; i++)
    {
        posL += weights[i] * mul(float4(input.pos, 1.f), gBoneTransformCB.boneTransform[indices[i]]).xyz;
    }
    float4 posH = float4(posL.xyz + input.normal * height, 1.f);
    float4 posW = mul(posH, gLocalCB.model);
   
    float4 posSV = mul(posW, gPBRGCB.view);
    posSV = mul(posSV, gPBRGCB.proj);
    
    output.svPosition = posSV;

    
    return output;
}