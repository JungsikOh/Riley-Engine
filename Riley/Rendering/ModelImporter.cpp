#include "ModelImporter.h"
#include "../Math/ComputeVectors.h"
#include "spdlog\spdlog.h"
#include <unordered_map>

namespace Riley {

ModelImporter::ModelImporter(ID3D11Device* device, entt::registry& reg)
    : m_device(device), m_registry(reg) {}

std::vector<entt::entity> ModelImporter::LoadSquare(const Vector3& pos,
                                                    const float& scale,
                                                    const Vector2& texScale) {

    std::vector<Vector3> positions;
    std::vector<Vector3> colors;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::vector<Vector3> tangents;
    std::vector<Vector3> bitangents;

    positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);

    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));

    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    std::vector<entt::entity> entities{};
    std::vector<Vertex> vertices;
    std::vector<uint32> indices{0, 1, 2, 0, 2, 3};

    ComputeAndSetTangets(indices, positions, normals, texcoords, tangents,
                         bitangents);

    for (int i = 0; i < positions.size(); ++i) {
        Vertex v;
        v.position = positions[i];
        v.normal = normals[i];
        v.texcoord = texcoords[i];
        v.tangent = tangents[i];
        v.bitangent = bitangents[i];

        vertices.push_back(v);
    }

    entt::entity entity = m_registry.create();
    Mesh mesh{};
    mesh.vertexBuffer = std::make_shared<DXBuffer>(
        m_device, VertexBufferDesc(vertices.size(), sizeof(Vertex)), vertices.data());
    mesh.indexBuffer = std::make_shared<DXBuffer>(
        m_device, IndexBufferDesc(indices.size(), false), indices.data());
    mesh.vertexCount = static_cast<uint32>(vertices.size());
    mesh.indexCount = static_cast<uint32>(indices.size());

    m_registry.emplace<Mesh>(entity, mesh);

    Material material{};
    material.shader = ShaderProgram::Solid;

    m_registry.emplace<Material>(entity, material);

    Transform transform{};
    transform.startingTransform =
        Matrix::CreateScale(scale) * Matrix::CreateTranslation(pos);
    transform.currentTransform =
        Matrix::CreateScale(scale) * Matrix::CreateTranslation(pos);

    m_registry.emplace<Transform>(entity, transform);

    return std::vector{entity};
}
} // namespace Riley