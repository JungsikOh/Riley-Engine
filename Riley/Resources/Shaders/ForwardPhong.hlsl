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

float ShadowCalculation(LightData light, float3 posWorld, float3 viewPos, Texture2D ShadowMap)
{
    float shadow = 0.0;
    if (light.castShadows)
    {
        float4 shadowMapCoords = mul(float4(viewPos, 1.0), shadowData.shadow_matrices[0]);
        float3 UVD = shadowMapCoords.xyz / shadowMapCoords.w;
        UVD.xy = 0.5 * UVD.xy + 0.5;
        UVD.y = 1.0 - UVD.y;
        
        // PCF 3x3
        float2 texel;
        ShadowMap.GetDimensions(texel.x, texel.y);
        
        float2 texelSize = 1.0 / texel;
        
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                shadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, UVD.xy + float2(x, y) * texelSize, UVD.z - 0.05).r;
            }
        }
        shadow /= 9.0;
    }
    return shadow;
}

PSOutput PhongPS(PSInput input)
{
    PSOutput output;
    
    float4 viewPosition = mul(float4(input.posWorld, 1.0), frameData.view);
    float3 V = normalize(0.0.xxx - viewPosition.xyz);
    
    float3 ambient = materialData.ambient * 0.5;
    
    LightingResult Lo;
    float shadowFactor = ShadowCalculation(lightData, input.posWorld, viewPosition.xyz, ShadowMap);
    
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
    
    output.pixelColor = float4(ambient + ((Lo.diffuse + Lo.specular)) * shadowFactor, 1.0);
    return output;
}