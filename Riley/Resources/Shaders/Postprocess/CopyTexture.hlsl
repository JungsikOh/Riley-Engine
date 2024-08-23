#include "../Common.hlsli"

Texture2D<float4> CopyTex : register(t0);


struct VSToPS
{
    float4 position : SV_POSITION;
    float2 texcoord : TEX;
};

float4 CopyTexture(VSToPS input) : SV_Target
{
    return CopyTex.Sample(LinearWrapSampler, input.texcoord);
}