#include "PointCloud.hlsli"

ConstantBuffer<LocalConstant> gLocalCB : register(b0);
ConstantBuffer<PBRGlobalConstant> gGlobalCB : register(b1);

float4 GetSVPosition(float3 pos)
{
    float4 newPos = float4(pos, 1.f);
    newPos = mul(newPos, gLocalCB.model);
    newPos = mul(newPos, gGlobalCB.view);
    newPos = mul(newPos, gGlobalCB.proj);
    
    return newPos;
}

[maxvertexcount(36)]
void main(
	point GSInput input[1],
	inout TriangleStream<PSInput> output
)
{
    float x = 0.03f;
    float y = 0.03f;
    float z = 0.03f;
    
    float halfX = x / 2.f;
    float halfY = y / 2.f;
    float halfZ = z / 2.f;
    
    float3 del[8] =
    {
        float3(-halfX, -halfY, -halfZ),
		float3(-halfX, halfY, -halfZ),
		float3(halfX, halfY, -halfZ),
		float3(halfX, -halfY, -halfZ),
		float3(-halfX, -halfY, halfZ),
		float3(-halfX, halfY, halfZ),
		float3(halfX, halfY, halfZ),
		float3(halfX, -halfY, halfZ)
	};
     
    int4 idxList[6] =
    {
        int4(0, 1, 2, 3),
        int4(3, 2, 6, 7),
        int4(4, 5, 1, 0),
        int4(7, 6, 5, 4),
        int4(4, 0, 3, 7),
        int4(1, 5, 6, 2)
    };
    
    for (uint i = 0; i < 6; i++)
    {
        int4 idx = idxList[i];
        
        PSInput p;
        p.color = input[0].color;
        
        float3 basePos = input[0].position;
        p.svPosition = GetSVPosition(basePos + del[idx[0]]);
        output.Append(p);
        p.svPosition = GetSVPosition(basePos + del[idx[1]]);
        output.Append(p);
        p.svPosition = GetSVPosition(basePos + del[idx[2]]);
        output.Append(p);
        output.RestartStrip();
        
        p.svPosition = GetSVPosition(basePos + del[idx[0]]);
        output.Append(p);
        p.svPosition = GetSVPosition(basePos + del[idx[2]]);
        output.Append(p);
        p.svPosition = GetSVPosition(basePos + del[idx[3]]);
        output.Append(p);
        output.RestartStrip();
    }    
}