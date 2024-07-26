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

Texture2D ShadowMap : register(t0);
TextureCube ShadowCubeMap : register(t1);

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

#endif