#include "PBRCommon.hlsli"
// BOOKMARK
float4 main(psInput input) : SV_TARGET
{
    float3 normalTex = gNormal.SampleLevel(gWrapLinearSampler, input.uv, 0.f).rgb;
    float4 albedo = gAlbedo.Sample(gWrapLinearSampler, input.uv);
    float4 modelPos = input.modelPosition;

    normalTex = normalTex * 2.f - 1.f;
    normalTex = normalize(normalTex);
    float3 tangent = normalize(input.tangent);

    float3 N = normalize(input.normal);
    float3 T = normalize(tangent - dot(tangent, N) * N);
    float3 B = normalize(cross(N, T));
    
    float3x3 TBN = float3x3(T, B, N);
    float3 n = normalize(mul(normalTex, TBN));

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