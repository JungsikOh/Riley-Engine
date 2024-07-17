#pragma once

namespace Riley {
struct DXShaderProgram;

enum ShaderId : uint8 { VS_Solid, PS_Solid, ShaderIdCount };

enum class ShaderProgram : uint8 { Solid, Solid2, UnKnown };

enum class LightType : int32 { Directional, Point, Spot };

namespace ShaderManager {
void Initialize(ID3D11Device* _device);
void Destroy();
DXShaderProgram* GetShaderProgram(ShaderProgram shaderProgram);
} // namespace ShaderManager

} // namespace Riley