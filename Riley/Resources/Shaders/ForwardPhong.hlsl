#include "Common.hlsli"
#include "LightUtil.hlsli"
#include "ShadowUtil.hlsli"

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
    float3 posWorld : POSITION0;
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
    output.posProj = mul(pos, frameData.viewProj);
    output.posWorld = pos.xyz;
    
    float3 normalWorld = normalize(mul(input.normalModel, (float3x3) transpose(meshData.worldInvTranspose)));
    output.normalView = mul(normalWorld, (float3x3) transpose(frameData.invView));
    output.tangentWorld = mul(input.tangentModel, (float3x3) meshData.world);
    output.bitangentWorld = mul(input.bitangentModel, (float3x3) meshData.world);
    output.normalWorld = normalWorld;
    output.texcoord = input.texcoord;

    return output;
}

struct PSOutput
{
    float4 pixelColor : SV_Target0;
};

PSOutput PhongPS(PSInput input)
{
    PSOutput output;
    
    float4 viewPosition = mul(float4(input.posWorld, 1.0), frameData.view);
    float3 V = normalize(0.0.xxx - viewPosition.xyz);
    
    float3 ambient = materialData.ambient * 0.5;
    
    LightingResult Lo;
    float shadowFactor = 0.0;
    
    switch (lightData.type)
    {
        case DIRECTIONAL_LIGHT:
            Lo = DoDirectionalLight(lightData, 2.0, V, input.normalView);
            break;
        case POINT_LIGHT:
            Lo = DoPointLight(lightData, 2.0, V, viewPosition.xyz, input.normalView);
            break;
        case SPOT_LIGHT:
            Lo = DoSpotLight(lightData, 8.0, V, viewPosition.xyz, input.normalView);
            break;
    }
    
    if (lightData.castShadows)
    {
        switch (lightData.type)
        {
            case DIRECTIONAL_LIGHT:
            case SPOT_LIGHT:
                shadowFactor = CalcShadowMapPCF3x3(lightData, viewPosition.xyz, ShadowMap);
                break;
            case POINT_LIGHT:
                shadowFactor = CalcShadowCubeMapPCF3x3x3(lightData, viewPosition.xyz, ShadowCubeMap);
                break;
        }
    }
    
    output.pixelColor = float4(ambient + (Lo.diffuse + Lo.specular) * shadowFactor, 1.0);
    return output;
}