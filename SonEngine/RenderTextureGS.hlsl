#include "RT.hlsli"



[maxvertexcount(6)]
void main(
	point GSInput input[1], 
	inout TriangleStream< PSInput > output
)
{
    float l = 1.f;
    PSInput outputs[4] =
    {
        { float4(-l, -l, 0, 1.f), float2(0.f, 1.f) },
        { float4(-l, l, 0, 1.f), float2(0.f, 0.f) },
        { float4(l, l, 0, 1.f), float2(1.f, 0.f) },
        { float4(l, -l, 0, 1.f), float2(1.f, 1.f) }
    
    };

    PSInput p;
    
    p = outputs[0];
    output.Append(p);
    p = outputs[1];
    output.Append(p);
    p = outputs[3];
    output.Append(p);
    p = outputs[2];
    output.Append(p);
    
    //p.svPosition = float4(pos[0], 0.1);
    //output.Append(p);
    //p.svPosition = float4(pos[2], 0.1);
    //output.Append(p);
    //p.svPosition = float4(pos[3], 0.1);
    //output.Append(p);
    
    //output.RestartStrip();
}