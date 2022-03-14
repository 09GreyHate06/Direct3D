struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 pixelPosition : PIXEL_POSITION;
    float3 viewPosition : VIEW_POSITION;
};

struct Camera
{
    float4x4 view;
    float4x4 projection;
    float3 position;
    float p0;
};

struct Entity
{
    float4x4 transform;
    float4x4 normalMatrix;
};

cbuffer VSSystemCBuf : register(b0)
{
    Camera camera;
};

cbuffer VSEntityCBuf : register(b1)
{
    Entity entity;
}

VSOutput main(VSInput input)
{
    VSOutput vso;
    vso.position = mul(float4(input.position, 1.0f), mul(mul(entity.transform, camera.view), camera.projection));
    vso.uv = input.uv;
    vso.normal = mul(input.normal, (float3x3)entity.normalMatrix);
    vso.pixelPosition = (float3)mul(float4(input.position, 1.0f), entity.transform);
    vso.viewPosition = camera.position;
    return vso;
}