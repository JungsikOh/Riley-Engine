#pragma once
#include "../Math/MathTypes.h"
#include "../Utilities/Event.h"
#include "Rendering.h"

namespace Riley {


class Transform {
  private:
    Matrix m_matrix;

    Vector3 m_position;
    Quaternion m_quaternion;
    Vector3 m_scale;
    Event m_OnTransformChanged;

  public:
    Transform() : m_matrix(Matrix()), m_scale(Vector3(1.0f)){};
    Transform(const Vector3& pos, const Vector3& _euler,
              const Vector3& _scale = Vector3(1.0f))
        : m_position(pos), m_quaternion(Quaternion::CreateFromYawPitchRoll(_euler)), m_scale(_scale) {
        UpdateTransform();
    }

    void OnEditor();
    void UpdateTransform();

    // noexcept : 예외를 방출하지 않겠다는 선언.
    void SetPosition(const Vector3& _position, bool notify = true) noexcept;
    void SetScale(const Vector3& _scale, bool notify = true) noexcept;
    void SetQuaternion(const Quaternion& _quaternion,
                       bool notify = true) noexcept;
    void SetQuaternion(const Vector3& _euler,
                       bool notify = true) noexcept;

    const Vector3 GetRight() const noexcept;
    const Vector3 GetUp() const noexcept;
    const Vector3 GetForward() const noexcept;

    const Vector3& GetPosition() const noexcept { return m_position; };
    const Vector3& GetScale() const noexcept { return m_scale; };
    const Matrix& GetMatrix() const noexcept { return m_matrix; };
    Matrix& GetMatrixRef() noexcept { return m_matrix; };
    const Quaternion& GetQuaternion() const noexcept { return m_quaternion; };
};

} // namespace Riley