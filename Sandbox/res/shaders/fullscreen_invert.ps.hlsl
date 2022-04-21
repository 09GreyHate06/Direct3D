
Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

float4 main(float2 uv : TEXCOORD) : SV_TARGET
{
    return float4(1.0f - tex.Sample(samplerState, uv).rgb, 1.0f);
}  
