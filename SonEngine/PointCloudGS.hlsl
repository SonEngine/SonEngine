#include "PointCloud.hlsli"

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<GlobalConstant> gGlobalCB : register(b1);

[maxvertexcount(3)]
void main(
	point GSInput input[1],
	inout TriangleStream<PSInput> output
)
{
    float3 del[3] =
    {
		float3(0, 0.1, 0),
		float3(0.1, -0.1, 0),
        float3(-0.1, -0.1, 0)
    };
	for (uint i = 0; i < 3; i++)
	{
        PSInput p;
        float4 pos = float4(input[0].position + del[i], 1.f);
        //pos = mul(pos, gLocalCB.model);
        pos = mul(pos, gGlobalCB.view);
        pos = mul(pos, gGlobalCB.proj);
        p.svPosition = pos;
        p.color = input[0].color;
        
		output.Append(p);
	}
    output.RestartStrip();
}