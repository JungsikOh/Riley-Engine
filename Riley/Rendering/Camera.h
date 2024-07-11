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
    Matrix viewMat;
    Matrix projMat;
    Matrix viewProjMat;

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

    void UpdateViewMat();
    void OnResize(uint32 w, uint32 h);
    void Tick(float dt);

    void SetTransform(Transform const& _transform);
    void SetNearAndFar(float n, float f);
    void SetAspectRatio(float ar);
    void SetFov(float _fov);
    void SetProjMat(float fov, float aspect, float zn, float zf);
    void SetViewMat();

    const float& GetNear() const { return nearPlane; }
    const float& GetFar() const { return farPlane; }
    const float& GetFov() const { return fov; }
    const float& GetAspectRatio() const { return aspectRatio; }
    const BoundingFrustum& GetFrustum() const {
        BoundingFrustum frustum(GetProj());
        frustum.Transform(frustum, viewMat.Invert());
        return frustum;
    }
    const Matrix& GetViewProj() const { return viewProjMat; }
    const Matrix& GetProj() const { return projMat; }
    const Matrix& GetView() const { return viewMat; }
    const Transform& GetTransform() const { return transform; }

  public:
    void MoveFoward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void RotatePitch(int64 dy);
    void RotateYaw(int64 dx);
};
} // namespace Riley