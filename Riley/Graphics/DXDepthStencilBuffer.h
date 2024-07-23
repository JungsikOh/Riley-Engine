#pragma once
#include "DXResource.h"

namespace Riley {
class DXDepthStencilBuffer : public DXResource {
  public:
    DXDepthStencilBuffer() = default;
    DXDepthStencilBuffer(ID3D11Device* device, uint32 width, uint32 height,
                         bool isStencilEnable = true, bool isTextureCube = false);
    virtual ~DXDepthStencilBuffer() { SAFE_RELEASE(m_dsv); }

    ID3D11DepthStencilView* GetDSV() { return m_dsv; }
    void Clear(ID3D11DeviceContext* context, float depth, uint8 stencil);

    uint32 m_width;
    uint32 m_height;

  private:
    ID3D11DepthStencilView* m_dsv;
    bool m_isStencilEnabled;
    bool m_isTextureCube;
};
} // namespace Riley