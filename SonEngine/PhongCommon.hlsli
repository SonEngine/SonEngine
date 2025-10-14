struct LocalConstant
{
    row_major matrix model;
};

struct PhongGlobalConstant
{
    row_major matrix view;
    row_major matrix proj;
    
    float4 cameraPos;
    float4 cameraDir;
 
    float4 DirectionLightPos;
    float4 DirectionLightDir;
};

struct psInput
{
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
    float fog : FOG;
};

struct vsInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};
