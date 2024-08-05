#include "Common.hlsli"

Texture2D AoTex : register(t0);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 SSAOBlur(VSToPS input) : SV_TARGET
{
    uint width = 1;
    uint height = 1;
    uint mipLevels = 0;
    AoTex.GetDimensions(0, width, height, mipLevels);
    
    float2 texelSize = 1.0 / float2(width, height);
    float result = 0.0;
    
    [unroll]
    for (int x = -4; x < 4; ++x)
    {
        [unroll]
        for (int y = -4; y < 4; ++y)
        {
            float2 offset = float2(float(x), float(y)) * texelSize;
            result += AoTex.Sample(LinearBorderSampler, input.texcoord + offset).r;
        }

    }
    
    float output = result / (8.0 * 8.0);
    return float4(output, output, output, 1.0);
}