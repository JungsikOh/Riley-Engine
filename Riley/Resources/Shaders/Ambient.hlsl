#include "Common.hlsli"

Texture2D DiffuseRoughnessTex : register(t0);
Texture2D NormalMetallicTex : register(t1);
Texture2D EmissiveTex : register(t2);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 AmbientPS(VSToPS input) : SV_TARGET
{
    float4 albeoRoughness = DiffuseRoughnessTex.Sample(LinearWrapSampler, input.texcoord);
    float3 albedo = albeoRoughness.rgb;
    
    return float4(albedo, 1.0f);
}