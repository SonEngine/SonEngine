#define HLSL
#include "PhongHLSLCompat.h"

struct LocalConstant
{
    row_major matrix model;
    row_major matrix modelInvTranspose;
    
};

struct VSInput
{
    float3 pos : POSITION;
};

struct GSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
};