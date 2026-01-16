#include "PBRCommon.hlsli"
// BOOKMARK
float4 main(psInput input) : SV_TARGET
{
    float4 modelPos = input.modelPosition;
    float2 uv = input.uv;
    float3 normalW = input.normal;
    float3 tangentW = input.tangent;
    float3 albedo = gAlbedo.Sample(gWrapLinearSampler, uv);
    //float3 albedo = gAlbedo.Sample(gWrapLinearSampler, uv);
    //float roughness = gLocalCB.roughness;
    float roughness = gRoughness.Sample(gWrapLinearSampler, uv);
    float3 ambient = albedo.xyz * 0.1f;
    float3 N = NormalSample(tangentW, normalW, uv);
    float3 V = normalize((gPBRGCB.cameraPos - modelPos).xyz);
    float3 R = reflect(-V, N);
   
    if (gLocalCB.useReflect)
    {
        return gCubeMapReflect.SampleLevel(gWrapLinearSampler, R, 0.f);
    }
   
    
    //float shadow = 0.f;
    //if(IsShadow(modelPos, 0.0005f))
    //{
    //    shadow = 1.f;
    //}
    
    float3 f0 = albedo;
    float2 BRDF = IBL_BRDF(N, V, roughness);
    float3 preFiltered = gCubeMapSpecular.SampleLevel(gWrapLinearSampler, R, roughness*5.f);
    float3 specularBRDF = f0 * BRDF.r + BRDF.g;
    float3 specularIBL = preFiltered * specularBRDF;
    
    return float4(ambient + albedo * ComputePhongDirectLight(modelPos, N, f0), 1.f);
    
    return float4(specularIBL, 1.f);
    //float3 direct = ComputePhongDirectLight(modelPos, N);
    //return float4(ambient + albedo * direct* (1.f-shadow), 1.f);
}