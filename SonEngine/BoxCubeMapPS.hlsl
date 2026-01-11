#include "PhongCommon.hlsli"

TextureCube gCubeMapDiffuse : register(t0);
TextureCube gCubeMap : register(t1);
TextureCube gCubeMapSpecular : register(t2);

Texture2D gAlbedo : register(t3);
Texture2D gAo : register(t4);
Texture2D gHeight : register(t5);
Texture2D gMetal : register(t6);
Texture2D gNormal : register(t7);
Texture2D gRoughness : register(t8);

SamplerState gSampler : register(s0);
SamplerState gClampSampler : register(s1);


float4 main(psInput input) : SV_TARGET
{
    float cubeMapExposure = 3.f;
    uint w, h, mipCount;
    gAlbedo.GetDimensions(0, w, h, mipCount);
    float4 albedo;
    if (gLocalCB.forceMip0 == 1)
    {
        albedo = gAlbedo.SampleLevel(gSampler, input.uv, 0.f);
    }
    else
    {
        albedo = gAlbedo.Sample(gSampler, input.uv);
    }
   
    float3 cubeMap = cubeMapExposure * gCubeMap.SampleLevel(gSampler, input.worldPosition, gLocalCB.cubeMapMipLevel).xyz;
    float3 diffuse = cubeMapExposure * gCubeMapDiffuse.SampleLevel(gSampler, input.worldPosition, gLocalCB.cubeMapMipLevel).xyz;
    //float3 specluar = cubeMapExposure * gCubeMapSpecular.SampleLevel(gSampler, input.worldPosition, gLocalCB.cubeMapMipLevel).xyz;
    
    float3 normal = gNormal.Sample(gSampler, input.uv).xyz;  
    float4 modelPos = input.modelPosition;
    return albedo;
    
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        PBRLightInfo light = gPhongGCB.lights[i];
        //float3 L = normalize(float3(-1, 1, -1));
        float3 L = normalize((light.location - modelPos).xyz);
        float3 I = -L;
        float3 n = input.normal;
        float3 r = normalize(reflect(I, input.normal));
        float3 v = normalize((gPhongGCB.cameraPos - modelPos).xyz);
        
        float diffuseStrength = clamp(dot(n, L), 0.f, 1.f);
        float specularStrength = pow(clamp(dot(r, v), 0.f, 1.f), 100);
        
        float3 diffuse = light.brightness.xyz * diffuseStrength;
        float3 specular = float3(1, 1, 1) * specularStrength;
        albedo.xyz *= diffuse + specular;
    }
    return albedo;
}