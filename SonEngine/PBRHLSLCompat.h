#ifndef PBRCOMPAT_H
#define PBRCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;
#endif

#define NUM_LIGHTS 1

struct PBRLightInfo
{
    XMFLOAT4 brightness;
    XMVECTOR location;
    XMVECTOR direction;
};

struct PBRGlobalConstant
{
    XMMATRIX view;
    XMMATRIX proj;

    XMVECTOR cameraPos;
    XMVECTOR cameraDir;

    PBRLightInfo lights[NUM_LIGHTS];
};


#endif