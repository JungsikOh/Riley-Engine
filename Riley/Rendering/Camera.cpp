#include "Camera.h"
#include "../Core/Input.h"

namespace Riley
{
  Camera::Camera(CameraParameters const& desc)
      : position{desc.position}, lookVector{desc.lootAt}, fov{desc.fov},
        aspectRatio{desc.aspectRatio}, nearPlane{desc.nearPlane},
        farPlane{desc.farPlane}, sensitivity{desc.sensitivity}
  {
    SetviewRow();
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
  }

  void Camera::Tick(float dt)
  {
    Input& input = g_Input;
    if(input.GetKey(KeyCode::Space)) return;

    if(input.GetKey(KeyCode::ShiftLeft)) speedFactor += 0.5f;
    if(input.GetKey(KeyCode::ShiftRight)) speedFactor *= 0.8f;
    if(input.GetKey(KeyCode::W)) MoveFoward(dt);
    if(input.GetKey(KeyCode::S)) MoveFoward(-dt);
    if(input.GetKey(KeyCode::D)) MoveRight(dt);
    if(input.GetKey(KeyCode::A)) MoveRight(-dt);
    if(input.GetKey(KeyCode::Q)) MoveUp(dt);
    if(input.GetKey(KeyCode::E)) MoveUp(-dt);
    if(input.GetKey(KeyCode::MouseRight))
      {
        float dx = input.GetMouseDeltaX();
        float dy = input.GetMouseDeltaY();
        RotatePitch((int64)dy);
        RotateYaw((int64)dx);
      }
    UpdateviewRow();
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
  }

  void Camera::UpdateviewRow()
  {
    lookVector.Normalize();
    upVector = lookVector.Cross(rightVector);
    upVector.Normalize();
    rightVector = upVector.Cross(lookVector);
    rightVector.Normalize();
    SetviewRow();
  }

  void Camera::OnResize(uint32 w, uint32 h)
  {
    SetAspectRatio(static_cast<float>(w) / h);
  }

  void Camera::SetAspectRatio(float ar)
  {
    aspectRatio = ar;
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
  }
  void Camera::SetFov(float _fov)
  {
    fov = _fov;
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
  }
  void Camera::SetNearAndFar(float n, float f)
  {
    nearPlane = n;
    farPlane = f;
    SetprojRow(fov, aspectRatio, nearPlane, farPlane);
  }

  void Camera::SetPosition(Vector3 const& pos) { position = pos; }

  void Camera::MoveFoward(float dt)
  {
    Vector3 oldPos = position;
    Vector3 newPos = oldPos + dt * speedFactor * lookVector;
    SetPosition(newPos);
    UpdateviewRow();
  }

  void Camera::MoveRight(float dt)
  {
    Vector3 oldPos = position;
    Vector3 newPos = oldPos + dt * speedFactor * rightVector;
    SetPosition(newPos);
    UpdateviewRow();
  }

  void Camera::MoveUp(float dt)
  {
    Vector3 oldPos = position;
    Vector3 newPos = oldPos + dt * speedFactor * upVector;
    SetPosition(newPos);
    UpdateviewRow();
  }

  void Camera::RotatePitch(int64 dy)
  {
    Matrix rotate = Matrix::CreateFromAxisAngle(
      rightVector, sensitivity * DirectX::XMConvertToRadians((float)dy));
    upVector = Vector3::TransformNormal(upVector, rotate);
    lookVector = Vector3::TransformNormal(lookVector, rotate);
    UpdateviewRow();
  }

  void Camera::RotateYaw(int64 dx)
  {
    Matrix rotate = Matrix::CreateRotationY(
      sensitivity * DirectX::XMConvertToRadians((float)dx));
    rightVector = Vector3::TransformNormal(rightVector, rotate);
    upVector = Vector3::TransformNormal(upVector, rotate);
    lookVector = Vector3::TransformNormal(lookVector, rotate);
    UpdateviewRow();
  }

  void Camera::SetprojRow(float fov, float aspect, float zn, float zf)
  {
    projRow = DirectX::XMMatrixPerspectiveFovLH(
      DirectX::XMConvertToRadians(fov), aspect, zn, zf);
  }

  void Camera::SetviewRow()
  {
    viewRow = DirectX::XMMatrixLookToLH(position, lookVector, upVector);
  }

} // namespace Riley