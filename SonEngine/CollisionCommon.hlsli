#define HLSL
#include "PBRHLSLCompat.h"

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PBRGlobalConstant> gPBRGCB : register(b1);


struct vsInput
{
    float3 pos : POSITION;
};

struct gsInput
{
    float3 pos : POSITION;
};

struct psInput
{
    float4 svPosition : SV_Position;
};