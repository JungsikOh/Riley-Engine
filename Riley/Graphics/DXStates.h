#pragma once

#include "../Math/MathTypes.h"
#include "../Core/CoreTypes.h"
#include "../Core/Rendering.h"
#include "DXShader.h"

namespace Riley
{
enum class DXPrimitiveTopology : uint8
{
   Undefined,
   TriangleList,
   TriangleStrip,
   PointList,
   LineList,
   LineStrip,
   PatchList1,
   PatchList2,
   PatchList3,
   PatchList4,
   PatchList5,
   PatchList6,
   PatchList7,
   PatchList8,
   PatchList9,
   PatchList10,
   PatchList11,
   PatchList12,
   PatchList13,
   PatchList14,
   PatchList15,
   PatchList16,
   PatchList17,
   PatchList18,
   PatchList19,
   PatchList20,
   PatchList21,
   PatchList22,
   PatchList23,
   PatchList24,
   PatchList25,
   PatchList26,
   PatchList27,
   PatchList28,
   PatchList29,
   PatchList30,
   PatchList31,
   PatchList32
};
enum class DXComparisonFunc : uint8
{
   Never,
   Less,
   Equal,
   LessEqual,
   Greater,
   NotEqual,
   GreaterEqual,
   Always,
};

enum class DXDepthWriteMask : uint8
{
   Zero,
   All,
};

enum class DXStencilOp : uint8
{
   Keep,
   Zero,
   Replace,
   IncrSat,
   DecrSat,
   Invert,
   Incr,
   Decr,
};

enum class DXBlend : uint8
{
   Zero,
   One,
   SrcColor,
   InvSrcColor,
   SrcAlpha,
   InvSrcAlpha,
   DstAlpha,
   InvDstAlpha,
   DstColor,
   InvDstColor,
   SrcAlphaSat,
   BlendFactor,
   InvBlendFactor,
   Src1Color,
   InvSrc1Color,
   Src1Alpha,
   InvSrc1Alpha,
};

enum class DXBlendOp : uint8
{
   Add,
   Subtract,
   RevSubtract,
   Min,
   Max,
};

enum class DXFillMode : uint8
{
   Wireframe,
   Solid,
};

enum class DXCullMode : uint8
{
   None,
   Front,
   Back,
};

enum class DXColorWrite : uint8
{
   Disable = 0,
   EnableRed = 1 << 0,
   EnableGreen = 1 << 1,
   EnableBlue = 1 << 2,
   EnableAlpha = 1 << 3,
   EnableAll = EnableRed | EnableGreen | EnableBlue | EnableAlpha,
};

struct DXRasterizerStateDesc
{
   DXFillMode fill_mode = DXFillMode::Solid;
   DXCullMode cull_mode = DXCullMode::Back;
   bool front_counter_clockwise = false;
   int32 depth_bias = 0;
   float depth_bias_clamp = 0.0f;
   float slope_scaled_depth_bias = 0.0f;
   bool depth_clip_enable = true;
   bool multisample_enable = false;
   bool antialiased_line_enable = false;
   bool conservative_rasterization_enable = false;
   uint32 forced_sample_count = 0;
};

struct DXDepthStencilStateDesc
{
   bool depth_enable = true;
   DXDepthWriteMask depth_write_mask = DXDepthWriteMask::All;
   DXComparisonFunc depth_func = DXComparisonFunc::Less;
   bool stencil_enable = false;
   uint8 stencil_read_mask = 0xff;
   uint8 stencil_write_mask = 0xff;
   struct DXDepthStencilOp
   {
      DXStencilOp stencil_fail_op = DXStencilOp::Keep;
      DXStencilOp stencil_depth_fail_op = DXStencilOp::Keep;
      DXStencilOp stencil_pass_op = DXStencilOp::Keep;
      DXComparisonFunc stencil_func = DXComparisonFunc::Always;
   };
   DXDepthStencilOp front_face{};
   DXDepthStencilOp back_face{};
};

struct DXBlendStateDesc
{
   bool alpha_to_coverage_enable = false;
   bool independent_blend_enable = false;
   struct DXRenderTargetBlendState
   {
      bool blend_enable = false;
      DXBlend src_blend = DXBlend::One;
      DXBlend dest_blend = DXBlend::Zero;
      DXBlendOp blend_op = DXBlendOp::Add;
      DXBlend src_blend_alpha = DXBlend::One;
      DXBlend dest_blend_alpha = DXBlend::Zero;
      DXBlendOp blend_op_alpha = DXBlendOp::Add;
      DXColorWrite render_target_write_mask = DXColorWrite::EnableAll;
   };
   DXRenderTargetBlendState render_target[8];
};

//////////////////////////
// State Class //
//////////////////////////

class DXRasterizerState
{
   public:
   DXRasterizerState(ID3D11Device* device, DXRasterizerStateDesc const& desc);
   ~DXRasterizerState()
   {
      SAFE_RELEASE(rasterizerState);
   }

   void Bind(ID3D11DeviceContext* context)
   {
      context->RSSetState(rasterizerState);
   }

