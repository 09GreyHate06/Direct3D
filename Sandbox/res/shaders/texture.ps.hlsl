struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer PSEntityCBuf
{
    float3 color;
    float p0;
    float2 tiling;
    float p1;
    float p2;
};

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

float4 main(VSOutput input) : SV_TARGET
{
    return tex.Sample(samplerState, input.uv * tiling) * float4(color, 1.0f);
}