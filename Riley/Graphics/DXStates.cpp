#include "DXStates.h"
#include "../Core/Rendering.h"
#include "../Utilities/EnumUtil.h"

namespace Riley {

////////////////////////////////////
// Convert DXState -> D3D11 State //
////////////////////////////////////

constexpr D3D11_FILL_MODE ConvertFillMode(DXFillMode value) {
    switch (value) {
    case DXFillMode::Wireframe:
        return D3D11_FILL_WIREFRAME;
        break;
    case DXFillMode::Solid:
        return D3D11_FILL_SOLID;
        break;
    default:
        break;
    }
    return D3D11_FILL_WIREFRAME;
}
constexpr D3D11_CULL_MODE ConvertCullMode(DXCullMode value) {
    switch (value) {
    case DXCullMode::None:
        return D3D11_CULL_NONE;
        break;
    case DXCullMode::Front:
        return D3D11_CULL_FRONT;
        break;
    case DXCullMode::Back:
        return D3D11_CULL_BACK;
        break;
    default:
        break;
    }
    return D3D11_CULL_NONE;
}

constexpr D3D11_DEPTH_WRITE_MASK ConvertDepthWriteMask(DXDepthWriteMask value) {
    switch (value) {
    case DXDepthWriteMask::Zero:
        return D3D11_DEPTH_WRITE_MASK_ZERO;
        break;
    case DXDepthWriteMask::All:
        return D3D11_DEPTH_WRITE_MASK_ALL;
        break;
    default:
        break;
    }
    return D3D11_DEPTH_WRITE_MASK_ZERO;
}

constexpr D3D11_STENCIL_OP ConvertStencilOp(DXStencilOp value) {
    switch (value) {
    case DXStencilOp::Keep:
        return D3D11_STENCIL_OP_KEEP;
        break;
    case DXStencilOp::Zero:
        return D3D11_STENCIL_OP_ZERO;
        break;
    case DXStencilOp::Replace:
        return D3D11_STENCIL_OP_REPLACE;
        break;
    case DXStencilOp::IncrSat:
        return D3D11_STENCIL_OP_INCR_SAT;
        break;
    case DXStencilOp::DecrSat:
        return D3D11_STENCIL_OP_DECR_SAT;
        break;
    case DXStencilOp::Invert:
        return D3D11_STENCIL_OP_INVERT;
        break;
    case DXStencilOp::Incr:
        return D3D11_STENCIL_OP_INCR;
        break;
    case DXStencilOp::Decr:
        return D3D11_STENCIL_OP_DECR;
        break;
    default:
        break;
    }
    return D3D11_STENCIL_OP_KEEP;
}

constexpr D3D11_BLEND ConvertBlend(DXBlend value) {
    switch (value) {
    case DXBlend::Zero:
        return D3D11_BLEND_ZERO;
    case DXBlend::One:
        return D3D11_BLEND_ONE;
    case DXBlend::SrcColor:
        return D3D11_BLEND_SRC_COLOR;
    case DXBlend::InvSrcColor:
        return D3D11_BLEND_INV_SRC_COLOR;
    case DXBlend::SrcAlpha:
        return D3D11_BLEND_SRC_ALPHA;
    case DXBlend::InvSrcAlpha:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case DXBlend::DstAlpha:
        return D3D11_BLEND_DEST_ALPHA;
    case DXBlend::InvDstAlpha:
        return D3D11_BLEND_INV_DEST_ALPHA;
    case DXBlend::DstColor:
        return D3D11_BLEND_DEST_COLOR;
    case DXBlend::InvDstColor:
        return D3D11_BLEND_INV_DEST_COLOR;
    case DXBlend::SrcAlphaSat:
        return D3D11_BLEND_SRC_ALPHA_SAT;
    case DXBlend::BlendFactor:
        return D3D11_BLEND_BLEND_FACTOR;
    case DXBlend::InvBlendFactor:
        return D3D11_BLEND_INV_BLEND_FACTOR;
    case DXBlend::Src1Color:
        return D3D11_BLEND_SRC1_COLOR;
    case DXBlend::InvSrc1Color:
        return D3D11_BLEND_INV_SRC1_COLOR;
    case DXBlend::Src1Alpha:
        return D3D11_BLEND_SRC1_ALPHA;
    case DXBlend::InvSrc1Alpha:
        return D3D11_BLEND_INV_SRC1_ALPHA;
    default:
        return D3D11_BLEND_ZERO;
    }
}

constexpr D3D11_BLEND ConvertAlphaBlend(DXBlend value) {
    switch (value) {
    case DXBlend::SrcColor:
        return D3D11_BLEND_SRC_ALPHA;
    case DXBlend::InvSrcColor:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case DXBlend::DstColor:
        return D3D11_BLEND_DEST_ALPHA;
    case DXBlend::InvDstColor:
        return D3D11_BLEND_INV_DEST_ALPHA;
    case DXBlend::Src1Color:
        return D3D11_BLEND_SRC1_ALPHA;
    case DXBlend::InvSrc1Color:
        return D3D11_BLEND_INV_SRC1_ALPHA;
    default:
        return ConvertBlend(value);
    }
}

constexpr D3D11_BLEND_OP ConvertBlendOp(DXBlendOp value) {
    switch (value) {
    case DXBlendOp::Add:
        return D3D11_BLEND_OP_ADD;
    case DXBlendOp::Subtract:
        return D3D11_BLEND_OP_SUBTRACT;
    case DXBlendOp::RevSubtract:
        return D3D11_BLEND_OP_REV_SUBTRACT;
    case DXBlendOp::Min:
        return D3D11_BLEND_OP_MIN;
    case DXBlendOp::Max:
        return D3D11_BLEND_OP_MAX;
    default:
        return D3D11_BLEND_OP_ADD;
    }
}

constexpr uint32 ParseColorWriteMask(DXColorWrite value) {
    uint32 _flag = 0;
    if (value == DXColorWrite::EnableAll) {
        return D3D11_COLOR_WRITE_ENABLE_ALL;
    } else {
        if (HasAnyFlag(value, DXColorWrite::EnableRed))
            _flag |= D3D11_COLOR_WRITE_ENABLE_RED;
        if (HasAnyFlag(value, DXColorWrite::EnableGreen))
            _flag |= D3D11_COLOR_WRITE_ENABLE_GREEN;
        if (HasAnyFlag(value, DXColorWrite::EnableBlue))
            _flag |= D3D11_COLOR_WRITE_ENABLE_BLUE;
        if (HasAnyFlag(value, DXColorWrite::EnableAlpha))
            _flag |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
    }
    return _flag;
}

constexpr D3D11_COMPARISON_FUNC ConvertComparisonFunc(DXComparisonFunc value) {
    switch (value) {
    case DXComparisonFunc::Never:
        return D3D11_COMPARISON_NEVER;
        break;
    case DXComparisonFunc::Less:
        return D3D11_COMPARISON_LESS;
        break;
    case DXComparisonFunc::Equal:
        return D3D11_COMPARISON_EQUAL;
        break;
    case DXComparisonFunc::LessEqual:
        return D3D11_COMPARISON_LESS_EQUAL;
        break;
    case DXComparisonFunc::Greater:
        return D3D11_COMPARISON_GREATER;
        break;
    case DXComparisonFunc::NotEqual:
        return D3D11_COMPARISON_NOT_EQUAL;
        break;
    case DXComparisonFunc::GreaterEqual:
        return D3D11_COMPARISON_GREATER_EQUAL;
        break;
    case DXComparisonFunc::Always:
        return D3D11_COMPARISON_ALWAYS;
        break;
    default:
        break;
    }
    return D3D11_COMPARISON_NEVER;
}

constexpr D3D11_FILTER ConvertFilter(DXFilter filter) {
    switch (filter) {
    case DXFilter::MIN_MAG_MIP_POINT:
        return D3D11_FILTER_MIN_MAG_MIP_POINT;
    case DXFilter::MIN_MAG_POINT_MIP_LINEAR:
        return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    case DXFilter::MIN_POINT_MAG_LINEAR_MIP_POINT:
        return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    case DXFilter::MIN_POINT_MAG_MIP_LINEAR:
        return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
    case DXFilter::MIN_LINEAR_MAG_MIP_POINT:
        return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    case DXFilter::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
        return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case DXFilter::MIN_MAG_LINEAR_MIP_POINT:
        return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    case DXFilter::MIN_MAG_MIP_LINEAR:
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    case DXFilter::ANISOTROPIC:
        return D3D11_FILTER_ANISOTROPIC;
    case DXFilter::COMPARISON_MIN_MAG_MIP_POINT:
        return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    case DXFilter::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
        return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
    case DXFilter::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
        return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
    case DXFilter::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
        return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
    case DXFilter::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
        return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
    case DXFilter::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
        return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case DXFilter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
        return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    case DXFilter::COMPARISON_MIN_MAG_MIP_LINEAR:
        return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    case DXFilter::COMPARISON_ANISOTROPIC:
        return D3D11_FILTER_COMPARISON_ANISOTROPIC;
    case DXFilter::MINIMUM_MIN_MAG_MIP_POINT:
        return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
    case DXFilter::MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
        return D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
    case DXFilter::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
        return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
    case DXFilter::MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
        return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
    case DXFilter::MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
        return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
    case DXFilter::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
        return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case DXFilter::MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
        return D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
    case DXFilter::MINIMUM_MIN_MAG_MIP_LINEAR:
        return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
    case DXFilter::MINIMUM_ANISOTROPIC:
        return D3D11_FILTER_MINIMUM_ANISOTROPIC;
    case DXFilter::MAXIMUM_MIN_MAG_MIP_POINT:
        return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
    case DXFilter::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
        return D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
    case DXFilter::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
        return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
    case DXFilter::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
        return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
    case DXFilter::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
        return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
    case DXFilter::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
        return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case DXFilter::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
        return D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
    case DXFilter::MAXIMUM_MIN_MAG_MIP_LINEAR:
        return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
    case DXFilter::MAXIMUM_ANISOTROPIC:
        return D3D11_FILTER_MAXIMUM_ANISOTROPIC;
    }
    return D3D11_FILTER_MAXIMUM_ANISOTROPIC;
}

constexpr D3D11_TEXTURE_ADDRESS_MODE
ConvertTextureAddressMode(DXTextureAddressMode mode) {
    switch (mode) {
    case DXTextureAddressMode::Wrap:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    case DXTextureAddressMode::Mirror:
        return D3D11_TEXTURE_ADDRESS_MIRROR;
    case DXTextureAddressMode::Clamp:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    case DXTextureAddressMode::Border:
        return D3D11_TEXTURE_ADDRESS_BORDER;
    }
    return D3D11_TEXTURE_ADDRESS_CLAMP;
}

D3D11_RASTERIZER_DESC ConvertRasterizerDesc(DXRasterizerStateDesc _rs) {
    D3D11_RASTERIZER_DESC rs{};
    rs.FillMode = ConvertFillMode(_rs.fill_mode);
    rs.CullMode = ConvertCullMode(_rs.cull_mode);
    rs.FrontCounterClockwise = _rs.front_counter_clockwise;
    rs.DepthBias = _rs.depth_bias;
    rs.DepthBiasClamp = _rs.depth_bias_clamp;
    rs.SlopeScaledDepthBias = _rs.slope_scaled_depth_bias;
    rs.DepthClipEnable = _rs.depth_clip_enable;
    rs.MultisampleEnable = _rs.multisample_enable;
    rs.AntialiasedLineEnable = _rs.antialiased_line_enable;
    return rs;
}

D3D11_DEPTH_STENCIL_DESC
ConvertDepthStencilDesc(DXDepthStencilStateDesc _dss) {
    D3D11_DEPTH_STENCIL_DESC dss{};
    dss.DepthEnable = _dss.depth_enable;
    dss.DepthWriteMask = ConvertDepthWriteMask(_dss.depth_write_mask);
    dss.DepthFunc = ConvertComparisonFunc(_dss.depth_func);
    dss.StencilEnable = _dss.stencil_enable;
    dss.StencilReadMask = _dss.stencil_read_mask;
    dss.StencilWriteMask = _dss.stencil_write_mask;
    dss.FrontFace.StencilDepthFailOp =
        ConvertStencilOp(_dss.front_face.stencil_depth_fail_op);
    dss.FrontFace.StencilFailOp =
        ConvertStencilOp(_dss.front_face.stencil_fail_op);
    dss.FrontFace.StencilFunc =
        ConvertComparisonFunc(_dss.front_face.stencil_func);
    dss.FrontFace.StencilPassOp =
        ConvertStencilOp(_dss.front_face.stencil_pass_op);
    dss.BackFace.StencilDepthFailOp =
        ConvertStencilOp(_dss.back_face.stencil_depth_fail_op);
    dss.BackFace.StencilFailOp =
        ConvertStencilOp(_dss.back_face.stencil_fail_op);
    dss.BackFace.StencilFunc =
        ConvertComparisonFunc(_dss.back_face.stencil_func);
    dss.BackFace.StencilPassOp =
        ConvertStencilOp(_dss.back_face.stencil_pass_op);
    return dss;
}

D3D11_BLEND_DESC ConvertBlendDesc(DXBlendStateDesc _bd) {
    D3D11_BLEND_DESC bd{};
    bd.AlphaToCoverageEnable = _bd.alpha_to_coverage_enable;
    bd.IndependentBlendEnable = _bd.independent_blend_enable;
    for (int32 i = 0; i < 8; ++i) {
        bd.RenderTarget[i].BlendEnable = _bd.render_target[i].blend_enable;
        bd.RenderTarget[i].SrcBlend =
            ConvertBlend(_bd.render_target[i].src_blend);
        bd.RenderTarget[i].DestBlend =
            ConvertBlend(_bd.render_target[i].dest_blend);
        bd.RenderTarget[i].BlendOp =
            ConvertBlendOp(_bd.render_target[i].blend_op);
        bd.RenderTarget[i].SrcBlendAlpha =
            ConvertAlphaBlend(_bd.render_target[i].src_blend_alpha);
        bd.RenderTarget[i].DestBlendAlpha =
            ConvertAlphaBlend(_bd.render_target[i].dest_blend_alpha);
        bd.RenderTarget[i].BlendOpAlpha =
            ConvertBlendOp(_bd.render_target[i].blend_op_alpha);
        bd.RenderTarget[i].RenderTargetWriteMask =
            ParseColorWriteMask(_bd.render_target[i].render_target_write_mask);
    }
    return bd;
}

D3D11_SAMPLER_DESC ConvertSamplerDesc(DXSamplerDesc const& desc) {
    D3D11_SAMPLER_DESC d3d11_desc{};
    d3d11_desc.Filter = ConvertFilter(desc.filter);
    d3d11_desc.AddressU = ConvertTextureAddressMode(desc.addressU);
    d3d11_desc.AddressV = ConvertTextureAddressMode(desc.addressV);
    d3d11_desc.AddressW = ConvertTextureAddressMode(desc.addressW);
    memcpy(d3d11_desc.BorderColor, desc.borderColor, sizeof(float[4]));
    d3d11_desc.ComparisonFunc = ConvertComparisonFunc(desc.comparisonFunc);
    d3d11_desc.MinLOD = desc.minLod;
    d3d11_desc.MaxLOD = desc.maxLod;
    d3d11_desc.MaxAnisotropy = desc.maxAnisotropy;
    d3d11_desc.MipLODBias = desc.mipLodBias;
    return d3d11_desc;
}

///////////////////
// Class DXState //
///////////////////

DXRasterizerState::DXRasterizerState(ID3D11Device* device,
                                     DXRasterizerStateDesc const& desc) {
    D3D11_RASTERIZER_DESC d3d11_desc = ConvertRasterizerDesc(desc);
    HR(device->CreateRasterizerState(&d3d11_desc, &rasterizerState));
}

DXDepthStencilState::DXDepthStencilState(ID3D11Device* device,
                                         DXDepthStencilStateDesc const& desc) {
    D3D11_DEPTH_STENCIL_DESC d3d11_desc = ConvertDepthStencilDesc(desc);
    HR(device->CreateDepthStencilState(&d3d11_desc, &depthStencilState));
}

DXBlendState::DXBlendState(ID3D11Device* device, DXBlendStateDesc const& desc) {
    D3D11_BLEND_DESC d3d11_desc = ConvertBlendDesc(desc);
    HR(device->CreateBlendState(&d3d11_desc, &blendState));
}

DXSampler::DXSampler(ID3D11Device* device, DXSamplerDesc const& desc) {
    D3D11_SAMPLER_DESC d3d11_desc = ConvertSamplerDesc(desc);
    HR(device->CreateSamplerState(&d3d11_desc, &sampler));
}

} // namespace Riley