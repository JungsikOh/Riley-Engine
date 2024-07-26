#include "DXResource.h"

namespace Riley {

void DXResource::CreateSRV(ID3D11Device *device, D3D11_SHADER_RESOURCE_VIEW_DESC *desc)
{
  ID3D11ShaderResourceView *srv;
  HR(device->CreateShaderResourceView(m_resource, desc, &srv));

  m_SRVs.push_back(srv);
}

void DXResource::CreateUAV(ID3D11Device *device, D3D11_UNORDERED_ACCESS_VIEW_DESC *desc)
{
  ID3D11UnorderedAccessView *uav;
  HR(device->CreateUnorderedAccessView(m_resource, desc, &uav));

  m_UAVs.push_back(uav);
}

bool DXResource::BindSRV(ID3D11DeviceContext *context, unsigned int bindSlot, DXShaderStage bindShader)
{
  if (!m_SRVs.empty()) {
    switch (bindShader) {
    case DXShaderStage::VS:
      context->VSSetShaderResources(bindSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), m_SRVs.data());
      break;
    case DXShaderStage::PS:
      context->PSSetShaderResources(bindSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), m_SRVs.data());
      break;
    case DXShaderStage::HS:
      context->HSSetShaderResources(bindSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), m_SRVs.data());
      break;
    case DXShaderStage::DS:
      context->DSSetShaderResources(bindSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), m_SRVs.data());
      break;
    case DXShaderStage::GS:
      context->GSSetShaderResources(bindSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), m_SRVs.data());
      break;
    case DXShaderStage::CS:
      context->CSSetShaderResources(bindSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), m_SRVs.data());
      break;
    default:
      break;
    }
    return true;
  }
  return false;
}

void DXResource::UnbindSRV(ID3D11DeviceContext *context, unsigned int boundSlot, DXShaderStage boundShader)
{
  if (!m_SRVs.empty()) {

    ID3D11ShaderResourceView *nullSRV = nullptr;

    switch (boundShader) {
    case DXShaderStage::VS:
      context->VSSetShaderResources(boundSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), &nullSRV);
      break;
    case DXShaderStage::PS:
      context->PSSetShaderResources(boundSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), &nullSRV);
      break;
    case DXShaderStage::HS:
      context->HSSetShaderResources(boundSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), &nullSRV);
      break;
    case DXShaderStage::DS:
      context->DSSetShaderResources(boundSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), &nullSRV);
      break;
    case DXShaderStage::GS:
      context->GSSetShaderResources(boundSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), &nullSRV);
      break;
    case DXShaderStage::CS:
      context->CSSetShaderResources(boundSlot + uint32(m_SRVs.size() - 1), uint32(m_SRVs.size()), &nullSRV);
      break;
    default:
      break;
    }
  }
}

bool DXResource::BindUAV(ID3D11DeviceContext *context, unsigned int bindSlot)
{
  if (!m_UAVs.empty()) {
    context->CSSetUnorderedAccessViews(
      bindSlot + uint32(m_UAVs.size() - 1), uint32(m_UAVs.size()), m_UAVs.data(), nullptr);
    return true;
  }
  return false;
}

void DXResource::UnbindUAV(ID3D11DeviceContext *context, unsigned int boundSlot)
{
  if (!m_UAVs.empty()) {
    ID3D11UnorderedAccessView *nullView = nullptr;
    context->CSSetUnorderedAccessViews(
      boundSlot + uint32(m_UAVs.size() - 1), uint32(m_UAVs.size()), &nullView, nullptr);
  }
}

}// namespace Riley