   private:
   ID3D11RasterizerState* rasterizerState;
};

class DXDepthStencilState
{
   public:
   DXDepthStencilState(ID3D11Device* device, DXDepthStencilStateDesc const& desc);
   ~DXDepthStencilState()
   {
      SAFE_RELEASE(depthStencilState);
   }

   void Bind(ID3D11DeviceContext* context, uint8 stencilRef)
   {
      context->OMSetDepthStencilState(depthStencilState, stencilRef);
   }

   private:
   ID3D11DepthStencilState* depthStencilState;
};

class DXBlendState
{
   public:
   DXBlendState(ID3D11Device* device, DXBlendStateDesc const& desc);
   ~DXBlendState()
   {
      SAFE_RELEASE(blendState);
   }

   void Bind(ID3D11DeviceContext* context, const float* blendFactor = nullptr, uint32 sampleMask = 0xffffff)
   {
      context->OMSetBlendState(blendState, blendFactor, sampleMask);
   }

   void Unbind(ID3D11DeviceContext* context, const float* blendFactor = nullptr, uint32 sampleMask = 0xffffff)
   {
      context->OMSetBlendState(nullptr, blendFactor, sampleMask);
   }

   private:
   ID3D11BlendState* blendState;
};

enum class DXTextureAddressMode
{
   Wrap,
   Mirror,
   Clamp,
   Border,
};
enum class DXFilter : uint32
{
   MIN_MAG_MIP_POINT,
   MIN_MAG_POINT_MIP_LINEAR,
   MIN_POINT_MAG_LINEAR_MIP_POINT,
   MIN_POINT_MAG_MIP_LINEAR,
   MIN_LINEAR_MAG_MIP_POINT,
   MIN_LINEAR_MAG_POINT_MIP_LINEAR,
   MIN_MAG_LINEAR_MIP_POINT,
   MIN_MAG_MIP_LINEAR,
   ANISOTROPIC,
   COMPARISON_MIN_MAG_MIP_POINT,
   COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
   COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
   COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
   COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
   COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
   COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
   COMPARISON_MIN_MAG_MIP_LINEAR,
   COMPARISON_ANISOTROPIC,
   MINIMUM_MIN_MAG_MIP_POINT,
   MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
   MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
   MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
   MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
   MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
   MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
   MINIMUM_MIN_MAG_MIP_LINEAR,
   MINIMUM_ANISOTROPIC,
   MAXIMUM_MIN_MAG_MIP_POINT,
   MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
   MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
   MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
   MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
   MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
   MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
   MAXIMUM_MIN_MAG_MIP_LINEAR,
   MAXIMUM_ANISOTROPIC,
};

struct DXSamplerDesc
{
   DXFilter filter = DXFilter::MIN_MAG_MIP_POINT;
   DXTextureAddressMode addressU = DXTextureAddressMode::Clamp;
   DXTextureAddressMode addressV = DXTextureAddressMode::Clamp;
   DXTextureAddressMode addressW = DXTextureAddressMode::Clamp;
   float mipLodBias = 0.0f;
   uint32 maxAnisotropy = 0;
   DXComparisonFunc comparisonFunc = DXComparisonFunc::Never;
   float borderColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
   float minLod = 0.0f;
   float maxLod = FLT_MAX;
};

class DXSampler
{
   public:
   DXSampler(ID3D11Device* device, DXSamplerDesc const& desc);
   ~DXSampler()
   {
      SAFE_RELEASE(sampler);
   }

   void Bind(ID3D11DeviceContext* context, uint32 slot, DXShaderStage const& stage)
   {
      switch (stage)
         {
         case DXShaderStage::VS:
            context->VSSetSamplers(slot, 1, &sampler);
            break;
         case DXShaderStage::PS:
            context->PSSetSamplers(slot, 1, &sampler);
            break;
         case DXShaderStage::CS:
             context->CSSetSamplers(slot, 1, &sampler);
             break;
         default:
            assert(false && "Unsupported Blend Shader Stage!");
         }
   }

   operator ID3D11SamplerState* const() const
   {
      return sampler;
   }

