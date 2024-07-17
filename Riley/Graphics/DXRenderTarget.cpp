#include "DXRenderTarget.h"

namespace Riley {
DXRenderTarget::DXRenderTarget(ID3D11Device* device, UINT width, UINT height,
                               DXFormat format, DXDepthStencilBuffer* buffer)
    : m_width(width), m_height(height), m_format(format),
      m_depthStencilBuffer(buffer) {

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = m_width;
    texDesc.Height = m_height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = ConvertDXFormat(m_format);
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HR(device->CreateTexture2D(
        &texDesc, nullptr, reinterpret_cast<ID3D11Texture2D**>(&m_resource)));

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    rtvDesc.Format = ConvertDXFormat(m_format);
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    HR(device->CreateRenderTargetView(
        reinterpret_cast<ID3D11Texture2D*>(&m_resource), &rtvDesc, &m_rtv));
}

DXRenderTarget::DXRenderTarget(ID3D11Device* device,
                               ID3D11RenderTargetView* rtv,
                               DXDepthStencilBuffer* buffer)
    : m_rtv(rtv), m_depthStencilBuffer(buffer) {
    ID3D11Resource* rtvResource = nullptr;
    rtv->GetResource(&rtvResource);
    if (rtvResource != nullptr) {
        ID3D11Texture2D* tex = reinterpret_cast<ID3D11Texture2D*>(rtvResource);

        D3D11_TEXTURE2D_DESC desc{};
        ZeroMemory(&desc, sizeof(desc));
        tex->GetDesc(&desc);
        m_width = buffer->m_width;
        m_height = buffer->m_height;
        m_format = ConvertDXGIFormat(desc.Format);

        SAFE_RELEASE(tex);
    } else
        m_rtv = nullptr;
}

DXRenderTarget::~DXRenderTarget() { SAFE_RELEASE(m_rtv); }

void DXRenderTarget::BindRenderTargetView(ID3D11DeviceContext* context) {
    if (m_rtv != nullptr)
        context->OMSetRenderTargets(1, &m_rtv, m_depthStencilBuffer->GetDSV());
}

void DXRenderTarget::UnBindRenderTargetView(ID3D11DeviceContext* context) {
    ID3D11RenderTargetView* nullRTV = nullptr;
    context->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void DXRenderTarget::Clear(ID3D11DeviceContext* context,
                           const float* clearColor) {
    context->ClearRenderTargetView(m_rtv, clearColor);
}

} // namespace Riley