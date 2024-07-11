#pragma once
#include <dxgiformat.h>

namespace Riley {
enum class DXFormat {
    UNKNOWN,
    R32G32B32A32_FLOAT,
    R32G32B32A32_UINT,
    R32G32B32A32_SINT,
    R32G32B32_FLOAT,
    R32G32B32_UINT,
    R32G32B32_SINT,
    R16G16B16A16_FLOAT,
    R16G16B16A16_UNORM,
    R16G16B16A16_UINT,
    R16G16B16A16_SNORM,
    R16G16B16A16_SINT,
    R32G32_FLOAT,
    R32G32_UINT,
    R32G32_SINT,
    R32G8X24_TYPELESS,
    D32_FLOAT_S8X24_UINT,
    R10G10B10A2_UNORM,
    R10G10B10A2_UINT,
    R11G11B10_FLOAT,
    R8G8B8A8_UNORM,
    R8G8B8A8_UNORM_SRGB,
    R8G8B8A8_UINT,
    R8G8B8A8_SNORM,
    R8G8B8A8_SINT,
    B8G8R8A8_UNORM,
    B8G8R8A8_UNORM_SRGB,
    R16G16_FLOAT,
    R16G16_UNORM,
    R16G16_UINT,
    R16G16_SNORM,
    R16G16_SINT,
    R32_TYPELESS,
    D32_FLOAT,
    R32_FLOAT,
    R32_UINT,
    R32_SINT,
    R24G8_TYPELESS,
    D24_UNORM_S8_UINT,
    R8G8_UNORM,
    R8G8_UINT,
    R8G8_SNORM,
    R8G8_SINT,
    R16_TYPELESS,
    R16_FLOAT,
    D16_UNORM,
    R16_UNORM,
    R16_UINT,
    R16_SNORM,
    R16_SINT,
    R8_UNORM,
    R8_UINT,
    R8_SNORM,
    R8_SINT,
    BC1_UNORM,
    BC1_UNORM_SRGB,
    BC2_UNORM,
    BC2_UNORM_SRGB,
    BC3_UNORM,
    BC3_UNORM_SRGB,
    BC4_UNORM,
    BC4_SNORM,
    BC5_UNORM,
    BC5_SNORM,
    BC6H_UF16,
    BC6H_SF16,
    BC7_UNORM,
    BC7_UNORM_SRGB
};
inline constexpr DXGI_FORMAT ConvertDXFormat(DXFormat _format) {
    switch (_format) {
    case DXFormat::UNKNOWN:
        return DXGI_FORMAT_UNKNOWN;

    case DXFormat::R32G32B32A32_FLOAT:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case DXFormat::R32G32B32A32_UINT:
        return DXGI_FORMAT_R32G32B32A32_UINT;

    case DXFormat::R32G32B32A32_SINT:
        return DXGI_FORMAT_R32G32B32A32_SINT;

    case DXFormat::R32G32B32_FLOAT:
        return DXGI_FORMAT_R32G32B32_FLOAT;

    case DXFormat::R32G32B32_UINT:
        return DXGI_FORMAT_R32G32B32_UINT;

    case DXFormat::R32G32B32_SINT:
        return DXGI_FORMAT_R32G32B32_SINT;

    case DXFormat::R16G16B16A16_FLOAT:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case DXFormat::R16G16B16A16_UNORM:
        return DXGI_FORMAT_R16G16B16A16_UNORM;

    case DXFormat::R16G16B16A16_UINT:
        return DXGI_FORMAT_R16G16B16A16_UINT;

    case DXFormat::R16G16B16A16_SNORM:
        return DXGI_FORMAT_R16G16B16A16_SNORM;

    case DXFormat::R16G16B16A16_SINT:
        return DXGI_FORMAT_R16G16B16A16_SINT;

    case DXFormat::R32G32_FLOAT:
        return DXGI_FORMAT_R32G32_FLOAT;

    case DXFormat::R32G32_UINT:
        return DXGI_FORMAT_R32G32_UINT;

    case DXFormat::R32G32_SINT:
        return DXGI_FORMAT_R32G32_SINT;

    case DXFormat::R32G8X24_TYPELESS:
        return DXGI_FORMAT_R32G8X24_TYPELESS;

    case DXFormat::D32_FLOAT_S8X24_UINT:
        return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    case DXFormat::R10G10B10A2_UNORM:
        return DXGI_FORMAT_R10G10B10A2_UNORM;

    case DXFormat::R10G10B10A2_UINT:
        return DXGI_FORMAT_R10G10B10A2_UINT;

    case DXFormat::R11G11B10_FLOAT:
        return DXGI_FORMAT_R11G11B10_FLOAT;

    case DXFormat::R8G8B8A8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM;

    case DXFormat::R8G8B8A8_UNORM_SRGB:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    case DXFormat::R8G8B8A8_UINT:
        return DXGI_FORMAT_R8G8B8A8_UINT;

    case DXFormat::R8G8B8A8_SNORM:
        return DXGI_FORMAT_R8G8B8A8_SNORM;

    case DXFormat::R8G8B8A8_SINT:
        return DXGI_FORMAT_R8G8B8A8_SINT;

    case DXFormat::R16G16_FLOAT:
        return DXGI_FORMAT_R16G16_FLOAT;

    case DXFormat::R16G16_UNORM:
        return DXGI_FORMAT_R16G16_UNORM;

    case DXFormat::R16G16_UINT:
        return DXGI_FORMAT_R16G16_UINT;

    case DXFormat::R16G16_SNORM:
        return DXGI_FORMAT_R16G16_SNORM;

    case DXFormat::R16G16_SINT:
        return DXGI_FORMAT_R16G16_SINT;

    case DXFormat::R32_TYPELESS:
        return DXGI_FORMAT_R32_TYPELESS;

    case DXFormat::D32_FLOAT:
        return DXGI_FORMAT_D32_FLOAT;

    case DXFormat::R32_FLOAT:
        return DXGI_FORMAT_R32_FLOAT;

    case DXFormat::R32_UINT:
        return DXGI_FORMAT_R32_UINT;

    case DXFormat::R32_SINT:
        return DXGI_FORMAT_R32_SINT;

    case DXFormat::R8G8_UNORM:
        return DXGI_FORMAT_R8G8_UNORM;

    case DXFormat::R8G8_UINT:
        return DXGI_FORMAT_R8G8_UINT;

    case DXFormat::R8G8_SNORM:
        return DXGI_FORMAT_R8G8_SNORM;

    case DXFormat::R8G8_SINT:
        return DXGI_FORMAT_R8G8_SINT;

    case DXFormat::R16_TYPELESS:
        return DXGI_FORMAT_R16_TYPELESS;

    case DXFormat::R16_FLOAT:
        return DXGI_FORMAT_R16_FLOAT;

    case DXFormat::D16_UNORM:
        return DXGI_FORMAT_D16_UNORM;

    case DXFormat::R16_UNORM:
        return DXGI_FORMAT_R16_UNORM;

    case DXFormat::R16_UINT:
        return DXGI_FORMAT_R16_UINT;

    case DXFormat::R16_SNORM:
        return DXGI_FORMAT_R16_SNORM;

    case DXFormat::R16_SINT:
        return DXGI_FORMAT_R16_SINT;

    case DXFormat::R8_UNORM:
        return DXGI_FORMAT_R8_UNORM;

    case DXFormat::R8_UINT:
        return DXGI_FORMAT_R8_UINT;

    case DXFormat::R8_SNORM:
        return DXGI_FORMAT_R8_SNORM;

    case DXFormat::R8_SINT:
        return DXGI_FORMAT_R8_SINT;

    case DXFormat::BC1_UNORM:
        return DXGI_FORMAT_BC1_UNORM;

    case DXFormat::BC1_UNORM_SRGB:
        return DXGI_FORMAT_BC1_UNORM_SRGB;

    case DXFormat::BC2_UNORM:
        return DXGI_FORMAT_BC2_UNORM;

    case DXFormat::BC2_UNORM_SRGB:
        return DXGI_FORMAT_BC2_UNORM_SRGB;

    case DXFormat::BC3_UNORM:
        return DXGI_FORMAT_BC3_UNORM;

    case DXFormat::BC3_UNORM_SRGB:
        return DXGI_FORMAT_BC3_UNORM_SRGB;

    case DXFormat::BC4_UNORM:
        return DXGI_FORMAT_BC4_UNORM;

    case DXFormat::BC4_SNORM:
        return DXGI_FORMAT_BC4_SNORM;

    case DXFormat::BC5_UNORM:
        return DXGI_FORMAT_BC5_UNORM;

    case DXFormat::BC5_SNORM:
        return DXGI_FORMAT_BC5_SNORM;

    case DXFormat::B8G8R8A8_UNORM:
        return DXGI_FORMAT_B8G8R8A8_UNORM;

    case DXFormat::B8G8R8A8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case DXFormat::BC6H_UF16:
        return DXGI_FORMAT_BC6H_UF16;

    case DXFormat::BC6H_SF16:
        return DXGI_FORMAT_BC6H_SF16;

    case DXFormat::BC7_UNORM:
        return DXGI_FORMAT_BC7_UNORM;

    case DXFormat::BC7_UNORM_SRGB:
        return DXGI_FORMAT_BC7_UNORM_SRGB;
    }
    return DXGI_FORMAT_UNKNOWN;
}
inline constexpr DXFormat ConvertDXGIFormat(DXGI_FORMAT _format) {
    switch (_format) {
    case DXGI_FORMAT_UNKNOWN:
        return DXFormat::UNKNOWN;

    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return DXFormat::R32G32B32A32_FLOAT;

    case DXGI_FORMAT_R32G32B32A32_UINT:
        return DXFormat::R32G32B32A32_UINT;

    case DXGI_FORMAT_R32G32B32A32_SINT:
        return DXFormat::R32G32B32A32_SINT;

    case DXGI_FORMAT_R32G32B32_FLOAT:
        return DXFormat::R32G32B32_FLOAT;

    case DXGI_FORMAT_R32G32B32_UINT:
        return DXFormat::R32G32B32_UINT;

    case DXGI_FORMAT_R32G32B32_SINT:
        return DXFormat::R32G32B32_SINT;

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return DXFormat::R16G16B16A16_FLOAT;

    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return DXFormat::R16G16B16A16_UNORM;

    case DXGI_FORMAT_R16G16B16A16_UINT:
        return DXFormat::R16G16B16A16_UINT;

    case DXGI_FORMAT_R16G16B16A16_SNORM:
        return DXFormat::R16G16B16A16_SNORM;

    case DXGI_FORMAT_R16G16B16A16_SINT:
        return DXFormat::R16G16B16A16_SINT;

    case DXGI_FORMAT_R32G32_FLOAT:
        return DXFormat::R32G32_FLOAT;

    case DXGI_FORMAT_R32G32_UINT:
        return DXFormat::R32G32_UINT;

    case DXGI_FORMAT_R32G32_SINT:
        return DXFormat::R32G32_SINT;

    case DXGI_FORMAT_R32G8X24_TYPELESS:
        return DXFormat::R32G8X24_TYPELESS;

    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return DXFormat::D32_FLOAT_S8X24_UINT;

    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return DXFormat::R10G10B10A2_UNORM;

    case DXGI_FORMAT_R10G10B10A2_UINT:
        return DXFormat::R10G10B10A2_UINT;

    case DXGI_FORMAT_R11G11B10_FLOAT:
        return DXFormat::R11G11B10_FLOAT;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return DXFormat::R8G8B8A8_UNORM;

    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return DXFormat::R8G8B8A8_UNORM_SRGB;

    case DXGI_FORMAT_R8G8B8A8_UINT:
        return DXFormat::R8G8B8A8_UINT;

    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return DXFormat::R8G8B8A8_SNORM;

    case DXGI_FORMAT_R8G8B8A8_SINT:
        return DXFormat::R8G8B8A8_SINT;

    case DXGI_FORMAT_R16G16_FLOAT:
        return DXFormat::R16G16_FLOAT;

    case DXGI_FORMAT_R16G16_UNORM:
        return DXFormat::R16G16_UNORM;

    case DXGI_FORMAT_R16G16_UINT:
        return DXFormat::R16G16_UINT;

    case DXGI_FORMAT_R16G16_SNORM:
        return DXFormat::R16G16_SNORM;

    case DXGI_FORMAT_R16G16_SINT:
        return DXFormat::R16G16_SINT;

    case DXGI_FORMAT_R32_TYPELESS:
        return DXFormat::R32_TYPELESS;

    case DXGI_FORMAT_D32_FLOAT:
        return DXFormat::D32_FLOAT;

    case DXGI_FORMAT_R32_FLOAT:
        return DXFormat::R32_FLOAT;

    case DXGI_FORMAT_R32_UINT:
        return DXFormat::R32_UINT;

    case DXGI_FORMAT_R32_SINT:
        return DXFormat::R32_SINT;

    case DXGI_FORMAT_R8G8_UNORM:
        return DXFormat::R8G8_UNORM;

    case DXGI_FORMAT_R8G8_UINT:
        return DXFormat::R8G8_UINT;

    case DXGI_FORMAT_R8G8_SNORM:
        return DXFormat::R8G8_SNORM;

    case DXGI_FORMAT_R8G8_SINT:
        return DXFormat::R8G8_SINT;

    case DXGI_FORMAT_R16_TYPELESS:
        return DXFormat::R16_TYPELESS;

    case DXGI_FORMAT_R16_FLOAT:
        return DXFormat::R16_FLOAT;

    case DXGI_FORMAT_D16_UNORM:
        return DXFormat::D16_UNORM;

    case DXGI_FORMAT_R16_UNORM:
        return DXFormat::R16_UNORM;

    case DXGI_FORMAT_R16_UINT:
        return DXFormat::R16_UINT;

    case DXGI_FORMAT_R16_SNORM:
        return DXFormat::R16_SNORM;

    case DXGI_FORMAT_R16_SINT:
        return DXFormat::R16_SINT;

    case DXGI_FORMAT_R8_UNORM:
        return DXFormat::R8_UNORM;

    case DXGI_FORMAT_R8_UINT:
        return DXFormat::R8_UINT;

    case DXGI_FORMAT_R8_SNORM:
        return DXFormat::R8_SNORM;

    case DXGI_FORMAT_R8_SINT:
        return DXFormat::R8_SINT;

    case DXGI_FORMAT_BC1_UNORM:
        return DXFormat::BC1_UNORM;

    case DXGI_FORMAT_BC1_UNORM_SRGB:
        return DXFormat::BC1_UNORM_SRGB;

    case DXGI_FORMAT_BC2_UNORM:
        return DXFormat::BC2_UNORM;

    case DXGI_FORMAT_BC2_UNORM_SRGB:
        return DXFormat::BC2_UNORM_SRGB;

    case DXGI_FORMAT_BC3_UNORM:
        return DXFormat::BC3_UNORM;

    case DXGI_FORMAT_BC3_UNORM_SRGB:
        return DXFormat::BC3_UNORM_SRGB;

    case DXGI_FORMAT_BC4_UNORM:
        return DXFormat::BC4_UNORM;

    case DXGI_FORMAT_BC4_SNORM:
        return DXFormat::BC4_SNORM;

    case DXGI_FORMAT_BC5_UNORM:
        return DXFormat::BC5_UNORM;

    case DXGI_FORMAT_BC5_SNORM:
        return DXFormat::BC5_SNORM;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return DXFormat::B8G8R8A8_UNORM;

    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return DXFormat::B8G8R8A8_UNORM_SRGB;

    case DXGI_FORMAT_BC6H_UF16:
        return DXFormat::BC6H_UF16;

    case DXGI_FORMAT_BC6H_SF16:
        return DXFormat::BC6H_SF16;

    case DXGI_FORMAT_BC7_UNORM:
        return DXFormat::BC7_UNORM;

    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return DXFormat::BC7_UNORM_SRGB;
    }
    return DXFormat::UNKNOWN;
}
inline constexpr uint32 GetDXFormatStride(DXFormat _format) {
    DXGI_FORMAT format = ConvertDXFormat(_format);
    switch (format) {
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC4_UNORM:
        return 8u;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return 16u;
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 12u;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
        return 8u;
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return 8u;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return 4u;
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
        return 2u;
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
        return 1u;
    default:
        break;
    }
    return 16u;
}
inline constexpr uint32 GetFormatStride(DXGI_FORMAT format) {
    switch (format) {
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC4_UNORM:
        return 8u;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return 16u;
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 12u;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
        return 8u;
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return 8u;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return 4u;
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
        return 2u;
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
        return 1u;
    default:
        break;
    }

    return 16u;
}
} // namespace adria