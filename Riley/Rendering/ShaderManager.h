#pragma once

namespace Riley
{
  struct DXShaderProgram;

  enum ShaderId : uint8
  {
    VS_Solid,
    VS_Phong,
    VS_Shadow,
    PS_Solid,
    PS_Phong,
    PS_Shadow,
    ShaderIdCount
  };

  enum class ShaderProgram : uint8
  {
    Solid,
    ForwardPhong,
    ShadowDepthMap,
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
    void Initialize(ID3D11Device *_device);
    void Destroy();
    DXShaderProgram *GetShaderProgram(ShaderProgram shaderProgram);
  } // namespace ShaderManager

} // namespace Riley