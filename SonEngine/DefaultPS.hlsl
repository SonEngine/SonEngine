


struct psInput
{
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);


float4 GetCheckColor(float2 uv)
{
    float4 w = float4(1, 1, 1, 1);
    float4 b = float4(0, 0, 0, 1);
    if (uv.x >= 1)
    {
        if (uv.y >= 1)
            return w;
        else
            return b;
    }
    else
    {
        if (uv.y >= 1)
            return b;
        else
            return w;
    }
}

float4 MakeCheckPattern(int c, float2 inputUV)
{
    float2 uv = inputUV * c;
    
    if (uv.x > 2)
    {
        int x = floor(uv.x);
        if(x%2==0)
            uv.x -= x;
        else
            uv.x -= (x - 1);
    }
    if (uv.y > 2)
    {
        int y = floor(uv.y);
        if (y % 2 == 0)
            uv.y -= y;
        else
            uv.y -= (y - 1);
    }
    
    return GetCheckColor(uv);
}

float4 main(psInput input) : SV_TARGET
{
    return MakeCheckPattern(7, input.uv);
}