#include "DXRenderPass.h"

namespace Riley
{
void DXRenderPassDesc::BeginRenderPass(ID3D11DeviceContext* context, bool isClearRTV, bool isClearDSV, uint8 isStencil)
{
   attachmentRS->Bind(context);
   attachmentDSS->Bind(context, isStencil);

   attachmentDSVs->Clear(context, 1.0f, isStencil);
   if (attachmentRTVs == nullptr)
      {
         attachmentDSVs->BindOnlyDSV(context);
      }
   else
      {
         if (isClearRTV)
            attachmentRTVs->Clear(context, clearColor);
         attachmentRTVs->BindRenderTargets(context);
      }
}

void DXRenderPassDesc::EndRenderPass(ID3D11DeviceContext* context)
{
   // https://stackoverflow.com/questions/69996893/d3d11-warning-when-resizing-the-window
   ID3D11RenderTargetView* nullViews[] = {nullptr};
   ID3D11DepthStencilView* nullDSV = nullptr;
   ID3D11RasterizerState* nullRS = nullptr;

   context->OMSetRenderTargets(0, nullViews, nullDSV);
   context->RSSetState(nullRS);
   context->OMSetDepthStencilState(nullptr, 0);
}

void DXRenderPassDesc::Destroy () 
{
   attachmentRTVs = nullptr;
   attachmentDSVs = nullptr;
   attachmentDSS = nullptr;
}

} // namespace Riley