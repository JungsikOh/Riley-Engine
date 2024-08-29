#include "../Common.hlsli"

Texture2D SunTex : register(t0);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 GodsRay(VSToPS input) : SV_TARGET
{
    static const int NUM_SAMPLES = 64;
    float2 texcoord = input.texcoord;
    
    float2 deltaTexcoord = (texcoord - lightData.screenSpacePosition.xy);
    deltaTexcoord *= 1.0f / NUM_SAMPLES * lightData.godrayDenstiy;
    
    float3 color = SunTex.Sample(LinearClampSampler, texcoord);
    float illuminationDecay = 1.0f;
    
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        texcoord -= deltaTexcoord;
        float3 sample = SunTex.Sample(LinearClampSampler, texcoord);
        sample *= illuminationDecay * lightData.godrayWeight;
        color += sample;
        illuminationDecay *= lightData.godrayDecay;
    }
  
    return float4(color * lightData.godrayExposure, 1.0f);
}