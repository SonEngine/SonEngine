#ifndef PBRHLSLCOMPAT_H
#define PBRHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX::SimpleMath;
using namespace DirectX;
#endif

#define NUM_LIGHTS 1
// BOOKMARK
struct LocalConstant
{
    Matrix model;
    Matrix modelInvTranspose;
    Matrix texTransform;

    int forceMip0;
    int cubeMapMipLevel;
    int useReflect;
    float heightScale;
    float roughness;
    float metallic;
    float dummy[26];
};


struct PBRLightInfo
{
    XMFLOAT4 brightness;
    XMVECTOR location;
    XMVECTOR direction;

    Matrix view;
    Matrix proj;
};

struct PBRGlobalConstant
{
    Matrix view;
    Matrix proj;

    XMVECTOR cameraPos;
    XMVECTOR cameraDir;

    PBRLightInfo lights[NUM_LIGHTS];
};


#endif