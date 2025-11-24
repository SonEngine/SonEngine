#ifndef PHONGHLSLCOMPAT_H
#define PHONGHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;
#endif

#define NUM_LIGHTS 1

struct LightInfo
{
    XMFLOAT4 brightness;
    XMVECTOR location;
    XMVECTOR direction;
};

struct PhongGlobalConstant
{
    XMMATRIX view;
    XMMATRIX proj;

    XMVECTOR cameraPos;
    XMVECTOR cameraDir;

    LightInfo lights[NUM_LIGHTS];
};
#endif