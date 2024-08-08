#include "Common.hlsli"

Texture2D DiffuseRoughnessTex : register(t0);
Texture2D NormalMetallicTex : register(t1);
Texture2D EmissiveTex : register(t2);
Texture2D AoTex : register(t3);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 AmbientPS(VSToPS input) : SV_TARGET
{
    float4 albeoRoughness = DiffuseRoughnessTex.Sample(LinearWrapSampler, input.texcoord);
    float3 albedo = albeoRoughness.rgb;
    
    float4 emissiveData = EmissiveTex.Sample(LinearWrapSampler, input.texcoord);
    float emissiveFactor = emissiveData.a * 256.0;
    float3 emissive = emissiveData.rgb * emissiveFactor;
    
    float ao = 1.0;
    switch (postData.AO)
    {
        case NONE_AO:
            ao = 1.0;
            break;
        case SSAO_AO:
            ao = AoTex.Sample(LinearWrapSampler, input.texcoord).r;
            break;
    }
    
    return float4(albedo * ao, 1.0) + float4(emissive.rgb, 1.0);
}