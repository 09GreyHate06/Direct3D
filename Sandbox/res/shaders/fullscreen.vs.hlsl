struct VSOutput
{
    float2 uv : TEXCOORD;
    float4 position : SV_POSITION;
};

VSOutput main(float2 position : POSITION)
{
    VSOutput vso;
    vso.position = float4(position, 0.0f, 1.0f);
    vso.uv = float2((position.x + 1) / 2.0f, -(position.y - 1) / 2.0f);
    return vso;
}