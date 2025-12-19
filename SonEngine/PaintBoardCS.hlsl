#define HLSL
#include "PaintBoardHLSLCompat.h"

RWTexture2D<float4> gOutput : register(u0);
ConstantBuffer<PBGlobalConstant> gGlobalCB : register(b0);

float DistPointToSegment(float2 p, float2 a, float2 b)
{
    float2 ab = b - a;
    float ab2 = dot(ab, ab);
    float t = (ab2 > 1e-6) ? saturate(dot(p - a, ab) / ab2) : 0.0;
    float2 q = a + t * ab; 
    return length(p - q);
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint w, h;
    gOutput.GetDimensions(w, h);
    if (DTid.x >= w || DTid.y >= h)
        return;
    
    if (gGlobalCB.lMouseClickDown == 0)
        return;
    
    float2 prev = float2(gGlobalCB.prevMouseX, gGlobalCB.prevMouseY);
    float2 curr = float2(gGlobalCB.mouseX, gGlobalCB.mouseY);
        
    float2 p = float2(DTid.xy) + 0.5;
    
    const float r = 3.0;

    float d = DistPointToSegment(p, prev, curr);
    if (d <= r)
        gOutput[DTid.xy] = float4(1, 0, 0, 1);
   
 }