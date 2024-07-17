#pragma once
#include "../Math/MathTypes.h"
#include <vector>

namespace Riley {

struct FrameBufferConsts {
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

struct LightConsts {
    Vector4 screenSpacePosition;
    Vector4 position;
    Vector4 direction;
    Vector4 lightColor;

    float range;
    int32 type;
    float outerCosine;
    float innerCosine;

    float castShadows;
    Vector3 _padding1;
};

struct ObjectConsts {
    Matrix worldRow;
    Matrix worldInvTransposeRow;
};

struct MaterialConsts {
    Vector3 ambient;
    float _padding1;
    Vector3 diffuse;
    float alphaCutoff;
    Vector3 specular;
    float shininess;

    float albedoFactor;
    float metallicFactor;
    float roughnessFactor;
    float emissiveFactor;
};

} // namespace Riley