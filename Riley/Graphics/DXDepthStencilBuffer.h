#pragma once
#include "DXResource.h"

namespace Riley
{
class DXDepthStencilBuffer : public DXResource
{
   public:
   DXDepthStencilBuffer() = default;
   DXDepthStencilBuffer(ID3D11Device* device, uint32 width, uint32 height, bool isStencilEnable = true, bool isTextureCube = false);
   virtual ~DXDepthStencilBuffer()
   {
      SAFE_RELEASE(m_dsv);
   }

   void Initialize(ID3D11Device* device, uint32 width, uint32 height, bool isStencilEnable = true,
                  D3D11_TEXTURE2D_DESC* texDesc = nullptr, D3D11_DEPTH_STENCIL_VIEW_DESC* dsvDesc = nullptr);

   ID3D11DepthStencilView* DSV() const
   {
      return m_dsv;
   }
   void BindOnlyDSV(ID3D11DeviceContext* context) const
   {
      ID3D11RenderTargetView* nullViews[] = {nullptr};
      context->OMSetRenderTargets(0, nullViews, m_dsv);
   }
   void Clear(ID3D11DeviceContext* context, float depth, uint8 stencil);

   private:
   ID3D11DepthStencilView* m_dsv = nullptr;
   bool m_isStencilEnabled;
   bool m_isTextureCube;

   uint32 m_width;
   uint32 m_height;
};
} // namespace Riley