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
        
    float penRadius;
    XMFLOAT3 penColor;

    int lMouseClickDown;
    XMFLOAT3 dummy;
};


#endif