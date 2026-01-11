#define HLSL
#include "PBRHLSLCompat.h"

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PBRGlobalConstant> gPhongGCB : register(b1);

struct vsInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct psInput
{
    float3 worldPosition : Position0;
    float4 modelPosition : Position1;
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};