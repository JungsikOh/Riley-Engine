#pragma once
#include "DXBuffer.h"

namespace Riley {
template <typename CBuffer> class DXConstantBuffer : public DXBuffer {
    static constexpr uint32 GetCBufferSize(uint32 buffer_size) {
        return (buffer_size + (64 - 1)) & ~(64 - 1);
    }

  public:
    DXConstantBuffer(ID3D11Device* device, bool dynamic = true);
    DXConstantBuffer(ID3D11Device* device, CBuffer const& initData,
                     bool dynamic = true);

    void Bind(ID3D11DeviceContext* context, DXShaderStage stage,
              uint32 slot) const;

  private:
    bool dynamic;
};

template <typename CBuffer>
DXConstantBuffer<CBuffer>::DXConstantBuffer(ID3D11Device* device,
                                            CBuffer const& initialdata,
                                            bool dynamic /*= true*/)
    : dynamic{dynamic} {
    DXBufferDesc desc{};
    desc.resourceUsage =
        dynamic ? DXResourceUsage::Dynamic : DXResourceUsage::Default;
    desc.size = GetCBufferSize(sizeof(CBuffer));
    desc.bindFlags = DXBindFlag::ConstantBuffer;
    desc.cpuAccess = dynamic ? DXCpuAccess::Write : DXCpuAccess::None;

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = (uint32)desc.size;
    bufferDesc.Usage = ConvertUsage(desc.resourceUsage);
    bufferDesc.BindFlags = ParseBindFlags(desc.bindFlags);
    bufferDesc.CPUAccessFlags = ParseCPUAccessFlags(desc.cpuAccess);
    bufferDesc.MiscFlags = ParseMiscFlags(desc.miscFlags);
    bufferDesc.StructureByteStride = desc.stride;

    D3D11_SUBRESOURCE_DATA init{};
    if (initData != nullptr) {
        init.pSysMem = initData;
        init.SysMemPitch = (uint32)desc.size;
        init.SysMemSlicePitch = 0;
    }

    device->CreateBuffer(&bufferDesc, &init,
                         reinterpret_cast<ID3D11Buffer**>(&m_resource));
}

template <typename CBuffer>
DXConstantBuffer<CBuffer>::DXConstantBuffer(ID3D11Device* device,
                                            bool dynamic /*= true*/)
    : dynamic{dynamic} {
    DXBufferDesc desc{};
    desc.resourceUsage =
        dynamic ? DXResourceUsage::Dynamic : DXResourceUsage::Default;
    desc.size = GetCBufferSize(sizeof(CBuffer));
    desc.bindFlags = DXBindFlag::ConstantBuffer;
    desc.cpuAccess = dynamic ? DXCpuAccess::Write : DXCpuAccess::None;

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = (uint32)desc.size;
    bufferDesc.Usage = ConvertUsage(desc.resourceUsage);
    bufferDesc.BindFlags = ParseBindFlags(desc.bindFlags);
    bufferDesc.CPUAccessFlags = ParseCPUAccessFlags(desc.cpuAccess);
    bufferDesc.MiscFlags = ParseMiscFlags(desc.miscFlags);
    bufferDesc.StructureByteStride = desc.stride;

    D3D11_SUBRESOURCE_DATA init{};
    if (initData != nullptr) {
        init.pSysMem = initData;
        init.SysMemPitch = (uint32)desc.size;
        init.SysMemSlicePitch = 0;
    }

    device->CreateBuffer(&bufferDesc, &init,
                         reinterpret_cast<ID3D11Buffer**>(&m_resource));
}

template <typename CBuffer>
void DXConstantBuffer<CBuffer>::Bind(ID3D11DeviceContext* context,
                                     DXShaderStage stage, uint32 slot) const {
    switch (stage) {
    case DXShaderStage::VS:
        context->VSSetConstantBuffers(
            slot, 1, reinterpret_cast<ID3D11Buffer**>(&m_resource));
        break;
    case DXShaderStage::PS:
        context->PSSetConstantBuffers(
            slot, 1, reinterpret_cast<ID3D11Buffer**>(&m_resource));
        break;
    case DXShaderStage::HS:
        context->HSSetConstantBuffers(
            slot, 1, reinterpret_cast<ID3D11Buffer**>(&m_resource));
        break;
    case DXShaderStage::DS:
        context->DSSetConstantBuffers(
            slot, 1, reinterpret_cast<ID3D11Buffer**>(&m_resource));
        break;
    case DXShaderStage::GS:
        context->GSSetConstantBuffers(
            slot, 1, reinterpret_cast<ID3D11Buffer**>(&m_resource));
        break;
    case DXShaderStage::CS:
        context->CSSetConstantBuffers(
            slot, 1, reinterpret_cast<ID3D11Buffer**>(&m_resource));
        break;
    }
}

} // namespace Riley