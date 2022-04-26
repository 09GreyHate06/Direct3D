
Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

static const int s_r = 7;
static const float s_divisor = (2 * s_r + 1) * (2 * s_r + 1);

float4 main(float2 uv : TEXCOORD) : SV_TARGET
{
    uint width, height;
    tex.GetDimensions(width, height);

    const float dx = 1.0f / width;
    const float dy = 1.0f / height;
    float accAlpha = 0.0f;
    float3 acc = float3(0.0f, 0.0f, 0.0f);
    for (int y = -s_r; y <= s_r; y++)
    {
        for (int x = -s_r; x <= s_r; x++)
        {
            const float2 tc = uv + float2(dx * x, dy * y);
            const float4 color = tex.Sample(samplerState, tc).rgba;
            
            accAlpha += color.a;
            acc = max(color.rgb, acc);
        }
    }

    return float4(acc, accAlpha / s_divisor);
}