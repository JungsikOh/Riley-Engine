#include "../Common.hlsli"

struct VSInput
{
    float3 posModel : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VSToPS
{
    float4 posProj : SV_POSITION; // Screen position
    float2 texcoord : TEXCOORD0;
};

VSToPS PickingVS(VSInput input)
{
    VSToPS output;
    
    float4 pos = float4(input.posModel, 1.0);
    pos = mul(pos, meshData.world);
    output.posProj = mul(pos, frameData.viewProj);
    output.texcoord = input.texcoord;

    return output;
}

cbuffer EntityIDConsts : register(b10)
{
    uint entityID;
    float3 _dummy4;
}

float4 PickingPS(VSToPS input) : SV_TARGET
{
    return float4(entityID, 0, 0, 0);
}