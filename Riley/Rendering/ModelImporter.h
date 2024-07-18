#pragma once
#include "../Core/CoreTypes.h"
#include "../Core/Rendering.h"
#include "../Math/MathTypes.h"
#include "Components.h"
#include <entt.hpp>
#include <optional>
#include <vector>

/** About std::optional
 * https://occamsrazr.net/tt/317
 * 보통 리턴 값이 없는 경우, 리턴하기 위해 std::pair<type, bool>의 형태를
 * 활용하게 되는데, 이 부분의 경우 빈 type을 생성해(by 생성자) 리턴하는 경우가
 * 발생한다. 이를 생성하지 않고 리턴을 쉽게 구현해주는 것이 std::optional이다.
 */

namespace Riley {
class ModelImporter {
  public:
    ModelImporter() = default;
    ModelImporter(ID3D11Device* device, entt::registry& reg);
    ~ModelImporter() = default;

    std::vector<entt::entity>
    LoadSquare(const Vector3& pos, const float& scale = 1.0f,
               const Vector2& texScale = Vector2(1.0f));

  private:
    entt::registry& m_registry;
    ID3D11Device* m_device;
};
} // namespace Riley