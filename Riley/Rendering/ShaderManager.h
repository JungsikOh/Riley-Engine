#pragma once
#include "Enums.h"

namespace Riley
{
struct DXShaderProgram;

enum ShaderId : uint8
{
    VS_ScreenQuad,
    VS_Solid,
    VS_Phong,
    VS_GBuffer,
    VS_Shadow,
    VS_ShadowCascade,
    VS_ShadowCube,
    VS_Picking,
    PS_Solid,
    PS_Phong,
    PS_GBuffer,
    PS_Ambient,
    PS_DeferredLighting,
    PS_Halo,
    PS_SSAO,
    PS_SSAOBlur,
    PS_SSR,
    PS_Shadow,
    PS_ShadowCascade,
    PS_ShadowCube,
    PS_Picking,
    GS_ShadowCascade,
    GS_ShadowCube,
    ShaderIdCount
};

enum class ShaderProgram : uint8
{
    Solid,
    ForwardPhong,
    GBuffer,
    Ambient,
    DeferredLighting,
    Halo,
    SSAO,
    SSAOBlur,
    SSR,
    ShadowDepthMap,
    ShadowCascadeMap,
    ShadowDepthCubeMap,
    Picking,
    UnKnown
};

namespace ShaderManager
{
void Initialize(ID3D11Device* _device);
void Destroy();
DXShaderProgram* GetShaderProgram(ShaderProgram shaderProgram);
} // namespace ShaderManager

} // namespace Riley