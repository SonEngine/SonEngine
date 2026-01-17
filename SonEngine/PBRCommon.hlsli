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

struct vsInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
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
    return gCubeMapBRDF.Sample(gClampLinearSampler, float2(nDotV, 1.f-roughness)).xy;
}

bool IsShadow(float4 modelPos, float bias)
{    
    float4 light_svPosition = mul(modelPos, gPBRGCB.lights[0].view);
    light_svPosition = mul(light_svPosition, gPBRGCB.lights[0].proj);
    
    float3 l_ndc = light_svPosition.xyz / light_svPosition.w;
    float2 l_uv = l_ndc.xy * 0.5f + 0.5f;
    l_uv.y = 1 - l_uv.y;
    
    bool isInUV = 
        l_uv.x < 1.f  && 
        l_uv.x >= 0.f && 
        l_uv.y < 1.f  && 
        l_uv.y >= 0.f;
    
    float currentPixelDepth = l_ndc.z;
    float shadowDepth = depthOnly.SampleLevel(gWrapLinearSampler, l_uv, 0.f).r;
    
    bool inShadow = (currentPixelDepth - shadowDepth) > bias;

    if (inShadow && isInUV)
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

float3 ComputePhongDirectLight(float4 modelPos, float3 N, float3 f)
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