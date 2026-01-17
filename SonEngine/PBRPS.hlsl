#include "PBRCommon.hlsli"
// BOOKMARK
float4 main(psInput input) : SV_TARGET
{
    float PI = 3.141592f;
    uint width, height, numMips;
    gCubeMapSpecular.GetDimensions(0, width, height, numMips);
    uint MAX_MIP = numMips - 1;
    
    float4 modelPos = input.modelPosition;
    float2 uv = input.uv;
    float3 normalW = input.normal;
    float3 tangentW = input.tangent;
    float3 albedo = gAlbedo.Sample(gWrapLinearSampler, uv).rgb;
    
    //float roughness = gLocalCB.roughness;
    //float metallic = gLocalCB.metallic;
    float metallic = gMetallic.Sample(gWrapLinearSampler, uv).r;
    float roughness = gRoughness.Sample(gWrapLinearSampler, uv).r;
    float3 ambient = albedo.xyz * 0.1f;
    float3 N = NormalSample(tangentW, normalW, uv);
    float3 V = normalize((gPBRGCB.cameraPos - modelPos).xyz);
    float3 R = reflect(-V, N);
    float NoV = dot(N, V);
   
    if (gLocalCB.useReflect)
    {
        return gCubeMapReflect.SampleLevel(gWrapLinearSampler, R, 0.f);
    }
    
    //float shadow = 0.f;
    //if(IsShadow(modelPos, 0.0005f))
    //{
    //    shadow = 1.f;
    //}
    float3 Fdieletric = 0.04f;
    float3 F0 = lerp(Fdieletric, albedo, metallic);
    float3 ks = F0 + (1.f - F0) * pow(1.f - NoV, 5.f);
    float2 BRDF = IBL_BRDF(N, V, roughness);
    float3 specularPreFiltered = gCubeMapSpecular.SampleLevel(gWrapLinearSampler, R, roughness * MAX_MIP).rgb;
    float3 specularBRDF = F0 * BRDF.r + BRDF.g;
    float3 specularIBL = specularPreFiltered * specularBRDF;
    
    float3 F = ks;
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    float3 diffusePreFiltered = gCubeMapDiffuse.Sample(gWrapLinearSampler, N).rgb;
    float3 Cdiff = albedo;
    float3 diffuseIBL = kd * (Cdiff) * diffusePreFiltered;
    
    float3 IBLColor = specularIBL + diffuseIBL;
    return float4(IBLColor, 1.f);
    //return float4(ambient + albedo * ComputePhongDirectLight(modelPos, N, f0), 1.f);
    
    return float4(specularIBL, 1.f);
    //float3 direct = ComputePhongDirectLight(modelPos, N);
    //return float4(ambient + albedo * direct* (1.f-shadow), 1.f);
}