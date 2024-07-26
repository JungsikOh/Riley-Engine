#include "DXRenderTarget.h"

namespace Riley {

DXRenderTarget::DXRenderTarget(ID3D11Device *device,
  UINT width,
  UINT height,
  DXFormat format,
  DXDepthStencilBuffer *buffer)
  : m_width(width), m_height(height), m_format(format), m_depthStencilBuffer(buffer)
{
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

  HR(device->CreateTexture2D(&texDesc, nullptr, reinterpret_cast<ID3D11Texture2D **>(&m_resource)));

  D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
  ZeroMemory(&rtvDesc, sizeof(rtvDesc));
  rtvDesc.Format = ConvertDXFormat(m_format);
  rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  rtvDesc.Texture2D.MipSlice = 0;

  ID3D11RenderTargetView *rtv;
  HR(device->CreateRenderTargetView(reinterpret_cast<ID3D11Texture2D *>(m_resource), &rtvDesc, &rtv));

  m_RTVs.push_back(rtv);
}

DXRenderTarget::DXRenderTarget(ID3D11Device *device, ID3D11RenderTargetView *rtv, DXDepthStencilBuffer *buffer)
  : m_depthStencilBuffer(buffer)
{
  ID3D11Resource *rtvResource = nullptr;
  rtv->GetResource(&rtvResource);
  m_resource = rtvResource;
  if (rtvResource != nullptr) {
    ID3D11Texture2D *tex = reinterpret_cast<ID3D11Texture2D *>(rtvResource);

    D3D11_TEXTURE2D_DESC desc{};
    ZeroMemory(&desc, sizeof(desc));
    tex->GetDesc(&desc);
    m_width = desc.Width;
    m_height = desc.Height;
    m_format = ConvertDXGIFormat(desc.Format);

    SAFE_RELEASE(tex);
  }
  m_RTVs.push_back(rtv);
}

DXRenderTarget::~DXRenderTarget()
{
  for (uint64 i = 0; i < m_RTVs.size(); ++i) { SAFE_RELEASE(m_RTVs[i]); }
}

void DXRenderTarget::Initalize(ID3D11Device *device,
  UINT width,
  UINT height,
  DXFormat format,
  DXDepthStencilBuffer *buffer)
{
  m_width = width;
  m_height = height;
  m_format = format;
  m_depthStencilBuffer = buffer;

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

  HR(device->CreateTexture2D(&texDesc, nullptr, reinterpret_cast<ID3D11Texture2D **>(&m_resource)));

  D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
  ZeroMemory(&rtvDesc, sizeof(rtvDesc));
  rtvDesc.Format = ConvertDXFormat(m_format);
  rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  rtvDesc.Texture2D.MipSlice = 0;

  ID3D11RenderTargetView *rtv;
  HR(device->CreateRenderTargetView(reinterpret_cast<ID3D11Texture2D *>(m_resource), &rtvDesc, &rtv));

  m_RTVs.push_back(rtv);
}

void CreateRenderTarget(ID3D11Device *device, DXFormat formar) {}

void DXRenderTarget::BindRenderTargetView(ID3D11DeviceContext *context)
{
  if (!m_RTVs.empty())
    context->OMSetRenderTargets(uint32(m_RTVs.size()), m_RTVs.data(), m_depthStencilBuffer->GetDSV());
}

/**
 * @brief Bind null RTV and DepthOnlyDSV
 */
void DXRenderTarget::BindOnlyDepthStencilView(ID3D11DeviceContext *context)
{
  ID3D11RenderTargetView *nullRTV = nullptr;
  context->OMSetRenderTargets(1, &nullRTV, m_depthStencilBuffer->GetDSV());
}

void DXRenderTarget::Clear(ID3D11DeviceContext *context, const float *clearColor)
{
  for (int i = 0; i < m_RTVs.size(); ++i) { context->ClearRenderTargetView(m_RTVs[i], clearColor); }
}

}// namespace Riley