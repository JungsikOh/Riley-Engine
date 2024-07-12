#pragma once
#include "../Utilities/EnumUtil.h"
#include "../Core/CoreTypes.h"

namespace Riley {

enum DXSubresourceType : uint8 {
    DXSubresourceType_SRV,
    DXSubresourceType_UAV,
    DXSubresourceType_RTV,
    DXSubresourceType_DSV,
    DXSubresourceType_Invalid
};

enum class DXBindFlag : uint32 {
    None = 0,
    ShaderResource = 1 << 0,
    RenderTarget = 1 << 1,
    DepthStencil = 1 << 2,
    UnorderedAccess = 1 << 3,
    VertexBuffer = 1 << 4,
    IndexBuffer = 1 << 5,
    ConstantBuffer = 1 << 6
};
DEFINE_ENUM_BIT_OPERATORS(DXBindFlag);

inline constexpr uint32 ParseBindFlags(DXBindFlag flags) {
    uint32 result = 0;
    if (HasAnyFlag(flags, DXBindFlag::VertexBuffer))
        result |= D3D11_BIND_VERTEX_BUFFER;
    if (HasAnyFlag(flags, DXBindFlag::IndexBuffer))
        result |= D3D11_BIND_INDEX_BUFFER;
    if (HasAnyFlag(flags, DXBindFlag::ConstantBuffer))
        result |= D3D11_BIND_CONSTANT_BUFFER;
    if (HasAnyFlag(flags, DXBindFlag::ShaderResource))
        result |= D3D11_BIND_SHADER_RESOURCE;
    if (HasAnyFlag(flags, DXBindFlag::RenderTarget))
        result |= D3D11_BIND_RENDER_TARGET;
    if (HasAnyFlag(flags, DXBindFlag::DepthStencil))
        result |= D3D11_BIND_DEPTH_STENCIL;
    if (HasAnyFlag(flags, DXBindFlag::UnorderedAccess))
        result |= D3D11_BIND_UNORDERED_ACCESS;
    return result;
}

enum class DXResourceUsage : uint8 {
    Default,
    Immutable,
    Dynamic,
    Staging,
};

inline constexpr D3D11_USAGE ConvertUsage(DXResourceUsage value) {
    switch (value) {
    case DXResourceUsage::Default:
        return D3D11_USAGE_DEFAULT;
        break;
    case DXResourceUsage::Immutable:
        return D3D11_USAGE_IMMUTABLE;
        break;
    case DXResourceUsage::Dynamic:
        return D3D11_USAGE_DYNAMIC;
        break;
    case DXResourceUsage::Staging:
        return D3D11_USAGE_STAGING;
        break;
    default:
        break;
    }
    return D3D11_USAGE_DEFAULT;
}

enum class DXCpuAccess : uint8 {
    None = 0b00,
    Write = 0b01,
    Read = 0b10,
    ReadWrite = 0b11
};
DEFINE_ENUM_BIT_OPERATORS(DXCpuAccess);

inline constexpr uint32 ParseCPUAccessFlags(DXCpuAccess value) {
    uint32 result = 0;
    if (HasAnyFlag(value, DXCpuAccess::Write))
        result |= D3D11_CPU_ACCESS_WRITE;
    if (HasAnyFlag(value, DXCpuAccess::Read))
        result |= D3D11_CPU_ACCESS_READ;
    return result;
}

enum class DXTextureMiscFlag : uint32 {
    None = 0,
    TextureCube = 1 << 0,
    GenerateMips = 1 << 1
};
DEFINE_ENUM_BIT_OPERATORS(DXTextureMiscFlag);

inline constexpr uint32 ParseMiscFlags(DXTextureMiscFlag value) {
    uint32 result = 0;
    if (HasAnyFlag(value, DXTextureMiscFlag::TextureCube))
        result |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    if (HasAnyFlag(value, DXTextureMiscFlag::GenerateMips))
        result |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    return result;
}

enum class DXBufferMiscFlag : uint32 {
    None,
    IndirectArgs = 1 << 0,
    BufferRaw = 1 << 1,
    BufferStructured = 1 << 2
};
DEFINE_ENUM_BIT_OPERATORS(DXBufferMiscFlag);

inline constexpr uint32 ParseMiscFlags(DXBufferMiscFlag value) {
    uint32 result = 0;
    if (HasAnyFlag(value, DXBufferMiscFlag::IndirectArgs))
        result |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    if (HasAnyFlag(value, DXBufferMiscFlag::BufferRaw))
        result |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    if (HasAnyFlag(value, DXBufferMiscFlag::BufferStructured))
        result |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    return result;
}

enum class DXMapType : uint32 {
    Read = 1,
    Write = 2,
    ReadWrite = 3,
    WriteDiscard = 4,
    WriteNoOverwrite = 5
};
inline constexpr D3D11_MAP ConvertMapType(DXMapType value) {
    switch (value) {
    case DXMapType::Read:
        return D3D11_MAP_READ;
    case DXMapType::Write:
        return D3D11_MAP_WRITE;
    case DXMapType::ReadWrite:
        return D3D11_MAP_READ_WRITE;
    case DXMapType::WriteDiscard:
        return D3D11_MAP_WRITE_DISCARD;
    case DXMapType::WriteNoOverwrite:
        return D3D11_MAP_WRITE_NO_OVERWRITE;
    default:
        return D3D11_MAP_WRITE;
    }
    return D3D11_MAP_WRITE;
}

struct DXMappedSubresource {
    void* p_data;
    uint32 row_pitch;
    uint32 depth_pitch;
};
} // namespace adria