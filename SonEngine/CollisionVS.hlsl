#include "CollisionCommon.hlsli"

gsInput main(vsInput input) 
{
    gsInput output;
    output.pos = input.pos;
    return output;
}