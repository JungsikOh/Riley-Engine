#include "Common.hlsli"

struct VSInput
{
    float3 posModel : POSITION; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float2 texcoord : TEXCOORD0;
};

struct VSToPS
{
    float4 posProj : SV_POSITION; // Screen position
    float2 texcoord : TEXCOORD0;
};

VSToPS SolidVS(VSInput input)
{
    VSToPS output;
    
    float4 pos = float4(input.posModel, 1.0);
    pos = mul(pos, meshData.world);
    output.posProj = mul(pos, frameData.viewProj);
    output.texcoord = input.texcoord;

    return output;
}

struct PSOutput
{
    float4 pixelColor : SV_Target;
};

PSOutput SolidPS(VSToPS input)
{
    PSOutput output;
    output.pixelColor = float4(materialData.diffuse, 1.0);
    return output;
}