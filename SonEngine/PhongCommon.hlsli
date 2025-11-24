#define HLSL
#include "PhongHLSLCompat.h"

struct LocalConstant
{
    row_major matrix model;
    row_major matrix modelInvTranspose;
};

//struct Light
//{
//    float4 brightness;
//    float4 directionLightPos;
//    float4 directionLightDir;
//};

//struct PhongGlobalConstant
//{
//    row_major matrix view;
//    row_major matrix proj;
    
//    float4 cameraPos;
//    float4 cameraDir;
    
//    Light lights[NUM_LIGHTS];
//};


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