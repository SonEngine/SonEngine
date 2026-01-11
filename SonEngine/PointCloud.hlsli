#define HLSL
#include "PBRHLSLCompat.h"


struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct GSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 svPosition : SV_Position;
    float4 color : COLOR;
};