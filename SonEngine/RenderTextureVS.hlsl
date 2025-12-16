#include "RT.hlsli"

GSInput main( VSInput input) 
{
    GSInput output;
    output.position = input.pos;
	return output;
}