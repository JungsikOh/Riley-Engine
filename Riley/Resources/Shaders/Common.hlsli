#ifndef _COMMON_
#define _COMMON_

cbuffer FrameBufferConsts : register(b0)
{
    matrix viewRow;
    matrix projRow;
    matrix viewProjRow;
    
    matrix invViewRow;
    matrix invProjRow;
    matrix invViewProjRow;

    matrix prevViewRow;
    matrix prevProjRow;
    matrix prevViewProjRow;

    float4 globalAmbient;
    float4 cameraPosition;
    float4 cameraFoward;

    float cameraNear;
    float cameraFar;
    float cameraJitterX;
    float cameraJitterY;

    float screenResolutionX;
    float screenResolutionY;
    float mouseNormalizedCoordsX;
    float mouseNormalizedCoordsY;
}

cbuffer ObjectConsts : register(b1)
{
    matrix worldRow;
    matrix worldInvTransposeRow;
};


cbuffer MaterialConsts : register(b0)
{
    float3 ambient;
    float _padding1;
    float3 diffuse;
    float alphaCutoff;
    float3 specular;
    float shininess;

    float albedoFactor;
    float metallicFactor;
    float roughnessFactor;
    float emissiveFactor;
};

#endif