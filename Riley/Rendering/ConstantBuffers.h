#pragma once
#include "RenderSetting.h"

namespace Riley
{

struct FrameBufferConsts
{
    Matrix view;
    Matrix proj;
    Matrix viewProj;

    Matrix invView;
    Matrix invProj;
    Matrix invViewProj;

    Matrix prevView;
    Matrix prevProj;
    Matrix prevViewProj;

    Vector4 globalAmbient;
    Vector4 cameraPosition;
    Vector4 cameraFoward;

    float cameraNear;
    float cameraFar;
    float cameraJitterX;
    float cameraJitterY;

    float screenResolutionX;
    float screenResolutionY;
    float mouseNormalizedCoordsX;
    float mouseNormalizedCoordsY;
};

struct LightConsts
{
    Vector4 screenSpacePosition;
    Vector4 position;
    Vector4 direction;
    Vector4 lightColor;

    float range;
    int32 type;
    float outerCosine;
    float innerCosine;

    int32 castShadows;
    Vector3 _padding1;
    int32 useCascades;
};

struct ObjectConsts
{
    Matrix world;
    Matrix worldInvTranspose;
};

struct MaterialConsts
{
    Vector3 ambient;
    int32 entityID;
    Vector3 diffuse;
    float alphaCutoff;
    Vector3 specular;
    float shininess;

    float albedoFactor;
    float metallicFactor;
    float roughnessFactor;
    float emissiveFactor;
};

DECLSPEC_ALIGN(16) struct PostprocessConsts
{
    int32 AO;
    Vector2 noiseScale;
    float ssaoRadius;

    float ssaoPower;
    Vector3 _dummy;

    Vector4 samples[16];
};

struct ShadowConsts
{
    Matrix lightView;
    Matrix lightViewProj;
    Matrix shadowMatrices[CASCADE_COUNT];
    Matrix shadowCascadeMapViewProj[CASCADE_COUNT]; // cascade shadow mapping
    Matrix shadowCubeMapViewProj[6];

    float split0;
    float split1;
    float split2;
    float split3;

    float softness;
    int32 shadow_map_size = CASCADE_COUNT;
    int32 visualize;
    float _padding1;
};

DECLSPEC_ALIGN(16) struct EntityIdConsts
{
    uint32 entityID = uint32(-1);
    Vector3 _dummy2;
};

} // namespace Riley