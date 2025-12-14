#include "PointCloud.hlsli"

GSInput main(VSInput input)
{
    GSInput output;
    output.position = input.pos;
    output.color = input.color;
    
    return output;
}