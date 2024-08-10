#pragma once
#include "../Rendering/RenderSetting.h"
#include "../Rendering/Camera.h"
#include "BoundingVolume.h"

namespace Riley
{

namespace LightFrustum
{
using namespace DirectX;

static std::pair<Matrix, Matrix> DirectionalLightViewProjection(const Light& light, Camera* camera, BoundingBox& cullBox)
{
    // [1] Camera view frustum
    BoundingFrustum frustum = camera->Frustum();
    std::array<Vector3, BoundingFrustum::CORNER_COUNT> corners = {};
    frustum.GetCorners(corners.data());

    BoundingSphere frustumSphere;
    BoundingSphere::CreateFromFrustum(frustumSphere, frustum);

    Vector3 frustumCenter(0.0f, 0.0f, 0.0f);
    for (uint32 i = 0; i < corners.size(); ++i)
    {
        frustumCenter = frustumCenter + corners[i];
    }
    frustumCenter /= static_cast<float>(corners.size());
    
    float radius = 0.0f;
    for (Vector3 const& corner : corners)
    {
        float distance = Vector3::Distance(corner, frustumCenter);
        radius = std::max(radius, distance);
    }
    radius = std::ceil(radius * 8.0f) / 8.0f; // ±×¸²ÀÚ ¸Ê ÇØ»óµµ¿¡ ¸ÂÃã.

    const Vector3 max_extents(radius, radius, radius);
    const Vector3 min_extents = -max_extents;
    const Vector3 cascade_extents = max_extents - min_extents;

    Vector4 lightDir = light.direction;
    lightDir.Normalize();
    Vector3 up = Vector3::Up;

    if (abs(up.Dot(Vector3(lightDir)) + 1.0f) < 1e-5)
        up = Vector3(1.0f, 0.0f, 0.0f);

    Matrix lightViewRow = XMMatrixLookAtLH(frustumCenter, frustumCenter + 1.0f * lightDir * radius, up);

    float l = min_extents.x;
    float b = min_extents.y;
    float n = min_extents.z;
    float r = max_extents.x;
    float t = max_extents.y;
    float f = max_extents.z * 1.5f;

    Matrix lightProjRow = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
    Matrix lightViewProjRow = lightViewRow * lightProjRow;

    // 카메라 위치 변화에 따른 그림자 떨림 방지를 위한 offset
    Vector3 shadowOrigin = Vector3(0.0f, 0.0f, 0.0f);
    shadowOrigin = Vector3::Transform(shadowOrigin, lightViewProjRow); // 그림자 좌표 공간에서의 원점
    shadowOrigin *= (SHADOW_MAP_SIZE / 2.0f);                          // 그림자 원점을 texture pixel 단위로 변환

    Vector3 roundedOrigin = XMVectorRound(shadowOrigin);
    Vector3 roundedOffset = roundedOrigin - shadowOrigin; // texel에 맞추기 위한 offset 계산

    roundedOffset *= (2.0f / SHADOW_MAP_SIZE); // 그림자 맵 좌표계로 다시 변환
    roundedOffset.z = 0.0f;                    // 깊이값 영향 무시

    // offset을 x, y값에 더하여 그림자가 texel의 중앙에 위치하도록 조정
    lightProjRow.m[3][0] += roundedOffset.x;
    lightProjRow.m[3][1] += roundedOffset.y;

    BoundingBox::CreateFromPoints(cullBox, Vector4(l, b, n, 1.0f), Vector4(r, t, f, 1.0f));
    cullBox.Transform(cullBox, lightViewRow.Invert()); // Camera View Space -> world Space

    return {lightViewRow, lightProjRow};
}

static std::pair<Matrix, Matrix> DirectionalCascadeLightViewProjection(const Light& light, Camera* camera, const Matrix& projMatrix, BoundingBox& cullBox)
{
    BoundingFrustum frustum(projMatrix);
    frustum.Transform(frustum, camera->GetView().Transpose().Invert());
    std::array<Vector3, BoundingFrustum::CORNER_COUNT> corners{};
    frustum.GetCorners(corners.data());

    Vector3 frustumCenter(0.0f);
    for (uint32 i = 0; i < corners.size(); ++i)
    {
        frustumCenter += corners[i];
    }
    frustumCenter /= static_cast<float>(corners.size());

    float radius = 0.0f;
    for (const Vector3& corner : corners)
    {
        float distance = Vector3::Distance(frustumCenter, corner);
        radius = std::max(radius, distance);
    }
    radius = std::ceil(radius * 8.0f) / 8.0f;

    const Vector3 maxExtents = Vector3(radius, radius, radius);
    const Vector3 minExtents = -maxExtents;
    const Vector3 cascadeExtents = maxExtents - minExtents;

    Vector4 lightDir = light.direction;
    lightDir.Normalize();
    Vector3 up = Vector3::Up;
    if (abs(up.Dot(Vector3(lightDir)) + 1.0f) < 1e-5)
        up = Vector3(1.0f, 0.0f, 0.0f);

    Matrix lightViewRow = XMMatrixLookAtLH(frustumCenter, frustumCenter + 1.0f * lightDir * radius, up);

    float l = minExtents.x;
    float b = minExtents.y;
    float n = minExtents.z - 1.5f * radius;
    float r = maxExtents.x;
    float t = maxExtents.y;
    float f = maxExtents.z * 1.5f;
    Matrix lightProjRow = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
    Matrix lightViewProjRow = lightViewRow * lightProjRow;

    // 카메라 위치 변화에 따른 그림자 떨림 방지를 위한 offset
    Vector3 shadowOrigin = Vector3(0.0f, 0.0f, 0.0f);
    shadowOrigin = Vector3::Transform(shadowOrigin, lightViewProjRow); // 그림자 좌표 공간에서의 원점
    shadowOrigin *= (SHADOW_CASCADE_SIZE / 2.0f);                      // 그림자 원점을 texture pixel 단위로 변환

    Vector3 roundedOrigin = XMVectorRound(shadowOrigin);
    Vector3 roundedOffset = roundedOrigin - shadowOrigin; // texel에 맞추기 위한 offset 계산

    roundedOffset *= (2.0f / SHADOW_CASCADE_SIZE); // 그림자 맵 좌표계로 다시 변환
    roundedOffset.z = 0.0f;                    // 깊이값 영향 무시

    // offset을 x, y값에 더하여 그림자가 texel의 중앙에 위치하도록 조정
    lightProjRow.m[3][0] += roundedOffset.x;
    lightProjRow.m[3][1] += roundedOffset.y;

    BoundingBox::CreateFromPoints(cullBox, Vector4(l, b, n, 1.0f), Vector4(r, t, f, 1.0f));
    cullBox.Transform(cullBox, lightViewRow.Invert()); // Camera View Space -> world Space

    return {lightViewRow, lightProjRow};
}

static std::array<Matrix, CASCADE_COUNT> RecalcProjectionMatrices(Camera* camera, const float& splitLambda, std::array<float, CASCADE_COUNT>& splitDistances)
{
    float nearZ = camera->Near();
    float farZ = camera->Far();
    float fov = camera->Fov();
    float aspectRatio = camera->AspectRatio();
    float f = 1.0f / CASCADE_COUNT;

    for (uint32 i = 0; i < splitDistances.size(); ++i)
    {
        float fi = (i + 1) * f;
        float l = nearZ * pow(farZ / nearZ, fi);
        float u = nearZ + (farZ - nearZ) * fi;
        splitDistances[i] = l * splitLambda + u * (1.0f - splitLambda);
    }

    std::array<Matrix, CASCADE_COUNT> projMatrices{};
    projMatrices[0] = XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, splitDistances[0]);
    for (uint32 i = 1; i < projMatrices.size(); ++i)
    {
        projMatrices[i] = XMMatrixPerspectiveFovLH(forv, aspectRatio, splitDistances[i - 1], splitDistances[i]);
    }
    
    return projMatrices;
}

} // namespace LightFrustum

} // namespace Riley