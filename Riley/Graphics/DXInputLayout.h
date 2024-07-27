#pragma once
#include "../Core/Rendering.h"
#include "DXFormat.h"
#include "DXShader.h"

namespace Riley {
enum class DXInputClassification { PerVertexData, PerInstanceData };
struct DXInputLayoutDesc {
    static constexpr uint32 APPEND_ALIGNED_ELEMENT = ~0u;
    struct DXInputElement {
        std::string semantic_name;
        uint32 semantic_index = 0;
        DXFormat format = DXFormat::UNKNOWN;
        uint32 input_slot = 0;
        uint32 aligned_byte_offset = APPEND_ALIGNED_ELEMENT;
        DXInputClassification input_slot_class =
            DXInputClassification::PerVertexData;
    };
    std::vector<DXInputElement> elements;
};

class DXInputLayout {
  public:
    DXInputLayout(ID3D11Device* device, DXShaderBytecode const& vs_blob);
    DXInputLayout(ID3D11Device* device, DXShaderBytecode const& vs_blob,
                  DXInputLayoutDesc const& desc);

    operator ID3D11InputLayout*() const { return m_inputLayout; }

  private:
    ID3D11InputLayout* m_inputLayout;
};
} // namespace Riley
