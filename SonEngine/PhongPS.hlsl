#include "PhongCommon.hlsli"

TextureCube gCubeMap : register(t0);
Texture2D gAlbedo : register(t1);

SamplerState gSampler : register(s0);
ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PhongGlobalConstant> gPhongGCB : register(b1);


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
    
    float4 cubeMap = cubeMapExposure * gCubeMap.SampleLevel(gSampler, input.worldPosition, gLocalCB.cubeMapMipLevel);
    //float4 albedo = gAlbedo.Sample(gSampler, input.uv);
    
    return cubeMap;
    float4 modelPos = input.modelPosition;
    
    // albedo = pow(albedo, 1.0 / 2.2);
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        LightInfo light = gPhongGCB.lights[i];
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