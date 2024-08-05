#pragma once

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
   VS_ShadowCube,
   PS_Solid,
   PS_Phong,
   PS_GBuffer,
   PS_Ambient,
   PS_DeferredLighting,
   PS_Shadow,
   PS_ShadowCube,
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
   ShadowDepthMap,
   ShadowDepthCubeMap,
   UnKnown
};

enum class LightType : int32
{
   Directional,
   Point,
   Spot
};

namespace ShaderManager
{
void Initialize(ID3D11Device* _device);
void Destroy();
DXShaderProgram* GetShaderProgram(ShaderProgram shaderProgram);
} // namespace ShaderManager

} // namespace Riley