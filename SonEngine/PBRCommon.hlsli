#define HLSL
#include "PBRHLSLCompat.h"

struct LocalConstant
{
    row_major matrix model;
    row_major matrix modelInvTranspose;
    
    int forceMip0;
    int cubeMapMipLevel;
    int useReflect;
    float heightScale;
};

TextureCube gCubeMapDiffuse : register(t0);
TextureCube gCubeMap : register(t1);
TextureCube gCubeMapSpecular : register(t2);

TextureCube gCubeMapReflect : register(t9);

Texture2D gAlbedo : register(t3);
Texture2D gAo : register(t4);
Texture2D gHeight : register(t5);
Texture2D gMetal : register(t6);
Texture2D gNormal : register(t7);
Texture2D gRoughness : register(t8);

SamplerState gSampler : register(s0);
ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PBRGlobalConstant> gPBRGCB : register(b1);

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