#define HLSL
#include "PBRHLSLCompat.h"

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PBRGlobalConstant> gPBRGCB : register(b1);

Texture2D gAlbedo : register(t0);
Texture2D gAo : register(t1);
Texture2D gHeight : register(t2);
Texture2D gMetal : register(t3);
Texture2D gNormal : register(t4);
Texture2D gRoughness : register(t5);

SamplerState gWrapLinearSampler : register(s0);

struct vsInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct psInput
{
    float4 svPosition : SV_Position;
};