#include "../Common.hlsli"
#include "../Util/LightUtil.hlsli"
#include "../Util/ShadowUtil.hlsli"

#define MAX_TILE_LIGHTS 256
#define TILED_GROUP_SIZE 16

Texture2D DiffuseRoughnessTex : register(t0);
Texture2D NormalMetallicTex : register(t1);
Texture2D EmissiveTex : register(t2);
Texture2D<float> DepthTex : register(t3);
StructuredBuffer<PackedLightData> LightsBuffer : register(t4);

Texture2D ShadowMap : register(t5);
TextureCube ShadowCubeMap : register(t6);
Texture2DArray ShadowCascadeMap : register(t7);

RWTexture2D<float4> OutputTex : register(u0);
RWTexture2D<float4> DebugTex : register(u1);
//RWStructuredBuffer<TiledLightListBuffer> OutputLightsList : register(u2);

groupshared uint s_MinZ;
groupshared uint s_MaxZ;
groupshared uint s_TileNumLights;
groupshared uint s_TileLightIndices[MAX_TILE_LIGHTS];

[numthreads(TILED_GROUP_SIZE, TILED_GROUP_SIZE, 1)]
void TiledDeferredLighting(int3 gID : SV_GroupID,
                        uint3 dtID : SV_DispatchThreadID,
                        uint3 gtID : SV_GroupThreadID,
                        uint gIdx : SV_GroupIndex)
{
    uint totalLights, unused;
    LightsBuffer.GetDimensions(totalLights, unused);
    
    float pixelMinZ = frameData.cameraFar;
    float pixelMaxZ = frameData.cameraNear;
    
    float2 texcoord = float2((dtID.x + 0.5) / frameData.screenResolutionX, (dtID.y + 0.5) / frameData.screenResolutionY);
    
    float depth = DepthTex.Load(int3(dtID.xy, 0)).r;
    float viewSpaceDepth = ConvertZToLinearDepth(depth);
    
    bool vaildPixel = viewSpaceDepth >= frameData.cameraNear && viewSpaceDepth < frameData.cameraFar;
    
    [flatten]
    if (vaildPixel)
    {
        pixelMinZ = min(pixelMinZ, viewSpaceDepth);
        pixelMaxZ = max(pixelMaxZ, viewSpaceDepth);
    }
    
    if (gIdx == 0)
    {
        s_TileNumLights = 0;
        s_MinZ = 0x7F7FFFFF;
        s_MaxZ = 0;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (pixelMaxZ >= pixelMinZ)
    {
        InterlockedMin(s_MinZ, asuint(pixelMinZ));
        InterlockedMax(s_MaxZ, asuint(pixelMaxZ));
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float tileMinZ = asfloat(s_MinZ);
    float tileMaxZ = asfloat(s_MaxZ);
    
    float2 tileScale = float2(frameData.screenResolutionX, frameData.screenResolutionY) * rcp(float(2 * TILED_GROUP_SIZE));
    float2 tileBias = tileScale - float2(gID.xy);
    
    float4 c1 = float4(frameData.proj._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, -frameData.proj._22 * tileScale.y, tileBias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);
    
    float4 frustumPlanes[6];
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c4 + c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c4 + c2;
    frustumPlanes[4] = float4(0.0f, 0.0f, 1.0f, -tileMinZ);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f, tileMaxZ);
    
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }
    
    for (uint lightIdx = gIdx; lightIdx < totalLights; lightIdx += TILED_GROUP_SIZE * TILED_GROUP_SIZE)
    {
        PackedLightData light = LightsBuffer[lightIdx];
        if (!light.active)
            continue;
        
        bool inFrustum = true;
        if (light.type != DIRECTIONAL_LIGHT)
        {
            [unroll]
            for (uint i = 0; i < 6; ++i)
            {
                float d = dot(frustumPlanes[i], float4(light.position.xyz, 1.0f));
                inFrustum = inFrustum && (d >= -light.range / 2.0f);
            }
        }
        
        [branch]
        if (inFrustum)
        {
            uint listIndex;
            InterlockedAdd(s_TileNumLights, 1, listIndex);
            s_TileLightIndices[listIndex] = lightIdx;
            DebugTex[dtID.xy] = s_TileNumLights / 3.0f;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    
    float3 viewSpacePosition = GetViewSpacePosition(texcoord, depth);
    float4 normalMetallic = NormalMetallicTex.Load(int3(dtID.xy, 0));
    float3 normal = 2.0 * normalMetallic.rgb - 1.0;
    float metallic = normalMetallic.a;
    
    float4 albedoRoughness = DiffuseRoughnessTex.Load(int3(dtID.xy, 0));
    
    float3 V = normalize(0.0f.xxx - viewSpacePosition);
    float roughness = albedoRoughness.a;
        
    float3 Lo = 0.0f;
    float shadowFactor = 0.0f;
    if (all(dtID.xy < float2(frameData.screenResolutionX, frameData.screenResolutionY)))
    {
        for (uint i = 0; i < s_TileNumLights; ++i)
        {
            PackedLightData packedLightData = LightsBuffer[s_TileLightIndices[i]];
            LightData light = ConvertFromPackedLightData(packedLightData);
            switch (light.type)
            {
                case DIRECTIONAL_LIGHT:
                    shadowFactor = lightData.useCascades ? CalcShadowCascadeMapFCF3x3(light, viewSpacePosition, ShadowCascadeMap) : CalcShadowMapPCF3x3(light, viewSpacePosition, ShadowMap);
                    Lo += DoDirectinoalLightPBR(light, viewSpacePosition, normal, V, albedoRoughness.rgb, metallic, roughness) * shadowFactor;
                    break;
                case POINT_LIGHT:
                    shadowFactor = CalcShadowCubeMapPCF3x3x3(light, viewSpacePosition, ShadowCubeMap);
                    Lo += DoPointLightPBR(light, viewSpacePosition, normal, V, albedoRoughness.rgb, metallic, roughness) * shadowFactor;
                    break;
                case SPOT_LIGHT:
                    shadowFactor = CalcShadowMapPCF3x3(light, viewSpacePosition, ShadowMap);
                    Lo += DoSpotLightPBR(light, viewSpacePosition, normal, V, albedoRoughness.rgb, metallic, roughness) * shadowFactor;
                    break;
            }
        }
    }
    
    float4 shadingColor = OutputTex.Load(int3(int2(dtID.xy), 0)) + float4(Lo, 1.0f);
    OutputTex[dtID.xy] = shadingColor;
    
    // 히트맵 색상 범위
    const float3 HeatMap[] =
    {
        float3(0, 0, 0), // Black
        float3(0, 0, 1), // Blue
        float3(0, 1, 1), // Cyan
        float3(0, 1, 0), // Green
        float3(1, 1, 0), // Yellow
        float3(1, 0, 0), // Red
    };
    
    float intensity = saturate(s_TileNumLights / 5.0f) * 3.0f;
    float3 a = HeatMap[floor(intensity)];
    float3 b = HeatMap[ceil(intensity)];
    
    float4 debugColor = float4(lerp(a, b, intensity - floor(intensity)), 0.5);
    //DebugTex[dtID.xy] = debugColor;

}