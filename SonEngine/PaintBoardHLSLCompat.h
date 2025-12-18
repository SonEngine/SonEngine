#ifndef PBHLSLCOMPAT_H
#define PBHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;
#endif

struct PBGlobalConstant
{
    float mouseX;
    float mouseY;
    float prevMouseX;
    float prevMouseY;

    int lMouseClickDown;
    XMFLOAT3 dummy;
};


#endif