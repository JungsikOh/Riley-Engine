#pragma once
#include "../Core/Rendering.h"
#include "../Math/MathTypes.h"
#include "DXDepthStencilBuffer.h"
#include "DXRenderTarget.h"
#include "DXStates.h"

namespace Riley {
struct DXRenderPassDesc {
    DXRenderTarget* attachmentRTVs;
    DXDepthStencilBuffer* attachmentDSVs;
    const float* clearColor;
    DXRasterizerState* attachmentRS;
    DXDepthStencilState* attachmentDSS;
    uint32 width, height; // For Setting Viewport size.

    void BindRenderPass(ID3D11DeviceContext* context, uint8 isStencil);
};
} // namespace Riley