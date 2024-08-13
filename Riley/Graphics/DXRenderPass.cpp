#include "DXRenderPass.h"
#include "../Utilities/StringUtil.h"

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
    ID3D11RenderTargetView* nullViews[1] = {nullptr};

    context->OMSetRenderTargets(1, nullViews, nullptr);
}

void DXRenderPassDesc::Destroy()
{
    attachmentRTVs = nullptr;
    attachmentDSVs = nullptr;
    attachmentDSS = nullptr;
}

} // namespace Riley