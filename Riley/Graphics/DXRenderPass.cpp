#include "DXRenderPass.h"

namespace Riley
{
void DXRenderPassDesc::BeginRenderPass(ID3D11DeviceContext* context, bool isClearRTVs, uint8 isStencil)
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
         if (isClearRTVs)
            attachmentRTVs->Clear(context, clearColor);
         attachmentRTVs->BindRenderTargets(context);
      }
}

void DXRenderPassDesc::EndRenderPass(ID3D11DeviceContext* context)
{
   context->OMSetRenderTargets(0, nullptr, nullptr);
}

} // namespace Riley