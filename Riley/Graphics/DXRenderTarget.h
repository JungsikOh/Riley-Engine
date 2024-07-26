#pragma once
#include "DXDepthStencilBuffer.h"
#include "DXFormat.h"
#include "DXResource.h"

namespace Riley {
class DXRenderTarget : public DXResource {
  public:
    DXRenderTarget() = default;
    DXRenderTarget(ID3D11Device* device, UINT width, UINT height,
                   DXFormat format = DXFormat::R8G8B8A8_UNORM,
                   DXDepthStencilBuffer* buffer = nullptr);
    DXRenderTarget(ID3D11Device* device, ID3D11RenderTargetView* rtv,
                   DXDepthStencilBuffer* buffer = nullptr);

    virtual ~DXRenderTarget();

    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    float GetAspectRatio() const { return (m_width / (float)m_height); }
    ID3D11Texture2D* GetTexture() const {
        return reinterpret_cast<ID3D11Texture2D*>(m_resource);
    }
    std::vector<ID3D11RenderTargetView*> GetRTVs() const { return m_RTVs; }
    ID3D11RenderTargetView* RTV(uint64 i = 0) const { return m_RTVs[i]; }

    void SetDepthStencilBuffer(DXDepthStencilBuffer* buffer) {
        m_depthStencilBuffer = buffer;
    }

    void Initalize(ID3D11Device* device, UINT width, UINT height,
                             DXFormat format, DXDepthStencilBuffer* buffer);

    void CreateRenderTarget(ID3D11Device* device, UINT width, UINT height,
                            DXFormat format, DXDepthStencilBuffer* buffer);
    void CreateRenderTarget(ID3D11Device* device, ID3D11RenderTargetView* rtv,
                            DXDepthStencilBuffer* buffer);

    void BindRenderTargetView(ID3D11DeviceContext* context);
    void BindOnlyDepthStencilView(ID3D11DeviceContext* context);
    void Clear(ID3D11DeviceContext* context, const float* clearColor);

  private:
    std::vector<ID3D11RenderTargetView*> m_RTVs;
    DXDepthStencilBuffer* m_depthStencilBuffer; // It is deleted by DXDepthStencilBuffer

    UINT m_width;
    UINT m_height;
    DXFormat m_format;
};

} // namespace Riley