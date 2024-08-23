#include "../Common.hlsli"

Texture2D<float4> OriginalTex : register(t0);
Texture2D<float4> AddTex : register(t1);

struct VSToPS
{
    float4 position : SV_POSITION;
    float2 texcoord : TEX;
};

float4 AddTexture(VSToPS input) : SV_Target
{
    return OriginalTex.Sample(LinearWrapSampler, input.texcoord) + AddTex.Sample(LinearWrapSampler, input.texcoord);
}