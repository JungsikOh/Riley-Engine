#include "DXDepthStencilBuffer.h"

namespace Riley {
DXDepthStencilBuffer::DXDepthStencilBuffer(ID3D11Device* device, uint32 width,
                                           uint32 height, bool isStencilEnable) {
    m_isStencilEnabled = isStencilEnable;

    D3D11_TEXTURE2D_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Width = width;
    bufferDesc.Height = height;
    bufferDesc.MipLevels = 1;
    bufferDesc.ArraySize = 1;
    bufferDesc.Format = m_isStencilEnabled ? DXGI_FORMAT_R24G8_TYPELESS
                                           : DXGI_FORMAT_R32_TYPELESS;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags =
        D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    HR(device->CreateTexture2D(
        &bufferDesc, nullptr,
        reinterpret_cast<ID3D11Texture2D**>(&m_resource)));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = m_isStencilEnabled ? DXGI_FORMAT_D24_UNORM_S8_UINT
                                        : DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    HR(device->CreateDepthStencilView(
        static_cast<ID3D11Texture2D*>(m_resource), &dsvDesc, &m_dsv));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = m_isStencilEnabled ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS
                                        : DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    HR(InitShaderResourceView(device, &srvDesc));
}

void DXDepthStencilBuffer::Clear(ID3D11DeviceContext* context, float depth,
                                 uint8 stencil) {
    auto flags =
        D3D11_CLEAR_DEPTH | (m_isStencilEnabled ? D3D11_CLEAR_STENCIL : 0);
    context->ClearDepthStencilView(m_dsv, flags, depth, stencil);
}
} // namespace Riley