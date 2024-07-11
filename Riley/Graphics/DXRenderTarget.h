#pragma once
#include "DXFormat.h"
#include "DXResource.h"

namespace Riley {
class DXRenderTarget : public DXResource {
  public:
    DXRenderTarget() = default;
    DXRenderTarget(ID3D11Device* device, UINT width, UINT height,
                   DXFormat format = DXFormat::R8G8B8A8_UNORM,
                   ID3D11DepthStencilView* dsv = nullptr);

    virtual ~DXRenderTarget();

    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    float GetAspectRatio() const { return (m_width / (float)m_height); }
    ID3D11Texture2D* GetTexture() const {
        return reinterpret_cast<ID3D11Texture2D*>(m_resource);
    }
    ID3D11RenderTargetView* GetRTV() const { return m_rtv; }

    void SetDepthStencilView(ID3D11DepthStencilView* dsv) {
        m_dsv = dsv;
    }

    void BindRenderTargetView(ID3D11DeviceContext* context);
    void Clear(ID3D11DeviceContext* context, const float* clearColor);

  private:
    ID3D11RenderTargetView* m_rtv;
    ID3D11DepthStencilView* m_dsv; // It is deleted by DXDepthStencilBuffer

    UINT m_width;
    UINT m_height;
    DXFormat m_format;
};

} // namespace Riley