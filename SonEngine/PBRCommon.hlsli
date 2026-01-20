#define HLSL
#include "PBRHLSLCompat.h"

Texture2D gCubeMapBRDF : register(t0);
TextureCube gCubeMapDiffuse : register(t1);
TextureCube gCubeMap : register(t2);
TextureCube gCubeMapSpecular : register(t3);

Texture2D gAlbedo : register(t4);
Texture2D gAo : register(t5);
Texture2D gHeight : register(t6);
Texture2D gMetallic : register(t7);
Texture2D gNormal : register(t8);
Texture2D gRoughness : register(t9);

TextureCube gCubeMapReflect : register(t10);

Texture2D<float> depthOnly : register(t11);


SamplerState gWrapLinearSampler : register(s0);
SamplerState gClampLinearSampler : register(s1);

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PBRGlobalConstant> gPBRGCB : register(b1);

static const float3 Fdieletric = 0.04f;
static const float PI = 3.141592f;

float2 IBL_BRDF(float3 n, float3 v, float roughness);

float SchlickGGX(float3 n, float3 v, float k);

float Geometry(float roughness, float3 l, float3 v, float3 h, float3 nn);
float NormalDistribution(float alpha2, float NoH);
float3 SchlickFresnel(float3 F0, float3 LoH);

struct vsInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct skinnedVsInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
    float4 blendWeight0 : BLENDWEIGHT0;
    float4 blendWeight1 : BLENDWEIGHT1;
    uint4 blendIndices0 : BLENDINDICES0;
    uint4 blendIndices1 : BLENDINDICES1;
};

struct psInput
{
    float3 worldPosition : Position0;
    float4 modelPosition : Position1;
    float4 svPosition : SV_Position;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

float2 IBL_BRDF(float3 n, float3 v, float roughness)
{
    float nDotV = saturate(dot(n, v));
    return gCubeMapBRDF.Sample(gClampLinearSampler, float2(nDotV, 1.f - roughness)).xy;
}

bool IsShadow(float3 modelPos, float bias)
{
    float4 light_svPosition = mul(float4(modelPos, 1.f), gPBRGCB.lights[0].view);
    light_svPosition = mul(light_svPosition, gPBRGCB.lights[0].proj);
    
    float3 l_ndc = light_svPosition.xyz / light_svPosition.w;
    float2 l_uv = l_ndc.xy * 0.5f + 0.5f;
    l_uv.y = 1 - l_uv.y;
    
    bool isInUV =
        l_uv.x < 1.f &&
        l_uv.x >= 0.f &&
        l_uv.y < 1.f &&
        l_uv.y >= 0.f;
    
    float currentPixelDepth = l_ndc.z;
    float shadowDepth = depthOnly.SampleLevel(gClampLinearSampler, l_uv, 0.f).r;
    
    bool inShadow = (currentPixelDepth - shadowDepth) > bias;

    if (isInUV && inShadow)
    {
        return true;
    }
    else
        return false;
}

float3 NormalSample(float3 tangent, float3 normalW, float2 uv)
{
    float3 normalTex = gNormal.Sample(gWrapLinearSampler, uv).rgb;
    normalTex = normalTex * 2.f - 1.f;
    normalTex = normalize(normalTex);
    
    float3 albedo = gAlbedo.Sample(gWrapLinearSampler, uv).xyz;
    
    float3 N = normalW;
    float3 T = normalize(tangent - dot(tangent, N) * N);
    float3 B = normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);
    float3 n = normalize(mul(normalTex, TBN));
    
    return n;
}

