#include "PBRCommon.hlsli"
// BOOKMARK
float4 main(psInput input) : SV_TARGET
{
    float3 normalTex = gNormal.Sample(gWrapLinearSampler, input.uv).rgb;
    float3 albedo = gAlbedo.Sample(gWrapLinearSampler, input.uv).xyz;
    float4 modelPos = input.modelPosition;

    normalTex = normalTex * 2.f - 1.f;
    normalTex = normalize(normalTex);
    
    if (gLocalCB.useReflect)
    {
        float3 v = normalize((modelPos - gPBRGCB.cameraPos).xyz);
        float3 r = normalize(reflect(v, input.normal));
        return gCubeMapReflect.SampleLevel(gWrapLinearSampler, r, 0.f);
    }
   
    float3 tangent = normalize(input.tangent);
    
    float4 l = mul(modelPos, gPBRGCB.lights[0].view);
    l = mul(l, gPBRGCB.lights[0].proj);
    float3 ndc = l.xyz / l.w;
    float2 lUV = ndc.xy * 0.5f + 0.5f;
    lUV.y = 1 - lUV.y;
    
    bool isInUV = lUV.x < 1.f && lUV.x >= 0.f && lUV.y < 1.f && lUV.y >= 0.f;
        
    float currentPixelDepth = ndc.z;
    float shadowDepth = depthOnly.SampleLevel(gWrapLinearSampler, lUV, 0.f).r;
    float bias = 0.0005f; 
    bool inShadow = (currentPixelDepth - shadowDepth) > bias;

    float3 ambient = albedo.xyz * 0.1f;
    float shadow = 0.f;
    if (inShadow && isInUV)
    {
        shadow = 1.f;
    }
    
    float3 N = normalize(input.normal);
    float3 T = normalize(tangent - dot(tangent, N) * N);
    float3 B = normalize(cross(N, T));
    
    float3x3 TBN = float3x3(T, B, N);
    float3 n = normalize(mul(normalTex, TBN));
    float3 direct = 0.f;
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
        direct += diffuse + specular;
    }
    return float4(ambient + albedo * direct* (1.f-shadow), 1.f);
}