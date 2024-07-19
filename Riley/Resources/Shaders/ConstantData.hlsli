#ifndef _CONSTNAN_DATA_
#define _CONSTNAN_DATA_

struct FrameData
{
    matrix view;
    matrix proj;
    matrix viewProj;
    
    matrix invView;
    matrix invProj;
    matrix invViewProj;

    matrix prevView;
    matrix prevProj;
    matrix prevViewProj;

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
};

struct LightData
{
    float4 screenSpacePosition;
    float4 position;
    float4 direction;
    float4 lightColor;

    float range;
    int type;
    float outerCosine;
    float innerCosine;

    float castShadows;
    float3 _padding2;
};

struct MeshData
{
    matrix world;
    matrix worldInvTranspose;
};

struct MaterialData
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