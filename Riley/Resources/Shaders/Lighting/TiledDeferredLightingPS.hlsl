#include "../Common.hlsli"
#include "../Util/LightUtil.hlsli"
#include "../Util/ShadowUtil.hlsli"

Texture2D DiffuseRoughnessTex : register(t0);
Texture2D NormalMetallicTex : register(t1);
Texture2D EmissiveTex : register(t2);
Texture2D<float> DepthTex : register(t3);

StructuredBuffer<PackedLightData> LightsBuffer : register(t4);
StructuredBuffer<TiledLightListBuffer> OutputLightsList : register(t5);

Texture2D ShadowMap : register(t6);
TextureCube ShadowCubeMap : register(t7);
Texture2DArray ShadowCascadeMap : register(t8);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 TiledDeferredLightingPS(VSToPS input) : SV_Target
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
    
    float3 Lo = 0.0f;
    float shadowFactor = 0.0f;

    TiledLightListBuffer tiledLightList = OutputLightsList[input.posWorld.x + frameData.screenResolutionX * input.posWorld.y];
    
    for (uint i = 0; i < tiledLightList.tileNumLights; ++i)
    {
        PackedLightData packedLightData = LightsBuffer[tiledLightList.tileLightIndices[i]];
        LightData light = ConvertFromPackedLightData(packedLightData);
        switch (light.type)
        {
            case DIRECTIONAL_LIGHT:
                shadowFactor = lightData.useCascades ? CalcShadowCascadeMapFCF3x3(light, positionVS, ShadowCascadeMap) : CalcShadowMapPCF3x3(light, positionVS, ShadowMap);
                Lo += DoDirectinoalLightPBR(light, positionVS, normalVS, viewDir, albedo.rgb, metallic, roughness) * shadowFactor;
                break;
            case POINT_LIGHT:
                shadowFactor = CalcShadowCubeMapPCF3x3x3(light, positionVS, ShadowCubeMap);
                Lo += DoPointLightPBR(light, positionVS, normalVS, viewDir, albedo.rgb, metallic, roughness) * shadowFactor;
                break;
            case SPOT_LIGHT:
                shadowFactor = CalcShadowMapPCF3x3(light, positionVS, ShadowMap);
                Lo += DoSpotLightPBR(light, positionVS, normalVS, viewDir, albedo.rgb, metallic, roughness) * shadowFactor;
                break;
        }

    }
    
    return float4(Lo * shadowFactor, 1.0);
}