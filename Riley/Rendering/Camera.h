#pragma once
#include "../Math/MathTypes.h"

namespace Riley
{
  struct CameraParameters
  {
    float aspectRatio;
    float nearPlane;
    float farPlane;
    float fov;
    float speed;
    float sensitivity;
    Vector3 position;
    Vector3 lootAt;
  };

  class Camera
  {
  private:
    Matrix viewRow;
    Matrix projRow;

    Vector3 position;
    Vector3 rightVector = Vector3(1.0f, 0.0f, 0.0f);
    Vector3 upVector = Vector3(0.0f, 1.0f, 0.0f);
    Vector3 lookVector = Vector3(0.0f, 0.0f, 1.0f);
    float aspectRatio;
    float fov;
    float nearPlane;
    float farPlane;

    float speedFactor = 1.0f;
    float sensitivity = 0.3f;

  public:
    Camera() = default;
    explicit Camera(CameraParameters const&);

    void UpdateviewRow();
    void OnResize(uint32 w, uint32 h);
    void Tick(float dt);

    void SetPosition(Vector3 const& pos);
    void SetNearAndFar(float n, float f);
    void SetAspectRatio(float ar);
    void SetFov(float _fov);
    void SetprojRow(float fov, float aspect, float zn, float zf);
    void SetviewRow();

    Vector3 Position() const { return position; }
    Vector3 Up() const { return upVector; }
    Vector3 Right() const { return rightVector; }
    Vector3 Forward() const { return lookVector; }

    const float& Near() const { return nearPlane; }
    const float& Far() const { return farPlane; }
    const float& Fov() const { return fov; }
    const float& AspectRatio() const { return aspectRatio; }

    DirectX::BoundingFrustum Frustum()
    {
      DirectX::BoundingFrustum frustum(GetProj().Transpose());
      frustum.Transform(frustum, viewRow.Invert());
      return frustum;
    }

    Matrix GetViewProj()
    {
      Matrix viewProj = (viewRow * projRow).Transpose();
      return viewProj;
    }
    Matrix GetProj()
    {
      Matrix proj = projRow.Transpose();
      return proj;
    }
    Matrix GetView()
    {
      Matrix view = viewRow.Transpose();
      return view;
    }

  public:
    void MoveFoward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void RotatePitch(int64 dy);
    void RotateYaw(int64 dx);
  };
} // namespace Riley