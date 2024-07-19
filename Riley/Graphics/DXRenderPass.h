#pragma once
#include "../Core/Rendering.h"
#include "../Math/MathTypes.h"
#include "DXDepthStencilBuffer.h"
#include "DXRenderTarget.h"

namespace Riley {
struct DXRenderPassDesc {
    std::vector<DXRenderTarget*> attachmentRTVs;
    DXDepthStencilBuffer* attachmentDSV;
    const float* clearColor;
    uint32 width, height; // For Setting Viewport size.
};
} // namespace Riley