float3 ComputePhongDirectLight(float3 modelPos, float3 N, float3 f)
{
    float3 direct = 0.f;
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        PBRLightInfo light = gPBRGCB.lights[i];
        float3 L = normalize((light.location - modelPos).xyz);
        float3 I = -L;
       
        float3 r = normalize(reflect(I, N));
        float3 v = normalize((gPBRGCB.cameraPos - modelPos).xyz);
        
        float diffuseStrength = clamp(dot(N, L), 0.f, 1.f);
        float specularStrength = pow(clamp(dot(r, v), 0.f, 1.f), 100);
        float3 diffuse = light.brightness.xyz * diffuseStrength;
        float3 specular = specularStrength;
        direct += diffuse + specular;
    }
    return direct;
}

float3 ComputePBRPointLight(float3 albedo, float3 F0, float3 p, float3 n, float3 v, float roughness, float metallic)
{
    float3 pointLight = 0.f;
    float NoV = saturate(dot(n, v));
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        PBRLightInfo light = gPBRGCB.lights[i];
        float3 l = normalize(light.location - p);
        float dist = length(light.location - p);
        float3 h = normalize(l + v);
        float NoH = saturate(dot(n, h));
        float NoL = saturate(dot(n, l));
        float LoH = saturate(dot(l, h));
        
        float D = NormalDistribution(alpha2, NoH);
        float G = Geometry(roughness, l, v, h, n);
        float3 F = SchlickFresnel(F0, LoH);
        float3 ks = F;
        
        float3 kd = lerp(1 - ks, 0.f, metallic);
        float3 diffuseBRDF = kd * albedo / PI;
        
        float3 specularBRDF = (D * F * G) / (1e-5 + 4.f * NoL * NoV);
       
        float attenuation = (1.f / (dist * dist));
        
        float3 Li = light.color * light.intensity * attenuation;
        float3 BRDF = specularBRDF + diffuseBRDF;
        pointLight += BRDF * Li * NoL;
    }
    return pointLight;
}

float NormalDistribution(float alpha2, float NoH)
{
    float NoH2 = NoH * NoH;
    float div = PI * pow(NoH2 * (alpha2 - 1.f) + 1.f , 2.f);
    return alpha2 / div;
}

float Geometry(float roughness, float3 l, float3 v, float3 h, float3 n)
{
    float alpha = (roughness + 1.f);
    float alpha2 = alpha * alpha;
    float k = alpha2 / 8.f;
    float G = SchlickGGX(n, v, k) * SchlickGGX(n, l, k);

    return G;
}

float SchlickGGX(float3 n, float3 v, float k)
{
    float NoV = saturate(dot(n, v));
    return NoV / (NoV * (1 - k) + k);
}

float3 SchlickFresnel(float3 F0, float3 LoH)
{
    return F0 + (1.f - F0) * pow(1.f - LoH, 5.f);
}

float3 SpecularIBL(float3 F0, float3 albedo, float3 N, float3 V, float3 R, float metallic, float roughness, float MAX_MIP)
{
    float2 BRDF = IBL_BRDF(N, V, roughness);
    float3 specularPreFiltered = gCubeMapSpecular.SampleLevel(gWrapLinearSampler, R, roughness * MAX_MIP).rgb;
    float3 specularBRDF = F0 * BRDF.r + BRDF.g;
    float3 specularIBL = specularPreFiltered * specularBRDF;
    
    return specularIBL;
}

float3 DiffuseIBL(float3 F0, float3 albedo, float3 N, float3 V, float NoV, float metallic)
{
    float3 F = SchlickFresnel(F0, NoV);
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    float3 diffusePreFiltered = gCubeMapDiffuse.Sample(gWrapLinearSampler, N).rgb;
    float3 Cdiff = albedo;
    float3 diffuseIBL = kd * (Cdiff) * diffusePreFiltered;
    
    return diffuseIBL;
}

float3 IBL(float3 F0, float3 albedo, float metallic, float roughness, float MAX_MIP, float3 R, float3 N, float3 V, float NoV)
{
    float3 specular = SpecularIBL(F0, albedo, N, V, R, metallic, roughness, MAX_MIP);
    float3 diffuse = DiffuseIBL(F0, albedo, N, V, NoV, metallic);
    
    return diffuse + specular;
}