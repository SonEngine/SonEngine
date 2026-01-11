#define HLSL
#include "PBRHLSLCompat.h"

struct vsInput
{
    float3 pos : POSITION;
};

struct psInput
{
    float3 worldPosition : Position;
    float4 svPosition : SV_Position;    
};