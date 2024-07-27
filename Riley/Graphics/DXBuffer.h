#pragma once
#include "DXFormat.h"
#include "DXResource.h"
#include "DXResourceCommon.h"

namespace Riley {

struct DXBufferDesc {
    uint64 size = 0;
    DXResourceUsage resourceUsage = DXResourceUsage::Default;
    DXCpuAccess cpuAccess = DXCpuAccess::None;
    DXBindFlag bindFlags = DXBindFlag::None;
    DXBufferMiscFlag miscFlags = DXBufferMiscFlag::None;
    uint32 stride = 0;
    DXFormat format = DXFormat::UNKNOWN;
    std::strong_ordering operator<=>(DXBufferDesc const& other) const = default;
};

static DXBufferDesc VertexBufferDesc(uint64 vertex_count, uint32 stride) {
    DXBufferDesc desc{};
    desc.bindFlags = DXBindFlag::VertexBuffer;
    desc.cpuAccess = DXCpuAccess::None;
    desc.resourceUsage = DXResourceUsage::Immutable;
    desc.size = vertex_count * stride;
    desc.stride = stride;
    desc.miscFlags = DXBufferMiscFlag::None;
    return desc;
}

static DXBufferDesc IndexBufferDesc(uint64 index_count, bool small_indices) {
    DXBufferDesc desc{};
    desc.bindFlags = DXBindFlag::IndexBuffer;
    desc.cpuAccess = DXCpuAccess::None;
    desc.resourceUsage = DXResourceUsage::Immutable;
    desc.stride = small_indices ? 2 : 4;
    desc.size = index_count * desc.stride;
    desc.miscFlags = DXBufferMiscFlag::None;
    desc.format = small_indices ? DXFormat::R16_UINT : DXFormat::R32_UINT;
    return desc;
}

class DXBuffer : public DXResource {
  public:
    DXBuffer() = default;
    DXBuffer(ID3D11Device* _device, DXBufferDesc const& _desc,
             void const* initData = nullptr)
        : m_desc(_desc) {
        D3D11_BUFFER_DESC desc{};
        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth = (uint32)_desc.size;
        desc.Usage = ConvertUsage(_desc.resourceUsage);
        desc.BindFlags = ParseBindFlags(_desc.bindFlags);
        desc.CPUAccessFlags = ParseCPUAccessFlags(_desc.cpuAccess);
        desc.MiscFlags = ParseMiscFlags(_desc.miscFlags);
        desc.StructureByteStride = _desc.stride;

        D3D11_SUBRESOURCE_DATA init{};
        if (initData != nullptr) {
            init.pSysMem = initData;
            init.SysMemPitch = (uint32)_desc.size;
            init.SysMemSlicePitch = 0;
        }
        HR(_device->CreateBuffer(&desc, initData == nullptr ? nullptr : &init,
                                reinterpret_cast<ID3D11Buffer**>(&m_resource)));
    }

    DXBuffer(DXBuffer const&) = delete;
    DXBuffer& operator=(DXBuffer const&) = delete;
    virtual ~DXBuffer() = default;

    // return ID3D11Buffer*
    ID3D11Buffer* GetNative() const {
        return reinterpret_cast<ID3D11Buffer*>(m_resource);
    }

    // return m_desc.size / m_desc.stride
    uint32 GetCount() const {
        assert(m_desc.stride != 0);
        return static_cast<uint32>(m_desc.size / m_desc.stride);
    }

    // return desc
    DXBufferDesc const& GetDesc() const { return m_desc; }

    // if buffer Usage is Dynamic, Update the Buffer by srcData.
    template <typename T_DATA>
    void Update(ID3D11DeviceContext* context, const T_DATA& srcData,
                uint64 dataSize) {
        if (m_desc.resourceUsage == DXResourceUsage::Dynamic) {
            memcpy(Map(context), &srcData, dataSize); // data º¹»ç
            UnMap(context);
        }
    }

    template <typename T> void Update(T const& src_data) {
        Update(&src_data, sizeof(T));
    }

  protected:
    DXBufferDesc m_desc;
};

static void BindIndexBuffer(ID3D11DeviceContext* context, DXBuffer* ib,
                            uint32 offset = 0) {
    context->IASetIndexBuffer(ib->GetNative(),
                             ConvertDXFormat(ib->GetDesc().format), offset);
}

static void BindVertexBuffer(ID3D11DeviceContext* context, DXBuffer* vb,
                             uint32 slot = 0, uint32 offset = 0) {
    ID3D11Buffer* const vbs = vb->GetNative();
    uint32 strides = vb->GetDesc().stride;
    context->IASetVertexBuffers(slot, 1, &vbs, &strides, &offset);
}

static void BindNullVertexBuffer(ID3D11DeviceContext* context) {
    static ID3D11Buffer* vb = nullptr;
    static const UINT stride = 0;
    static const UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
}

} // namespace Riley