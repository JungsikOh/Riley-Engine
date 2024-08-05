#include "Common.hlsli"
#include "LightUtil.hlsli"
#include "ShadowUtil.hlsli"

Texture2D DiffuseRoughnessTex : register(t0);
Texture2D NormalMetallicTex : register(t1);
Texture2D EmissiveTex : register(t2);
Texture2D<float> DepthTex : register(t3);

Texture2D ShadowMap : register(t4);
TextureCube ShadowCubeMap : register(t5);

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
    float roughness = diffuseRoughness.a;
    
    float4 emission = EmissiveTex.Sample(LinearWrapSampler, input.texcoord);
    
    LightingResult Lo;
    float shadowFactor = 1.0;
    
    // Lighting
    switch (lightData.type)
    {
        case DIRECTIONAL_LIGHT:
            Lo = DoDirectionalLight(lightData, 2.0, viewDir, normalVS);
            shadowFactor = CalcShadowMapPCF3x3(lightData, positionVS, ShadowMap);
            break;
        case POINT_LIGHT:
            Lo = DoPointLight(lightData, 2.0, viewDir, positionVS, normalVS);
            shadowFactor = CalcShadowCubeMapPCF3x3x3(lightData, positionVS, ShadowCubeMap);
            break;
        case SPOT_LIGHT:
            Lo = DoSpotLight(lightData, 2.0, viewDir, positionVS, normalVS);
            shadowFactor = CalcShadowMapPCF3x3(lightData, positionVS, ShadowMap);
            break;
    }
    
    return float4((Lo.diffuse + Lo.specular) * shadowFactor, 1.0);
}