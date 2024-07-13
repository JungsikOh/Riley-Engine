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

    virtual void SetEntity(Entity* entity) = 0;
    virtual Entity* GetEntity() = 0;

    void Update(ID3D11Device* device, ID3D11DeviceContext* context, Transform& transform);
    void Update(ID3D11Device* device, ID3D11DeviceContext* context,
                        Matrix& matrix);

    void SetMeshList(const std::vector<Mesh>& meshes) { m_meshes = meshes; }
    void SetMaterialList(const Material& material) { m_material = material; }

    std::vector<Mesh> GetMeshList() const { return m_meshes; }
    Material GetMaterialList() const { return m_material; }
    std::string GetName() const { return m_name; }

  private:
    std::string m_name;

    ObjectConsts m_meshConstsCPU;
    MaterialConsts m_materialConstsCPU;
    DXConstantBuffer<ObjectConsts>* m_meshConstsGPU;
    DXConstantBuffer<MaterialConsts>* m_materialConstsGPU;

    std::vector<Mesh> m_meshes;
    Material m_material;
};
} // namespace Riley