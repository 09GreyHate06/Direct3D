struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer VSSystemCBuf
{
    float4x4 view;
    float4x4 projection;
};

cbuffer VSEntityCBuf
{
    float4x4 transform;
};

VSOutput main(VSInput input)
{
    VSOutput vso;
    vso.position = mul(float4(input.position, 1.0f), mul(mul(transform, view), projection));
    vso.uv = input.uv;
	return vso;
}