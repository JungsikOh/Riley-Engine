#pragma once
#include "../Core/Rendering.h"
#include "../Math/MathTypes.h"
#include "DXDepthStencilBuffer.h"
#include "DXRenderTarget.h"
#include "DXStates.h"

namespace Riley
{
struct DXRenderPassDesc
{
    DXRenderTarget* attachmentRTVs = nullptr;
    DXDepthStencilBuffer* attachmentDSVs = nullptr;
    const float* clearColor;
    DXRasterizerState* attachmentRS = nullptr;
    DXDepthStencilState* attachmentDSS = nullptr;
    uint32 width, height; // For Setting Viewport size.

    void BeginRenderPass(ID3D11DeviceContext* context, bool isClearRTVs = true,
                         bool isClearDSV = true, uint8 isStencil = 0);
    void EndRenderPass(ID3D11DeviceContext* context);
    void Destroy();
};
} // namespace Riley