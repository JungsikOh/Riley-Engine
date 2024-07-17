#include "Transform.h"
#include "../Math/MathTypes.h"
#ifndef CEDITOR
#include "../Editor/Editor.h"
#endif

namespace Riley {

void Transform::UpdateTransform() {
    Matrix scale = Matrix::CreateScale(GetScale());
    Matrix rotation = Matrix::CreateFromQuaternion(GetQuaternion());
    Matrix translation = Matrix::CreateTranslation(GetPosition());

    m_matrix = Matrix() * scale * rotation * translation;
    m_quaternion = Quaternion::CreateFromRotationMatrix(m_matrix);
    m_OnTransformChanged();
}

void Transform::SetPosition(const Vector3& position, bool notify) noexcept {
    m_position = position;
    if (notify)
        UpdateTransform();
}

void Transform::SetScale(const Vector3& scale, bool notify) noexcept {
    m_scale = scale;
    if (notify)
        UpdateTransform();
}

void Transform::SetQuaternion(const Quaternion& quaternion,
                              bool notify) noexcept {
    m_quaternion = quaternion;
    if (notify)
        UpdateTransform();
}

void Transform::SetQuaternion(const Vector3& _euler,
                              bool notify) noexcept {
    m_quaternion = Quaternion::CreateFromYawPitchRoll(_euler);
    if (notify)
        UpdateTransform();
}

const Vector3 Transform::GetRight() const noexcept {
    return Vector3::TransformNormal(Vector3(1.0f, 0.0f, 0.0f),
                              Matrix::CreateFromQuaternion(m_quaternion));
}

const Vector3 Transform::GetUp() const noexcept {
    return Vector3::TransformNormal(Vector3(0.0f, 1.0f, 0.0f),
                              Matrix::CreateFromQuaternion(m_quaternion));
}

const Vector3 Transform::GetForward() const noexcept {
    return Vector3::TransformNormal(Vector3(0.0f, 0.0f, 1.0f),
                              Matrix::CreateFromQuaternion(m_quaternion));
}


} // namespace Riley