#include "PointCloud.hlsli"

float4 main(PSInput input) : SV_TARGET
{
    float3 c = pow(input.color.rgb, 2.2f);
    
    return float4(c, 1);
}