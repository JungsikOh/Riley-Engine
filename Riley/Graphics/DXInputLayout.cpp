#include "DXInputLayout.h"

namespace Riley {
static inline void
ConvertInputLayout(DXInputLayoutDesc const& input_layout,
                   std::vector<D3D11_INPUT_ELEMENT_DESC>& element_descs) {

    element_descs.resize(input_layout.elements.size());

    for (uint32 i = 0; i < element_descs.size(); ++i) {
        DXInputLayoutDesc::DXInputElement const& element =
            input_layout.elements[i];

        D3D11_INPUT_ELEMENT_DESC desc{};
        desc.AlignedByteOffset = element.aligned_byte_offset;
        desc.Format = ConvertDXFormat(element.format);
        desc.InputSlot = element.input_slot;

        switch (element.input_slot_class) {
        case DXInputClassification::PerVertexData:
            desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            break;
        case DXInputClassification::PerInstanceData:
            desc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
            break;
        }

        desc.InstanceDataStepRate = 0;
        if (desc.InputSlotClass == D3D11_INPUT_PER_INSTANCE_DATA)
            desc.InstanceDataStepRate = 1;
        desc.SemanticIndex = element.semantic_index;
        desc.SemanticName = element.semantic_name.c_str();
        element_descs[i] = desc;
    }
}

DXInputLayout::DXInputLayout(ID3D11Device* device,
                             DXShaderBytecode const& vs_blob,
                             DXInputLayoutDesc const& desc) {
    ConvertInputLayout(desc, m_elementDescs);
    HR(device->CreateInputLayout(
        m_elementDescs.data(), (uint32)m_elementDescs.size(),
        vs_blob.GetPointer(), vs_blob.GetLength(), &m_inputLayout));
}

DXInputLayout::DXInputLayout(ID3D11Device* device,
                             DXShaderBytecode const& vs_blob) {
    DXInputLayoutDesc desc;
    ConvertInputLayout(desc, m_elementDescs);
    HR(device->CreateInputLayout(
        m_elementDescs.data(), (uint32)m_elementDescs.size(),
        vs_blob.GetPointer(), vs_blob.GetLength(), &m_inputLayout));
}

} // namespace Riley