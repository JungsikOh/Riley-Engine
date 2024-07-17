#pragma once
#include "../Core/Transform.h"
#include "../Math/MathTypes.h"

namespace Riley {
struct CameraParameters {
    float aspectRatio;
    float nearPlane;
    float farPlane;
    float fov;
    float speed;
    float sensitivity;
    Transform transform;
};

class Camera {
  private:
    Matrix viewRow;
    Matrix projRow;

    Transform transform;
    float aspectRatio;
    float fov;
    float nearPlane;
    float farPlane;

    float pitch;
    float yaw;

    float speedFactor = 3.0f;
    float sensitivity = 20.0f;

  public:
    Camera() = default;
    explicit Camera(CameraParameters const&);

    void UpdateviewRow();
    void OnResize(uint32 w, uint32 h);
    void Tick(float dt);

    void SetTransform(Transform const& _transform);
    void SetNearAndFar(float n, float f);
    void SetAspectRatio(float ar);
    void SetFov(float _fov);
    void SetprojRow(float fov, float aspect, float zn, float zf);
    void SetviewRow();

    const float& GetNear() const { return nearPlane; }
    const float& GetFar() const { return farPlane; }
    const float& GetFov() const { return fov; }
    const float& GetAspectRatio() const { return aspectRatio; }
    const BoundingFrustum& GetFrustum() {
        BoundingFrustum frustum(GetProj());
        frustum.Transform(frustum, viewRow.Invert());
        return frustum;
    }
    const Matrix& GetViewProj() {
        Matrix viewProj = (viewRow * projRow).Transpose();
        return viewProj;
    }
    const Matrix& GetProj() { 
        Matrix proj = projRow.Transpose();
        return proj; }
    const Matrix& GetView() {
        Matrix view = viewRow.Transpose();
        return view; }
    const Transform& GetTransform() { return transform; }

  public:
    void MoveFoward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void RotatePitch(int64 dy);
    void RotateYaw(int64 dx);
};
} // namespace Riley