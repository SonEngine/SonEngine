#include "CollisionCommon.hlsli"


struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(100)]
void main(
	point gsInput input[1], 
	inout LineStream< psInput > output
)
{
    float halfX = gLocalCB.collisionScale.x;
    float halfY = gLocalCB.collisionScale.y;
    float halfZ = gLocalCB.collisionScale.z;
    //float halfX = 1.f;
    //float halfY = 1.f;
    //float halfZ = 1.f;
    
    float3 pos[8] =
    {
        float3(-halfX, halfY, -halfZ), // 왼 앞 위
		float3(halfX, halfY, -halfZ),  // 우 앞
		float3(halfX, halfY, halfZ),   // 우 뒤
		float3(-halfX, halfY, halfZ),  // 좌 뒤
		float3(-halfX, -halfY, -halfZ),// 왼 앞
		float3(halfX, -halfY, -halfZ), // 우 앞
		float3(halfX, -halfY, halfZ),  // 우 뒤
		float3(-halfX, -halfY, halfZ), // 좌 뒤
    };
    uint indices[10] =
    {
        0, 1, 2, 3, 0,
        4, 5, 6, 7, 4
    };
    uint indices2[8] =
    {
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };
    for (int i = 0; i < 10; i++)
    {
        int index = indices[i];
        psInput o;
        float4 svPos = mul(float4(pos[index], 1.f), gLocalCB.model);
        svPos = mul(svPos, gPBRGCB.view);
        svPos = mul(svPos, gPBRGCB.proj);
        o.svPosition = svPos;
        output.Append(o);
        if ((i + 1) % 5 == 0)
        {
            output.RestartStrip();
        }
    }
    for (int j = 0; j < 8; j++)
    {
        int index = indices2[j];
        psInput o;
        float4 svPos = mul(float4(pos[index], 1.f), gLocalCB.model);
        svPos = mul(svPos, gPBRGCB.view);
        svPos = mul(svPos, gPBRGCB.proj);
        o.svPosition = svPos;
        output.Append(o);
        if ((j + 1) % 2 == 0)
        {
            output.RestartStrip();
        }
    }
}