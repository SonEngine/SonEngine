#ifndef PBRHLSLCOMPAT_H
#define PBRHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX::SimpleMath;
using namespace DirectX;
#endif

#define NUM_LIGHTS 1

struct LocalConstant
{
    Matrix model;
    Matrix modelInvTranspose;

    int forceMip0;
    int cubeMapMipLevel;
    int useReflect;
    float heightScale;

    float dummy[28];
};


struct PBRLightInfo
{
    XMFLOAT4 brightness;
    XMVECTOR location;
    XMVECTOR direction;
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