#include "Model.h"

namespace Riley {
Model::Model(ID3D11Device* device, std::string name, ObjectConsts& meshConsts,
             MaterialConsts& materialConsts)
    : m_meshConstsCPU(meshConsts), m_materialConstsCPU(materialConsts) {

    m_name = name;
    m_meshConstsGPU =
        new DXConstantBuffer<ObjectConsts>(device, &m_meshConstsCPU);
    m_materialConstsGPU =
        new DXConstantBuffer<MaterialConsts>(device, &m_materialConstsCPU);
}

/**
 * @brief Material is prduced by Defalt Setting
 */
Model::Model(ID3D11Device* device, std::string name, Transform& transform) {

    m_name = name;
    m_transform = transform;
    m_meshConstsCPU.worldRow = transform.GetMatrix().Transpose();
    m_meshConstsCPU.worldInvTransposeRow =
        transform.GetMatrix().Invert().Transpose();

    m_meshConstsGPU =
        new DXConstantBuffer<ObjectConsts>(device, &m_meshConstsCPU);
    m_materialConstsGPU =
        new DXConstantBuffer<MaterialConsts>(device, &m_materialConstsCPU);
}

Model::~Model() {
    SAFE_DELETE(m_meshConstsGPU);
    SAFE_DELETE(m_materialConstsGPU);
    for (auto& m : m_meshes) {
        m.DeInit();
    }
}

void Model::Update(ID3D11Device* device, ID3D11DeviceContext* context,
                   Transform& transform) {
    m_meshConstsCPU.worldRow = transform.GetMatrix().Transpose();
    m_meshConstsCPU.worldInvTransposeRow =
        transform.GetMatrix().Invert().Transpose();

    if (m_meshConstsGPU != nullptr) {
        m_meshConstsGPU->Update(context, &m_meshConstsCPU,
                                sizeof(m_meshConstsCPU));
    }
}

void Model::Update(ID3D11Device* device, ID3D11DeviceContext* context,
                   Matrix& matrix) {
    m_meshConstsCPU.worldRow = matrix.Transpose();
    m_meshConstsCPU.worldInvTransposeRow = matrix.Invert().Transpose();

    if (m_meshConstsGPU != nullptr) {
        m_meshConstsGPU->Update(context, &m_meshConstsCPU,
                                sizeof(m_meshConstsCPU));
    }
}

} // namespace Riley