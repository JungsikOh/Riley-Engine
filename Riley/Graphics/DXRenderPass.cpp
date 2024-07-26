#include "DXRenderPass.h"

namespace Riley
{
  void DXRenderPassDesc::BindRenderPass(ID3D11DeviceContext* context, uint8 isStencil)
  {
    attachmentRS->Bind(context);
    attachmentDSS->Bind(context, isStencil);
  }
}