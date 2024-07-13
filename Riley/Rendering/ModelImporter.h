#pragma once
#include "../Core/CoreTypes.h"
#include "../Core/ECS.h"
#include "../Core/Rendering.h"
#include "../Math/MathTypes.h"
#include "Object.h"
#include "Model.h"
#include <array>
#include <optional>

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
    ~ModelImporter() = default;

    std::vector<Mesh> LoadSquare(const float& scale = 1.0f, const Vector2& texScale = Vector2(1.0f));
    std::vector<Mesh> LoadBox(const float& scale = 1.0f, bool invertNormal = false);
};
} // namespace Riley