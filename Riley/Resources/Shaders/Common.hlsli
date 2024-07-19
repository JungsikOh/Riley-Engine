#ifndef _COMMON_
#define _COMMON_

#include "ConstantData.hlsli"

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

cbuffer FrameBufferConsts : register(b0)
{
    FrameData frameData;
}

cbuffer LightConsts : register(b2)
{
    LightData lightData;
}

cbuffer MeshConsts : register(b1)
{
    MeshData meshData;
};

cbuffer MaterialConsts : register(b1)
{
    MaterialData materialData;
};
#endif