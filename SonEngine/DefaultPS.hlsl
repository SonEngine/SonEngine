struct Test
{
    float4 color;
};

ConstantBuffer<Test> gConstantBuffer : register(b0);

float4 main() : SV_TARGET
{
    return gConstantBuffer.color;
}