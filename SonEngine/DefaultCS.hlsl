RWTexture2D<float4> gOutput : register(u0);

float2 c_mul(float2 a, float2 b)
{
    return float2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}
float magnitude2(float2 a)
{
    return a.x * a.x + a.y * a.y;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint w, h;
    gOutput.GetDimensions(w, h);
    float jx = 1.5f * (((float) w / 2 - DTid.x) / ((float) w / 2));
    float jy = 1.5f * (((float) h / 2 - DTid.y) / ((float) h / 2));

    float2 c = float2(-0.8, 0.156);
    float2 a = float2(jx, jy);
    
    
    for (int i = 0; i < 200; i++)
    {
        a = c_mul(a, a) + c;
        
        if (magnitude2(a) > 1000)
        {
            gOutput[DTid.xy] = float4(0, 0, 0, 1);
            return;
        }
    }
    gOutput[DTid.xy] = float4(1, 1, 1, 1);
}