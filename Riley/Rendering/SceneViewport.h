#pragma once
#include "../Core/CoreTypes.h"

namespace Riley {
class SceneViewport {
  public:
    SceneViewport() = default;
    ~SceneViewport() = default;

    void SetWidth(float width) { m_width = width; }
    float GetWidth() const { return m_width; }

    void SetHeight(float height) { m_height = height; }
    float GetHeight() const { return m_height; }

    void SetMinDepth(float minDepth) { m_minDepth = minDepth; }
    float GetMinDepth() const { return m_minDepth; }

    void SetMaxDepth(float maxDepth) { m_maxDepth = maxDepth; }
    float GetMaxDepth() const { return m_maxDepth; }

    void SetTopLeftX(float topLeftX) { m_topLeftX = topLeftX; }
    float GetTopLeftX() const { return m_topLeftX; }

    void SetTopLeftY(float topLeftY) { m_topLeftY = topLeftY; }
    float GetTopLeftY() const { return m_topLeftY; }

    D3D11_VIEWPORT GetD3DViewport() const {
        D3D11_VIEWPORT viewport;
        ZeroMemory(&viewport, sizeof(viewport));

        viewport.Width = m_width;
        viewport.Height = m_height;
        viewport.MinDepth = m_minDepth;
        viewport.MaxDepth = m_maxDepth;
        viewport.TopLeftX = m_topLeftX;
        viewport.TopLeftY = m_topLeftY;
        return viewport;
    }

    void BindViewport(ID3D11DeviceContext* context) {
        D3D11_VIEWPORT viewport = GetD3DViewport();
        context->RSSetViewports(1, &viewport);
    }

  public:
    float m_width;
    float m_height;
    float m_minDepth = 0.0f;
    float m_maxDepth = 1.0f;
    float m_topLeftX = 0.0f;
    float m_topLeftY = 0.0f;

    float m_mousePositionX;
    float m_mousePositionY;

    bool isViewportFocused;
};
} // namespace Riley