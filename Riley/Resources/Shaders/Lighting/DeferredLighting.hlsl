#include "../Common.hlsli"
#include "../Util/LightUtil.hlsli"
#include "../Util/ShadowUtil.hlsli"

Texture2D DiffuseRoughnessTex : register(t0);
Texture2D NormalMetallicTex : register(t1);
Texture2D EmissiveTex : register(t2);
Texture2D<float> DepthTex : register(t3);

Texture2D ShadowMap : register(t4);
TextureCube ShadowCubeMap : register(t5);
Texture2DArray ShadowCascadeMap : register(t6);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 DeferredLightingPS(VSToPS input) : SV_Target
{
    float depth = DepthTex.Sample(LinearWrapSampler, input.texcoord);
    float3 positionVS = GetViewSpacePosition(input.texcoord, depth);
    float3 viewDir = normalize(0.0f.xxx - positionVS);
    
    float4 normalMetallic = NormalMetallicTex.Sample(LinearWrapSampler, input.texcoord);
    float3 normalVS = normalMetallic.rgb * 2.0 - 1.0;
    float metallic = normalMetallic.a;
    
    float4 diffuseRoughness = DiffuseRoughnessTex.Sample(LinearWrapSampler, input.texcoord);
    float3 albedo = diffuseRoughness.rgb;
    float roughness = diffuseRoughness.a;
    
    LightingResult Lo;
    float3 LoPBR = float3(0, 0, 0);
    float shadowFactor = 0.0;
    
    // Lighting
    switch (lightData.type)
    {
        case DIRECTIONAL_LIGHT:
            //Lo = DoDirectionalLight(lightData, 2.0, viewDir, normalVS);
            LoPBR = DoDirectinoalLightPBR(lightData, positionVS, normalVS, viewDir, albedo, metallic, roughness);
            shadowFactor = lightData.useCascades ? CalcShadowCascadeMapFCF3x3(lightData, positionVS, ShadowCascadeMap) : CalcShadowMapPCF3x3(lightData, positionVS, ShadowMap);
            break;
        case POINT_LIGHT:
            //Lo = DoPointLight(lightData, 2.0, viewDir, positionVS, normalVS);
            LoPBR = DoPointLightPBR(lightData, positionVS, normalVS, viewDir, albedo, metallic, roughness);
            shadowFactor = CalcShadowCubeMapPCF3x3x3(lightData, positionVS, ShadowCubeMap);
            break;
        case SPOT_LIGHT:
            //Lo = DoSpotLight(lightData, 2.0, viewDir, positionVS, normalVS);
            LoPBR = DoSpotLightPBR(lightData, positionVS, normalVS, viewDir, albedo, metallic, roughness);
            shadowFactor = CalcShadowMapPCF3x3(lightData, positionVS, ShadowMap);
            break;
    }
    
    return float4(LoPBR * shadowFactor, 1.0);
}