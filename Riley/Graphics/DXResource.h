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

class DXResource {
  public:
    DXResource()
        : m_resource(nullptr), m_srv(nullptr), m_uav(nullptr),
          m_isMapped(false) {}

    virtual ~DXResource() {
        SAFE_RELEASE(m_srv);
        SAFE_RELEASE(m_uav);
        SAFE_RELEASE(m_resource);
    }

    ID3D11Resource* GetResource() const { return m_resource; }
    ID3D11ShaderResourceView* GetShaderResourceView() const { return m_srv; }
    ID3D11UnorderedAccessView* GetUnorderedAccessView() const { return m_uav; }

    bool IsMapped() const { return m_isMapped; }

    void* Map(ID3D11DeviceContext* context) {
        D3D11_MAPPED_SUBRESOURCE ms;
        auto result =
            context->Map(m_resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        if (HR(result)) {
            m_isMapped = true;
            return ms.pData;
        }
        return nullptr;
    }

    bool UnMap(ID3D11DeviceContext* context) {
        if (IsMapped()) {
            context->Unmap(m_resource, 0);
            m_isMapped = false;
            return true;
        }
        return false;
    }

    template <typename ResourceType>
    ResourceType* Map(ID3D11DeviceContext* context) {
        return reinterpret_cast<ResourceType*>(Map(context));
    }

    bool BindSRV(ID3D11DeviceContext* context,
                                unsigned int bindSlot,
                                DXShaderStage bindShader) {
        if (m_srv != nullptr) {
            switch (bindShader) {
            case DXShaderStage::VS:
                context->VSSetShaderResources(bindSlot, 1, &m_srv);
                break;
            case DXShaderStage::PS:
                context->PSSetShaderResources(bindSlot, 1, &m_srv);
                break;
            case DXShaderStage::HS:
                context->HSSetShaderResources(bindSlot, 1, &m_srv);
                break;
            case DXShaderStage::DS:
                context->DSSetShaderResources(bindSlot, 1, &m_srv);
                break;
            case DXShaderStage::GS:
                context->GSSetShaderResources(bindSlot, 1, &m_srv);
                break;
            case DXShaderStage::CS:
                context->CSSetShaderResources(bindSlot, 1, &m_srv);
                break;
            default:
                break;
            }
            return true;
        }
        return false;
    }

    void UnbindSRV(ID3D11DeviceContext* context, unsigned int boundSlot,
                   DXShaderStage boundShader) {
        if (m_srv != nullptr) {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            switch (boundShader) {
            case DXShaderStage::VS:
                context->VSSetShaderResources(boundSlot, 1, &nullSRV);
                break;
            case DXShaderStage::PS:
                context->PSSetShaderResources(boundSlot, 1, &nullSRV);
                break;
            case DXShaderStage::HS:
                context->HSSetShaderResources(boundSlot, 1, &nullSRV);
                break;
            case DXShaderStage::DS:
                context->DSSetShaderResources(boundSlot, 1, &nullSRV);
                break;
            case DXShaderStage::GS:
                context->GSSetShaderResources(boundSlot, 1, &nullSRV);
                break;
            case DXShaderStage::CS:
                context->CSSetShaderResources(boundSlot, 1, &nullSRV);
                break;
            default:
                break;
            }
        }
    }

    bool BindUAV(ID3D11DeviceContext* context, unsigned int bindSlot) {
        if (m_uav != nullptr) {
            context->CSSetUnorderedAccessViews(bindSlot, 1, &m_uav, nullptr);
            return true;
        }
        return false;
    }

    void UnbindUAV(ID3D11DeviceContext* context, unsigned int boundSlot) {
        if (m_uav != nullptr) {
            ID3D11UnorderedAccessView* nullView = nullptr;
            context->CSSetUnorderedAccessViews(boundSlot, 1, &nullView,
                                               nullptr);
        }
    }

    ID3D11ShaderResourceView* SRV(uint64 i = 0) const { return m_srv; }
    ID3D11UnorderedAccessView* UAV(uint64 i = 0) const { return m_uav; }

    DXResource& operator=(const DXResource& r) {
        m_resource = r.GetResource();
        m_srv = r.GetShaderResourceView();
        m_uav = r.GetUnorderedAccessView();
    }

    HRESULT CreateSRV(ID3D11Device* device,
                      D3D11_SHADER_RESOURCE_VIEW_DESC* desc) {
        if (m_srv == nullptr) {
            HR(device->CreateShaderResourceView(m_resource, desc, &m_srv));
            return S_OK;
        }
        return S_FALSE;
    }

    HRESULT CreateUAV(ID3D11Device* device,
                      D3D11_UNORDERED_ACCESS_VIEW_DESC* desc) {
        if (m_uav == nullptr) {
            HR(device->CreateUnorderedAccessView(m_resource, desc, &m_uav));
            return S_OK;
        }
        return S_FALSE;
    }

  protected:
    ID3D11Resource* m_resource;
    ID3D11ShaderResourceView* m_srv;
    ID3D11UnorderedAccessView* m_uav;
    bool m_isMapped;
};
} // namespace Riley