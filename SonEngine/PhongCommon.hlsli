#define HLSL
#include "PhongHLSLCompat.h"

struct LocalConstant
{
    row_major matrix model;
    row_major matrix modelInvTranspose;
    int forceMip0;
};

struct vsInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct psInput
{
    float4 worldPosition : Position;
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};