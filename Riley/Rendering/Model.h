#pragma once
#include "../Core/CoreTypes.h"
#include "../Core/ECS.h"
#include "../Core/Rendering.h"
#include "../Graphics/DXConstantBuffer.h"
#include "../Math/MathTypes.h"
#include "ConstantBuffers.h"
#include "Object.h"
#include <iostream>
#include <vector>

namespace Riley {
class Model : public Component {
  public:
    Model() = default;
    Model(ID3D11Device* device, std::string name, ObjectConsts& meshConsts,
          MaterialConsts& materialConsts);
    Model(ID3D11Device* device, std::string name, Transform& transform);
    virtual ~Model();

    //virtual void SetEntity(Entity* entity) = 0;
    //virtual Entity* GetEntity() = 0;

    virtual void Update(const float& dt){};
    virtual void Update(ID3D11Device* device, ID3D11DeviceContext* context, Transform& transform);
    virtual void Update(ID3D11Device* device, ID3D11DeviceContext* context,
                        Matrix& matrix);

    void SetMeshList(const Mesh& meshes) { m_meshes = meshes; }

    Mesh GetMeshList() const { return m_meshes; }
    std::string GetName() const { return m_name; }

    void Bind(ID3D11DeviceContext* context) {
        //m_meshConstsGPU->Bind(context, DXShaderStage::VS, 1);
        //m_materialConstsGPU->Bind(context, DXShaderStage::PS, 2);
    }

  private:
    ObjectConsts m_meshConstsCPU;
    MaterialConsts m_materialConstsCPU;
    //DXConstantBuffer<ObjectConsts>* m_meshConstsGPU;
    //DXConstantBuffer<MaterialConsts>* m_materialConstsGPU;

    Mesh m_meshes;
};
} // namespace Riley