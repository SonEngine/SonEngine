#include "CubeCommon.hlsli"

ConstantBuffer<PBRGlobalConstant> gPhongGCB : register(b0);

psInput main(vsInput input )
{    
    psInput output;
    output.worldPosition = input.pos;
        
    float4 posW = mul(float4(input.pos, 0.f), gPhongGCB.view);
    posW = mul(float4(posW.xyz, 1.f), gPhongGCB.proj);
    
    output.svPosition = posW.xyzw;
    
	return output;
}