#include "PBRCommon.hlsli"

float4 main(psInput input) : SV_TARGET
{
    float3 normalT = gNormal.Sample(gSampler, input.uv).xyz;
    
    normalT = 2.f * normalT - 1.f;
    
    float3 N = input.normal;
    float3 tangent = normalize(input.tangent);
    float3 T = normalize(input.tangent - dot(N, input.tangent) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N);
    float3 n = normalize(mul(TBN, normalT));
        
    float4 modelPos = input.modelPosition;
    float4 albedo = gAlbedo.Sample(gSampler, input.uv);
    //return float4(normal, 1.f);
    //return gAlbedo.Sample(gSampler, input.uv);
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        PBRLightInfo light = gPBRGCB.lights[i];
        float3 L = normalize((light.location - modelPos).xyz);
        float3 I = -L;
       
        float3 r = normalize(reflect(I, n));
        float3 v = normalize((gPBRGCB.cameraPos - modelPos).xyz);
        
        float diffuseStrength = clamp(dot(n, L), 0.f, 1.f);
        float specularStrength = pow(clamp(dot(r, v), 0.f, 1.f), 100);
        
        float3 diffuse = light.brightness.xyz * diffuseStrength;
        float3 specular = float3(1, 1, 1) * specularStrength;
        albedo.xyz *= diffuse + specular;
    }
    return albedo;
}