   private:
   ID3D11SamplerState* sampler;
};

inline DXBlendStateDesc OpaqueBlendStateDesc()
{
   DXBlendStateDesc desc{};
   desc.render_target[0].blend_enable = false;
   desc.render_target[0].src_blend = desc.render_target[0].src_blend_alpha = DXBlend::One;
   desc.render_target[0].dest_blend = desc.render_target[0].dest_blend_alpha = DXBlend::Zero;
   desc.render_target[0].blend_op = desc.render_target[0].blend_op_alpha = DXBlendOp::Add;
   desc.render_target[0].render_target_write_mask = DXColorWrite::EnableAll;
   return desc;
}
inline DXBlendStateDesc AlphaBlendStateDesc()
{
   DXBlendStateDesc desc{};
   desc.render_target[0].blend_enable = true;
   desc.render_target[0].src_blend = desc.render_target[0].src_blend_alpha = DXBlend::SrcAlpha;
   desc.render_target[0].dest_blend = desc.render_target[0].dest_blend_alpha = DXBlend::InvSrcAlpha;
   desc.render_target[0].blend_op = desc.render_target[0].blend_op_alpha = DXBlendOp::Add;
   desc.render_target[0].render_target_write_mask = DXColorWrite::EnableAll;
   return desc;
}
inline DXBlendStateDesc AdditiveBlendStateDesc()
{
   DXBlendStateDesc desc{};
   desc.render_target[0].blend_enable = true;
   desc.render_target[0].src_blend = desc.render_target[0].src_blend_alpha = DXBlend::One;
   desc.render_target[0].dest_blend = desc.render_target[0].dest_blend_alpha = DXBlend::One;
   desc.render_target[0].blend_op = desc.render_target[0].blend_op_alpha = DXBlendOp::Add;
   desc.render_target[0].render_target_write_mask = DXColorWrite::EnableAll;
   return desc;
}

inline DXDepthStencilStateDesc NoneDepthDesc()
{
   DXDepthStencilStateDesc desc{};
   desc.depth_enable = false;
   desc.depth_write_mask = DXDepthWriteMask::Zero;
   desc.depth_func = DXComparisonFunc::LessEqual;
   desc.stencil_enable = false;
   desc.stencil_read_mask = 0xff;
   desc.stencil_write_mask = 0xff;

   desc.front_face.stencil_func = DXComparisonFunc::Always;
   desc.front_face.stencil_pass_op = DXStencilOp::Keep;
   desc.front_face.stencil_fail_op = DXStencilOp::Keep;
   desc.front_face.stencil_depth_fail_op = DXStencilOp::Keep;
   desc.back_face = desc.front_face;

   return desc;
}
inline DXDepthStencilStateDesc DefaultDepthDesc()
{
   DXDepthStencilStateDesc desc{};
   desc.depth_enable = true;
   desc.depth_write_mask = DXDepthWriteMask::All;
   desc.depth_func = DXComparisonFunc::LessEqual;
   desc.stencil_enable = false;
   desc.stencil_read_mask = 0xff;
   desc.stencil_write_mask = 0xff;

   desc.front_face.stencil_func = DXComparisonFunc::Always;
   desc.front_face.stencil_pass_op = DXStencilOp::Keep;
   desc.front_face.stencil_fail_op = DXStencilOp::Keep;
   desc.front_face.stencil_depth_fail_op = DXStencilOp::Keep;
   desc.back_face = desc.front_face;

   return desc;
}
inline DXDepthStencilStateDesc ReadDepthDesc()
{
   DXDepthStencilStateDesc desc{};
   desc.depth_enable = true;
   desc.depth_write_mask = DXDepthWriteMask::Zero;
   desc.depth_func = DXComparisonFunc::LessEqual;
   desc.stencil_enable = false;
   desc.stencil_read_mask = 0xff;
   desc.stencil_write_mask = 0xff;

   desc.front_face.stencil_func = DXComparisonFunc::Always;
   desc.front_face.stencil_pass_op = DXStencilOp::Keep;
   desc.front_face.stencil_fail_op = DXStencilOp::Keep;
   desc.front_face.stencil_depth_fail_op = DXStencilOp::Keep;
   desc.back_face = desc.front_face;

   return desc;
}

inline DXRasterizerStateDesc CullNoneDesc()
{
   DXRasterizerStateDesc desc{};
   desc.cull_mode = DXCullMode::None;
   desc.fill_mode = DXFillMode::Solid;
   desc.depth_clip_enable = true;
   desc.multisample_enable = true;
   return desc;
}
inline DXRasterizerStateDesc CullCWDesc()
{
   DXRasterizerStateDesc desc{};
   desc.cull_mode = DXCullMode::Front;
   desc.fill_mode = DXFillMode::Solid;
   desc.depth_clip_enable = true;
   desc.multisample_enable = true;
   return desc;
}
inline DXRasterizerStateDesc CullCCWDesc()
{
   DXRasterizerStateDesc desc{};
   desc.cull_mode = DXCullMode::Back;
   desc.fill_mode = DXFillMode::Solid;
   desc.depth_clip_enable = true;
   desc.multisample_enable = true;
   return desc;
}
inline DXRasterizerStateDesc WireframeDesc()
{
   DXRasterizerStateDesc desc{};
   desc.cull_mode = DXCullMode::Back;
   desc.fill_mode = DXFillMode::Wireframe;
   desc.depth_clip_enable = true;
   desc.multisample_enable = true;
   return desc;
}

inline DXSamplerDesc SamplerDesc(DXFilter filter, DXTextureAddressMode mode)
{
   DXSamplerDesc desc{};
   desc.filter = filter;
   desc.addressU = mode;
   desc.addressV = mode;
   desc.addressW = mode;
   desc.maxAnisotropy = 16;
   desc.maxLod = FLT_MAX;
   desc.comparisonFunc = DXComparisonFunc::Never;
   return desc;
}
} // namespace Riley