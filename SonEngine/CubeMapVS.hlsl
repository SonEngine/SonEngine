#include "CubeCommon.hlsli"

ConstantBuffer<PhongGlobalConstant> gPhongGCB : register(b0);

psInput main(vsInput input )
{    
    psInput output;
    output.worldPosition = input.pos;
        
    float4 posW = float4(input.pos, 1.f);
    posW.xyz += gPhongGCB.cameraPos.xyz;
    
    posW = mul(posW, gPhongGCB.view);
    posW = mul(posW, gPhongGCB.proj);
    
    output.svPosition = posW.xyzw;
    
	return output;
}