#pragma once
#include "../Core/Rendering.h"
#include "DXShader.h"

namespace Riley {
enum class DXRenderResourceType {
  VertexBuffer,
  IndexBuffer,
  ConstantBuffer,
  StructuredBuffer,
  Texture1D,
  Texture2D,
  Texture3D,
  TextureCube,
  RenderTarget,
  DepthStencilBuffer,
  DynamicCubemap,
};

class DXResource
{
public:
  DXResource() : m_resource(nullptr), m_isMapped(false) {}

  virtual ~DXResource()
  {
    for (uint64 i = 0; i < m_SRVs.size(); ++i) { SAFE_RELEASE(m_SRVs[i]); }
    for (uint64 i = 0; i < m_UAVs.size(); ++i) { SAFE_RELEASE(m_UAVs[i]); }
    SAFE_RELEASE(m_resource);
  }

  ID3D11Resource *GetResource() const { return m_resource; }
  std::vector<ID3D11ShaderResourceView *> GetShaderResourceViews() const { return m_SRVs; }
  std::vector<ID3D11UnorderedAccessView *> GetUnorderedAccessViews() const { return m_UAVs; }

  void CreateSRV(ID3D11Device *device, D3D11_SHADER_RESOURCE_VIEW_DESC *desc)
  {
    ID3D11ShaderResourceView *srv;
    HR(device->CreateShaderResourceView(m_resource, desc, &srv));

    m_SRVs.push_back(srv);
  }

  void CreateUAV(ID3D11Device *device, D3D11_UNORDERED_ACCESS_VIEW_DESC *desc)
  {
    ID3D11UnorderedAccessView *uav;
    HR(device->CreateUnorderedAccessView(m_resource, desc, &uav));

    m_UAVs.push_back(uav);
  }

  bool BindSRV(ID3D11DeviceContext *context, unsigned int bindSlot, DXShaderStage bindShader)
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

  void UnbindSRV(ID3D11DeviceContext *context, unsigned int boundSlot, DXShaderStage boundShader)
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

  bool BindUAV(ID3D11DeviceContext *context, unsigned int bindSlot)
  {
    if (!m_UAVs.empty()) {
      context->CSSetUnorderedAccessViews(
        bindSlot + uint32(m_UAVs.size() - 1), uint32(m_UAVs.size()), m_UAVs.data(), nullptr);
      return true;
    }
    return false;
  }

  void UnbindUAV(ID3D11DeviceContext *context, unsigned int boundSlot)
  {
    if (!m_UAVs.empty()) {
      ID3D11UnorderedAccessView *nullView = nullptr;
      context->CSSetUnorderedAccessViews(
        boundSlot + uint32(m_UAVs.size() - 1), uint32(m_UAVs.size()), &nullView, nullptr);
    }
  }

  ID3D11ShaderResourceView *SRV(uint64 i = 0) const { return m_SRVs[i]; }
  ID3D11UnorderedAccessView *UAV(uint64 i = 0) const { return m_UAVs[i]; }

  DXResource &operator=(const DXResource &r)
  {
    m_resource = r.GetResource();
    m_SRVs = r.GetShaderResourceViews();
    m_UAVs = r.GetUnorderedAccessViews();
  }

  bool IsMapped() const { return m_isMapped; }

  void *Map(ID3D11DeviceContext *context)
  {
    D3D11_MAPPED_SUBRESOURCE ms;
    auto result = context->Map(m_resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    if (HR(result)) {
      m_isMapped = true;
      return ms.pData;
    }
    return nullptr;
  }

  bool UnMap(ID3D11DeviceContext *context)
  {
    if (IsMapped()) {
      context->Unmap(m_resource, 0);
      m_isMapped = false;
      return true;
    }
    return false;
  }

  template<typename ResourceType> ResourceType *Map(ID3D11DeviceContext *context)
  {
    return reinterpret_cast<ResourceType *>(Map(context));
  }

protected:
  ID3D11Resource *m_resource;
  std::vector<ID3D11ShaderResourceView *> m_SRVs;
  std::vector<ID3D11UnorderedAccessView *> m_UAVs;
  bool m_isMapped;
};
}// namespace Riley