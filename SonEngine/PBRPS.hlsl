#include "PBRCommon.hlsli"
// BOOKMARK
float4 main(psInput input) : SV_TARGET
{
    uint width, height, numMips;
    gCubeMapSpecular.GetDimensions(0, width, height, numMips);
    uint MAX_MIP = numMips - 1;
    
    float3 modelPos = input.modelPosition.xyz;
    float2 uv = input.uv;
    float3 normalW = input.normal;
    float3 tangentW = input.tangent;
    float3 albedo = gAlbedo.Sample(gWrapLinearSampler, uv).rgb;
    
    //float roughness = gLocalCB.roughness;
    //float metallic = gLocalCB.metallic;
    float metallic = gMetallic.Sample(gWrapLinearSampler, uv).g;
    float roughness = gRoughness.Sample(gWrapLinearSampler, uv).g;
    float3 ambient = albedo.xyz * 0.1f;
    
    float3 N = NormalSample(tangentW, normalW, uv);
    float3 V = normalize((gPBRGCB.cameraPos - modelPos).xyz);
    float3 R = reflect(-V, N);
    
    //return float4(albedo, 1.f);
    float NoV = saturate(dot(N, V));
    float3 F0 = lerp(Fdieletric, albedo, metallic);
    
    if (gLocalCB.useReflect)
    {
        return gCubeMapReflect.SampleLevel(gWrapLinearSampler, R, 0.f);
    }
    
    float shadow = 0.f;
    if (IsShadow(modelPos, 0.0005f))
    {
        shadow = 1.f;
    }
    float3 pointLight = ComputePBRPointLight(albedo, F0, modelPos, N, V, roughness, metallic);
    float3 phongLight = ComputePhongDirectLight(modelPos, N, F0);
    float3 IBLColor = IBL(F0, albedo, metallic, roughness, MAX_MIP, R, N, V, NoV);
    
    //float3 color = IBLColor;
    float3 color = ambient + (pointLight) * (1.f - shadow);
    //float3 color = ambient + (IBLColor + pointLight) * (1.f - shadow);
    
    return float4(color, 1.f);
    //return float4(ambient + albedo * ComputePhongDirectLight(modelPos, N, f0), 1.f);
    
    //float3 direct = ComputePhongDirectLight(modelPos, N);
    //return float4(ambient + color * (1.f - shadow), 1.f);
}