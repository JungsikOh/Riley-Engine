#include "DXResource.h"

namespace Riley
{

void DXResource::Initialize(ID3D11Device* device, UINT width, UINT height, DXFormat format, D3D11_SUBRESOURCE_DATA* initData)
{

   D3D11_TEXTURE2D_DESC texDesc;
   ZeroMemory(&texDesc, sizeof(texDesc));
   texDesc.Width = width;
   texDesc.Height = height;
   texDesc.MipLevels = 1;
   texDesc.ArraySize = 1;
   texDesc.Format = ConvertDXFormat(format);
   texDesc.SampleDesc.Count = 1;
   texDesc.SampleDesc.Quality = 0;
   texDesc.Usage = D3D11_USAGE_DEFAULT;
   texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
   texDesc.CPUAccessFlags = 0;
   texDesc.MiscFlags = 0;

   HR(device->CreateTexture2D(&texDesc, initData, reinterpret_cast<ID3D11Texture2D**>(&m_resource)));
}

void DXResource::Initialize(ID3D11Device* device, D3D11_TEXTURE2D_DESC& desc, D3D11_SUBRESOURCE_DATA* initData)
{
   HR(device->CreateTexture2D(&desc, initData, reinterpret_cast<ID3D11Texture2D**>(&m_resource)));
}

void DXResource::Initialize( ID3D11Resource* resource , ID3D11ShaderResourceView* srv , ID3D11UnorderedAccessView* uav)
{
    m_resource = resource;
    if (srv)
        m_SRVs.push_back(srv);
    if (uav)
        m_UAVs.push_back(uav);
}

void DXResource::CreateSRV(ID3D11Device* device, D3D11_SHADER_RESOURCE_VIEW_DESC* desc)
{
   ID3D11ShaderResourceView* srv;
   HR(device->CreateShaderResourceView(m_resource, desc, &srv));
   SAFE_RELEASE(m_resource);
   m_SRVs.push_back(srv);
}

void DXResource::CreateUAV(ID3D11Device* device, D3D11_UNORDERED_ACCESS_VIEW_DESC* desc)
{
   ID3D11UnorderedAccessView* uav;
   HR(device->CreateUnorderedAccessView(m_resource, desc, &uav));
   SAFE_RELEASE(m_resource);
   m_UAVs.push_back(uav);
}

bool DXResource::BindSRV(ID3D11DeviceContext* context, unsigned int bindSlot, DXShaderStage bindShader)
{
   if (!m_SRVs.empty())
      {
         switch (bindShader)
            {
            case DXShaderStage::VS:
               context->VSSetShaderResources(bindSlot, uint32(m_SRVs.size()), m_SRVs.data());
               break;
            case DXShaderStage::PS:
               context->PSSetShaderResources(bindSlot, uint32(m_SRVs.size()), m_SRVs.data());
               break;
            case DXShaderStage::HS:
               context->HSSetShaderResources(bindSlot, uint32(m_SRVs.size()), m_SRVs.data());
               break;
            case DXShaderStage::DS:
               context->DSSetShaderResources(bindSlot, uint32(m_SRVs.size()), m_SRVs.data());
               break;
            case DXShaderStage::GS:
               context->GSSetShaderResources(bindSlot, uint32(m_SRVs.size()), m_SRVs.data());
               break;
            case DXShaderStage::CS:
               context->CSSetShaderResources(bindSlot, uint32(m_SRVs.size()), m_SRVs.data());
               break;
            default:
               break;
            }
         return true;
      }
   return false;
}

void DXResource::UnbindSRV(ID3D11DeviceContext* context, unsigned int boundSlot, DXShaderStage boundShader)
{
   if (!m_SRVs.empty())
      {
         std::vector<ID3D11ShaderResourceView*> nullSRV;
         nullSRV.resize(m_SRVs.size(), nullptr);

         switch (boundShader)
            {
            case DXShaderStage::VS:
               context->VSSetShaderResources(boundSlot, uint32(m_SRVs.size()), nullSRV.data());
               break;
            case DXShaderStage::PS:
               context->PSSetShaderResources(boundSlot, uint32(m_SRVs.size()), nullSRV.data());
               break;
            case DXShaderStage::HS:
               context->HSSetShaderResources(boundSlot, uint32(m_SRVs.size()), nullSRV.data());
               break;
            case DXShaderStage::DS:
               context->DSSetShaderResources(boundSlot, uint32(m_SRVs.size()), nullSRV.data());
               break;
            case DXShaderStage::GS:
               context->GSSetShaderResources(boundSlot, uint32(m_SRVs.size()), nullSRV.data());
               break;
            case DXShaderStage::CS:
               context->CSSetShaderResources(boundSlot, uint32(m_SRVs.size()), nullSRV.data());
               break;
            default:
               break;
            }
      }
}

bool DXResource::BindUAV(ID3D11DeviceContext* context, unsigned int bindSlot)
{
   if (!m_UAVs.empty())
      {
         context->CSSetUnorderedAccessViews(bindSlot, uint32(m_UAVs.size()), m_UAVs.data(), nullptr);
         return true;
      }
   return false;
}

void DXResource::UnbindUAV(ID3D11DeviceContext* context, unsigned int boundSlot)
{
   if (!m_UAVs.empty())
      {
         ID3D11UnorderedAccessView* nullView = nullptr;
         context->CSSetUnorderedAccessViews(boundSlot, uint32(m_UAVs.size()), &nullView, nullptr);
      }
}

} // namespace Riley