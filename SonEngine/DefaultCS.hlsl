RWTexture2D<float4> gOutput : register(u0);

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 color = float3(1, 0, 1);
    gOutput[DTid.xy] = float4(color, 1.f);
}