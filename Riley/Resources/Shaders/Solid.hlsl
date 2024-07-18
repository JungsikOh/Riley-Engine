#include "Common.hlsli"

struct VSInput
{
    float3 posModel : POSITION; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float3 normalModel : NORMAL0; // ¸ðµ¨ ÁÂÇ¥°èÀÇ normal    
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
    float3 bitangentModel : BITANGENT0;
};

struct PSInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 normalWorld : NORMAL0;
    float3 normalView : NORMAL1;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 bitangentWorld : BITANGENT0;
};

PSInput SolidVS(VSInput input)
{
    PSInput output;
    
    float4 pos = float4(input.posModel, 1.0);
    pos = mul(pos, worldRow);
    output.posProj = mul(pos, viewProjRow);
    
    float3 normalWorld = mul(input.normalModel, (float3x3) worldInvTransposeRow);
    output.normalView = mul(normalWorld, (float3x3) invViewRow);
    output.tangentWorld = mul(input.tangentModel, (float3x3) worldRow);
    output.bitangentWorld = mul(input.bitangentModel, (float3x3) worldRow);
    output.normalWorld = normalWorld;
    output.texcoord = input.texcoord;

    return output;
}

struct PSOutput
{
    float4 pixelColor : SV_Target;
};

PSOutput SolidPS(PSInput input)
{
    PSOutput output;
    output.pixelColor = float4(diffuse, 1.0);
    return output;
}