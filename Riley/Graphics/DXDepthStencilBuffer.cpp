#include "DXDepthStencilBuffer.h"

namespace Riley
{
DXDepthStencilBuffer::DXDepthStencilBuffer(ID3D11Device* device, uint32 width, uint32 height, bool isStencilEnable, bool isTextureCube)
{
   m_isStencilEnabled = isStencilEnable;
   m_isTextureCube = isTextureCube;

   D3D11_TEXTURE2D_DESC bufferDesc;
   ZeroMemory(&bufferDesc, sizeof(bufferDesc));
   bufferDesc.Width = width;
   bufferDesc.Height = height;
   bufferDesc.MipLevels = 1;
   bufferDesc.ArraySize = 1;
   bufferDesc.Format = m_isStencilEnabled ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
   bufferDesc.SampleDesc.Count = 1;
   bufferDesc.SampleDesc.Quality = 0;
   bufferDesc.Usage = D3D11_USAGE_DEFAULT;
   bufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
   bufferDesc.CPUAccessFlags = 0;
   bufferDesc.MiscFlags = 0;
   if (m_isTextureCube)
      {
         bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
         bufferDesc.ArraySize = 6;
      }

   HR(device->CreateTexture2D(&bufferDesc, nullptr, reinterpret_cast<ID3D11Texture2D**>(&m_resource)));

   D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
   ZeroMemory(&dsvDesc, sizeof(dsvDesc));
   dsvDesc.Format = m_isStencilEnabled ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
   dsvDesc.ViewDimension = m_isTextureCube ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;
   dsvDesc.Texture2D.MipSlice = 0;
   if (m_isTextureCube)
      {
         dsvDesc.Texture2DArray.ArraySize = 6;
         dsvDesc.Texture2DArray.FirstArraySlice = 0;
         dsvDesc.Texture2DArray.MipSlice = 0;
      }

   HR(device->CreateDepthStencilView(reinterpret_cast<ID3D11Texture2D*>(m_resource), &dsvDesc, &m_dsv));

   m_width = width;
   m_height = height;
}

void DXDepthStencilBuffer::Initialize(ID3D11Device* device, uint32 width, uint32 height, bool isStencilEnable,
                                      D3D11_TEXTURE2D_DESC* texDesc /*= nullptr*/,
                                      D3D11_DEPTH_STENCIL_VIEW_DESC* dsvDesc /*= nullptr*/)
{
   m_isStencilEnabled = isStencilEnable;
   m_width = width;
   m_height = height;

   if (texDesc == nullptr)
      {
         D3D11_TEXTURE2D_DESC bufferDesc;
         ZeroMemory(&bufferDesc, sizeof(bufferDesc));
         bufferDesc.Width = m_width;
         bufferDesc.Height = m_height;
         bufferDesc.MipLevels = 1;
         bufferDesc.ArraySize = 1;
         bufferDesc.Format = m_isStencilEnabled ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
         bufferDesc.SampleDesc.Count = 1;
         bufferDesc.SampleDesc.Quality = 0;
         bufferDesc.Usage = D3D11_USAGE_DEFAULT;
         bufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
         bufferDesc.CPUAccessFlags = 0;
         bufferDesc.MiscFlags = 0;

         HR(device->CreateTexture2D(&bufferDesc, nullptr, reinterpret_cast<ID3D11Texture2D**>(&m_resource)));
      }
   else
      {
         HR(device->CreateTexture2D(texDesc, nullptr, reinterpret_cast<ID3D11Texture2D**>(&m_resource)));
      }

   if (dsvDesc == nullptr)
      {
         D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
         ZeroMemory(&dsvDesc, sizeof(dsvDesc));
         dsvDesc.Format = m_isStencilEnabled ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
         dsvDesc.ViewDimension = m_isTextureCube ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;
         dsvDesc.Texture2D.MipSlice = 0;

         HR(device->CreateDepthStencilView(reinterpret_cast<ID3D11Texture2D*>(m_resource), &dsvDesc, &m_dsv));
      }
   else
      {
         HR(device->CreateDepthStencilView(reinterpret_cast<ID3D11Texture2D*>(m_resource), dsvDesc, &m_dsv));
      }
}

void DXDepthStencilBuffer::Clear(ID3D11DeviceContext* context, float depth, uint8 stencil)
{
   auto flags = D3D11_CLEAR_DEPTH | (m_isStencilEnabled ? D3D11_CLEAR_STENCIL : 0);
   context->ClearDepthStencilView(m_dsv, flags, depth, stencil);
}
} // namespace Riley