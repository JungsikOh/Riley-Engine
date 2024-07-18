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
 * ���� ���� ���� ���� ���, �����ϱ� ���� std::pair<type, bool>�� ���¸�
 * Ȱ���ϰ� �Ǵµ�, �� �κ��� ��� �� type�� ������(by ������) �����ϴ� ��찡
 * �߻��Ѵ�. �̸� �������� �ʰ� ������ ���� �������ִ� ���� std::optional�̴�.
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