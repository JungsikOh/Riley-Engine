#ifndef __SHADOW_UTILS__
#define __SHADOW_UTILS__

#include "Common.hlsli"

float CalcShadowMapPCF3x3(LightData light, float3 viewPos, Texture2D shadowMap)
{
    float shadowFactor = 0.0;
    if (light.castShadows)
    {
        float4 shadowMapCoords = mul(float4(viewPos, 1.0), shadowData.shadowMatrices[0]);
        float3 UVD = shadowMapCoords.xyz / shadowMapCoords.w;
        UVD.xy = 0.5 * UVD.xy + 0.5;
        UVD.y = 1.0 - UVD.y;
        
        // PCF 3x3
        float2 texel;
        shadowMap.GetDimensions(texel.x, texel.y);
        
        float2 texelSize = 1.0 / texel;
        
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                shadowFactor += shadowMap.SampleCmpLevelZero(ShadowSampler, UVD.xy + float2(x, y) * texelSize, UVD.z).r;
            }
        }
        shadowFactor /= 9.0;

    }
    return shadowFactor;
}

float CalcShadowCascadeMapFCF3x3(LightData light, float3 viewPos, Texture2DArray shadowCascadeMap)
{
    float shadowFactor = 0.0;
    if (light.castShadows)
    {
        for (uint idx = 0; idx < CASCADE_COUNT; ++idx)
        {
            if (viewPos.z < shadowData.splits[idx])
            {
            
                float4 shadowCascadeMapCoords = mul(float4(viewPos, 1.0), shadowData.shadowMatrices[idx]);
                float3 UVD = shadowCascadeMapCoords.xyz / shadowCascadeMapCoords.w;
                UVD.xy = 0.5 * UVD.xy + 0.5;
                UVD.y = 1.0 - UVD.y;
            
            // Basic
            //shadowFactor += shadowCascadeMap.SampleCmpLevelZero(ShadowSampler, float3(UVD.xy, idx), UVD.z).r;
            
            // PCF 3x3
                const float dx = 1.0f / shadowData.shadowMapSize;
                const float2 offsets[9] =
                {
                    float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
                };
        
                for (int j = 0; j < 9; ++j)
                {
                    shadowFactor += shadowCascadeMap.SampleCmpLevelZero(ShadowSampler, float3(UVD.xy + offsets[j], idx), UVD.z).r;
                }
                shadowFactor /= (3.0 * 3.0);
                
                return shadowFactor;
            }
        }
    }
    return shadowFactor;
}

float CalcShadowCubeMapPCF3x3x3(LightData light, float3 viewPos, TextureCube shadowCubeMap)
{
    float shadowFactor = 0.0;
    
    float3 lightToPixelVS = viewPos - light.position.xyz; // View Space
    float3 lightToPixelWS = mul(float4(lightToPixelVS, 0.0), frameData.invView);
    
    const float zf = light.range;
    const float zn = 0.5;
    const float c1 = zf / (zf - zn);
    const float c0 = -zn * zf / (zf - zn);
    
    const float3 m = abs(lightToPixelWS).xyz;
    const float major = max(m.x, max(m.y, m.z));
    
    float fragmentDepth = (c1 * major + c0) / major;
    
    // Basic
    //shadow = shadowCubeMap.SampleCmpLevelZero(ShadowSampler, normalize(lightToPixelWS).xyz, fragmentDepth - 0.005).r;
    
    // PCF 3x3x3
    const float dx = 1.0 / shadowData.shadowMapSize;
        
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                shadowFactor += shadowCubeMap.SampleCmpLevelZero(ShadowSampler, normalize(lightToPixelWS).xyz + float3(x * dx, y * dx, z * dx), fragmentDepth).r;
            }
        }
    }
    shadowFactor /= (3.0 * 3.0 * 3.0);
    
    return shadowFactor;
}

#endif