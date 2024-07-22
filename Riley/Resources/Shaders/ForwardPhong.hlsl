#include "Common.hlsli"
#include "LightUtil.hlsli"

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
    float4 posWorld : position1;
    float3 normalWorld : NORMAL0;
    float3 normalView : NORMAL1;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 bitangentWorld : BITANGENT0;
};

PSInput PhongVS(VSInput input)
{
    PSInput output;
    
    float4 pos = float4(input.posModel, 1.0);
    pos = mul(pos, meshData.world);
    output.posWorld = pos;
    output.posProj = mul(pos, frameData.viewProj);
    
    float3 normalWorld = mul(input.normalModel, (float3x3) meshData.worldInvTranspose);
    output.normalView = mul(normalWorld, (float3x3) transpose(frameData.invView));
    output.tangentWorld = mul(input.tangentModel, (float3x3) meshData.world);
    output.bitangentWorld = mul(input.bitangentModel, (float3x3) meshData.world);
    output.normalWorld = normalWorld;
    output.texcoord = input.texcoord;

    return output;
}

struct PSOutput
{
    float4 pixelColor : SV_Target;
};

PSOutput PhongPS(PSInput input)
{
    PSOutput output;
    
    float4 viewPosition = mul(input.posWorld, frameData.view);
    float3 pixelToEye = normalize(0.0.xxx - viewPosition.xyz);
    
    float3 ambient = materialData.ambient;
    
    LightingResult Lo;
    switch (lightData.type)
    {
        case DIRECTIONAL_LIGHT:
            Lo = DoDirectionalLight(lightData, 2.0, pixelToEye, input.normalView);
            break;
        case POINT_LIGHT:
            Lo = DoPointLight(lightData, 2.0, pixelToEye, viewPosition.xyz, input.normalView);
            break;
        case SPOT_LIGHT:
            Lo = DoSpotLight(lightData, 2.0, pixelToEye, viewPosition.xyz, input.normalView);
            break;
    }
    
    output.pixelColor = float4(ambient + (Lo.diffuse + Lo.specular), 1.0);
    return output;
}