#ifndef _CONSTNAN_DATA_
#define _CONSTNAN_DATA_

static const int SSAO_KERNEL_SIZE = 16;
static const int CASCADE_COUNT = 4;

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
    
    float cameraFrustumX;
    float cameraFrustumY;
    float2 _dummy1;

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

    int castShadows;
    int useCascades;
    float radius;
    float haloStrength;
    
    float godrayDenstiy;
    float godrayWeight;
    float godrayDecay;
    float godrayExposure;
};

struct MeshData
{
    matrix world;
    matrix worldInvTranspose;
};

struct MaterialData
{
    float3 ambient;
    int useNormalMap;
    float3 diffuse;
    float alphaCutoff;
    float3 specular;
    float shininess;

    float albedoFactor;
    float metallicFactor;
    float roughnessFactor;
    float emissiveFactor;
};

struct PostprocessData
{
    int AO;
    float2 noiseScale;
    float ssaoRadius;
    
    float ssaoPower;
    float3 _dummy2;
    
    float4 samples[16];
    
    float ssrRayStep; // 한걸음에 얼마나 나아갈지에 대한 변수
    float ssrThickness;
    float2 _dummy3;
};

struct ShadowData
{
    matrix lightView;
    matrix lightViewProj;
    matrix shadowMatrices[CASCADE_COUNT];
    matrix shadowCascadeMapViewProj[CASCADE_COUNT];
    matrix shadowCubeMapViewProj[6];

    float4 splits;

    float softness;
    int shadowMapSize;
    int visualize;
    float _padding1;
};

#endif