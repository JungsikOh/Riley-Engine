#include "Camera.h"
#include "../Core/Input.h"
#include <algorithm>

namespace Riley {
Camera::Camera(CameraParameters const& desc)
    : transform{desc.transform}, fov{desc.fov}, aspectRatio{desc.aspectRatio},
      nearPlane{desc.nearPlane}, farPlane{desc.farPlane},
      sensitivity{desc.sensitivity} {
    SetviewRow();
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
}

void Camera::Tick(float dt) {
    Input& input = g_Input;
    input.Tick();
    if (input.GetKey(KeyCode::Space))
        return;

    if (input.GetKey(KeyCode::ShiftLeft))
        speedFactor *= 5.0f;
    if (input.GetKey(KeyCode::ShiftRight))
        speedFactor *= 0.2f;
    if (input.GetKey(KeyCode::W))
        MoveFoward(dt);
    if (input.GetKey(KeyCode::S))
        MoveFoward(-dt);
    if (input.GetKey(KeyCode::D))
        MoveRight(dt);
    if (input.GetKey(KeyCode::A))
        MoveRight(-dt);
    if (input.GetKey(KeyCode::Q))
        MoveUp(dt);
    if (input.GetKey(KeyCode::E))
        MoveUp(-dt);
    if (input.GetKey(KeyCode::MouseRight)) {
        float dx = input.GetMouseDeltaX();
        float dy = input.GetMouseDeltaY();
        pitch = (int64)dy;
        yaw = (int64)dx;
    }
    UpdateviewRow();
}

void Camera::UpdateviewRow() { SetviewRow(); }

void Camera::OnResize(uint32 w, uint32 h) {
    SetAspectRatio(static_cast<float>(w) / h);
}

void Camera::SetAspectRatio(float ar) {
    aspectRatio = ar;
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
}
void Camera::SetFov(float _fov) {
    fov = _fov;
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
}
void Camera::SetNearAndFar(float n, float f) {
    nearPlane = n;
    farPlane = f;
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
}

void Camera::SetTransform(Transform const& t) { transform = t; }

void Camera::MoveFoward(float dt) {
    Vector3 oldPos = transform.GetPosition();
    Vector3 newPos = oldPos + dt * speedFactor * transform.GetForward();
    transform.SetPosition(newPos);
    UpdateviewRow();
}

void Camera::MoveRight(float dt) {
    Vector3 oldPos = transform.GetPosition();
    Vector3 newPos = oldPos + dt * speedFactor * transform.GetRight();
    transform.SetPosition(newPos);
    UpdateviewRow();
}

void Camera::MoveUp(float dt) {
    Vector3 oldPos = transform.GetPosition();
    Vector3 newPos = oldPos + dt * speedFactor * transform.GetUp();
    transform.SetPosition(newPos);
    UpdateviewRow();
}

void Camera::RotatePitch(int64 dy) {
    Matrix rotate = Matrix::CreateFromAxisAngle(
        transform.GetRight(),
        sensitivity * DirectX::XMConvertToRadians((float)dy));
    transform.SetPosition(Vector3::Transform(transform.GetPosition(), rotate));
    UpdateviewRow();
}

void Camera::RotateYaw(int64 dx) {
    Matrix rotate = Matrix::CreateRotationY(
        sensitivity * DirectX::XMConvertToRadians((float)dx));
    transform.SetPosition(Vector3::Transform(transform.GetPosition(), rotate));
    UpdateviewRow();
}

void Camera::SetprojRow(float fov, float aspect, float zn, float zf) {
    projRow = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), aspect,
                                                zn, zf);
}

void Camera::SetviewRow() {
    viewRow = DirectX::XMMatrixLookToLH(
        transform.GetPosition(), transform.GetForward(), transform.GetUp());
}

} // namespace Riley