#include "Common.hlsli"
#include "LightUtil.hlsli"

Texture2D NormalMetallicTx : register(t0);
Texture2D DiffuseRoughnessTx : register(t1);
Texture2D<float> DepthTx : register(t2);

Texture2D<float> ShadowMap : register(t4);
TextureCube<float> ShadowCubeMap : register(t5);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 DeferredLightingPS(VSToPS input) : SV_Target
{
    
}