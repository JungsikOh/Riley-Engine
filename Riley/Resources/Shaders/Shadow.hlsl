#include "Common.hlsli"

struct VSInput
{
    float3 posModel : POSITION; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float2 texcoord : TEXCOORD0;
};

struct PSInput
{
    float4 posProj : SV_POSITION; // Screen position
    float2 texcoord : TEXCOORD0;
};

PSInput ShadowVS(VSInput input)
{
    PSInput output;
    
    float4 pos = float4(input.posModel, 1.0);
    pos = mul(pos, meshData.world);
    output.posProj = mul(pos, shadowData.lightViewProj);
    output.texcoord = input.texcoord;

    return output;
}


void ShadowPS(PSInput input)
{
}