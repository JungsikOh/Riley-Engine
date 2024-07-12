#pragma once
#include "DXResource.h"

namespace Riley {
class DXDepthStencilBuffer : public DXResource {
  public:
    DXDepthStencilBuffer() = default;
    DXDepthStencilBuffer(ID3D11Device* device, uint32 width, uint32 height,
                         bool isStencilEnable = true);
    virtual ~DXDepthStencilBuffer() { SAFE_RELEASE(m_dsv); }

    ID3D11DepthStencilView* GetDSV() const { return m_dsv; }
    void Clear(ID3D11DeviceContext* context, float depth, uint8 stencil);

  private:
    ID3D11DepthStencilView* m_dsv;
    bool m_isStencilEnabled;
};
} // namespace Riley