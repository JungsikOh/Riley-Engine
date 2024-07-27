#pragma once
#include "../Core/CoreTypes.h"
#include "../Core/Rendering.h"
#include "../Math/MathTypes.h"
#include "Components.h"

/** About std::optional
 * https://occamsrazr.net/tt/317
*/

namespace Riley {

enum class LightMesh {
    NoMesh,
    Quad, // For Sun
    Cube
};

class ModelImporter {
  public:
    ModelImporter() = default;
    ModelImporter(ID3D11Device* device, entt::registry& reg);
    ~ModelImporter() = default;

    std::vector<entt::entity>
    LoadSquare(const Vector3& pos, const float& scale = 1.0f,
               const float& rotate = 0.0f);

    std::vector<entt::entity> LoadBox(const Vector3& pos,
                                      const float& scale = 1.0f,
                                      const Vector2& texScale = Vector2(1.0f),
                                      bool invertNormals = false);

    std::vector<entt::entity> LoadLight(Light& lightData, LightMesh meshType,
                                        const float& scale = 1.0f);

  private:
    entt::registry& m_registry;
    ID3D11Device* m_device;
};
} // namespace Riley