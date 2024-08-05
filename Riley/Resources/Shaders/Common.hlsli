#ifndef _COMMON_
#define _COMMON_

#include "ConstantData.hlsli"

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

SamplerState LinearWrapSampler : register(s0);
SamplerState LinearClampSampler : register(s1);
SamplerState ShadowPointSampler : register(s2);
SamplerComparisonState ShadowSampler : register(s3);

cbuffer FrameBufferConsts : register(b0)
{
    FrameData frameData;
}

cbuffer MeshConsts : register(b1)
{
    MeshData meshData;
};

cbuffer MaterialConsts : register(b1)
{
    MaterialData materialData;
};

cbuffer LightConsts : register(b2)
{
    LightData lightData;
}

cbuffer ShadowConsts : register(b3)
{
    ShadowData shadowData;
}

static float3 GetViewSpacePosition(float2 texcoord, float depth)
{
    float4 clipSpaceLocation;
    clipSpaceLocation.xy = texcoord * 2.0f - 1.0f;
    clipSpaceLocation.y *= -1;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    float4 homogenousLocation = mul(clipSpaceLocation, frameData.invProj);
    return homogenousLocation.xyz / homogenousLocation.w;
}

#endif