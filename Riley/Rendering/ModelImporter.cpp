#include "ModelImporter.h"
#include "../Core/Log.h"
#include "../Math/BoundingVolume.h"
#include "../Math/ComputeVectors.h"
#include "../Utilities/FileUtil.h"
#include "../Utilities/StringUtil.h"
#include "Enums.h"
#include "ModelLoader.h"

namespace Riley
{

ModelImporter::ModelImporter(ID3D11Device* device, entt::registry& reg) : m_device(device), m_registry(reg)
{
}

std::vector<entt::entity> ModelImporter::LoadSquare(const Vector3& pos, const float& scale, const float& rotate)
{
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::vector<Vector3> tangents;
    std::vector<Vector3> bitangents;

    positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);

    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    std::vector<Vertex> vertices;
    std::vector<uint32> indices{0, 1, 2, 0, 2, 3};

    ComputeAndSetTangets(indices, positions, normals, texcoords, tangents, bitangents);

    for (int i = 0; i < positions.size(); ++i)
    {
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
    mesh.vertexBuffer = std::make_shared<DXBuffer>(m_device, VertexBufferDesc(vertices.size(), sizeof(Vertex)), vertices.data());
    mesh.indexBuffer = std::make_shared<DXBuffer>(m_device, IndexBufferDesc(indices.size(), false), indices.data());
    mesh.vertexCount = static_cast<uint32>(vertices.size());
    mesh.indexCount = static_cast<uint32>(indices.size());

    m_registry.emplace<Mesh>(entity, mesh);

    Material material{};
    material.diffuse = Vector3(0.2f, 0.3f, 0.2f);
    material.shader = ShaderProgram::ForwardPhong;

    m_registry.emplace<Material>(entity, material);

    Transform transform{};
    transform.startingTransform =
        Matrix::CreateScale(scale) * Matrix::CreateRotationX(DirectX::XMConvertToRadians(rotate)) * Matrix::CreateTranslation(pos);
    transform.currentTransform =
        Matrix::CreateScale(scale) * Matrix::CreateRotationX(DirectX::XMConvertToRadians(rotate)) * Matrix::CreateTranslation(pos);

    m_registry.emplace<Transform>(entity, transform);

    AABB aabb{};
    BoundingBox _boundingBox = AABBFromVertices(vertices);
    aabb.orginalBox = _boundingBox;
    _boundingBox.Transform(_boundingBox, transform.currentTransform);
    aabb.boundingBox = _boundingBox;
    aabb.isDrawAABB = false;
    aabb.UpdateBuffer(m_device);

    m_registry.emplace<AABB>(entity, aabb);

    Tag tag{};
    tag.name = "Square";
    m_registry.emplace<Tag>(entity, tag);

    return std::vector{entity};
}

std::vector<entt::entity> ModelImporter::LoadBox(const Vector3& pos, const float& scale /*1.0f*/,
                                                 const Vector2& texScale /*Vector2(1.0f)*/, bool invertNormals /*false*/)
{
    float flag = 1.0f;
    if (invertNormals)
        flag *= -1.0f;

    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::vector<Vector3> tangents;
    std::vector<Vector3> bitangents;

    // À­¸é
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ¾Æ·§¸é
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ¾Õ¸é
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // µÞ¸é
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ¿ÞÂÊ
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ¿À¸¥ÂÊ
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    std::vector<uint32> indices{
        0,  1,  2,  0,  2,  3,  // À­¸é
        4,  5,  6,  4,  6,  7,  // ¾Æ·§
        8,  9,  10, 8,  10, 11, // ¾Õ¸é
        12, 13, 14, 12, 14, 15, // µÞ¸é
        16, 17, 18, 16, 18, 19, // ¿ÞÂÊ
        20, 21, 22, 20, 22, 23  // ¿À¸¥
    };

    ComputeAndSetTangets(indices, positions, normals, texcoords, tangents, bitangents);

    std::vector<Vertex> vertices;

    for (uint32 i = 0; i < positions.size(); ++i)
    {
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
    mesh.vertexBuffer = std::make_shared<DXBuffer>(m_device, VertexBufferDesc(vertices.size(), sizeof(Vertex)), vertices.data());
    mesh.indexBuffer = std::make_shared<DXBuffer>(m_device, IndexBufferDesc(indices.size(), false), indices.data());
    mesh.vertexCount = static_cast<uint32>(vertices.size());
    mesh.indexCount = static_cast<uint32>(indices.size());

    m_registry.emplace<Mesh>(entity, mesh);

    Material material{};
    material.shader = ShaderProgram::ForwardPhong;
    material.diffuse = Vector3(0.1f, 0.5f, 0.0f);

    m_registry.emplace<Material>(entity, material);

    Transform transform{};
    transform.startingTransform = Matrix::CreateScale(scale) * Matrix::CreateTranslation(pos);
    transform.currentTransform = Matrix::CreateScale(scale) * Matrix::CreateTranslation(pos);

    m_registry.emplace<Transform>(entity, transform);

    AABB aabb{};
    BoundingBox _boundingBox = AABBFromVertices(vertices);
    aabb.orginalBox = _boundingBox;
    _boundingBox.Transform(_boundingBox, transform.currentTransform);
    aabb.boundingBox = _boundingBox;
    aabb.isDrawAABB = false;
    aabb.UpdateBuffer(m_device);
    m_registry.emplace<AABB>(entity, aabb);

    Tag tag{};
    tag.name = "Box";
    m_registry.emplace<Tag>(entity, tag);

    return std::vector<entt::entity>{entity};
}

std::vector<entt::entity> ModelImporter::LoadSqhere(Vector3 const& pos, float const& radius /*= 1.0f*/, uint32 numSlices,
                                                    uint32 numStacks)
{
    const float dTheta = -XM_2PI / float(numSlices);
    const float dPhi = -XM_PI / float(numStacks);

    std::vector<Vertex> vertices;
    for (uint32 j = 0; j <= numStacks; ++j)
    {
        Vector3 stackStartPoint = Vector3::Transform(Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * float(j)));
        for (uint32 i = 0; i <= numSlices; ++i)
        {
            Vertex v;
            v.position = Vector3::Transform(stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));
            v.normal = v.position;
            v.normal.Normalize();
            v.texcoord = Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks) * Vector2(1.0f);
            v.bitangent = Vector3(0.0f, 1.0f, 0.0f);

            Vector3 normalOrth = v.normal - v.bitangent.Dot(v.normal) * v.normal;
            normalOrth.Normalize();
            v.tangent = v.bitangent.Cross(normalOrth);
            v.tangent.Normalize();

            vertices.push_back(v);
        }
    }

    std::vector<uint32> indices;

    for (uint32 j = 0; j < numStacks; ++j)
    {
        const int offset = (numSlices + 1) * j;
        for (uint32 i = 0; i < numSlices; ++i)
        {
            indices.push_back(offset + i);
            indices.push_back(offset + i + numSlices + 1);
            indices.push_back(offset + i + 1 + numSlices + 1);

            indices.push_back(offset + i);
            indices.push_back(offset + i + 1 + numSlices + 1);
            indices.push_back(offset + i + 1);
        }
    }

    entt::entity entity = m_registry.create();
    Mesh mesh{};
    mesh.vertexBuffer = std::make_shared<DXBuffer>(m_device, VertexBufferDesc(vertices.size(), sizeof(Vertex)), vertices.data());
    mesh.indexBuffer = std::make_shared<DXBuffer>(m_device, IndexBufferDesc(indices.size(), false), indices.data());
    mesh.vertexCount = static_cast<uint32>(vertices.size());
    mesh.indexCount = static_cast<uint32>(indices.size());

    m_registry.emplace<Mesh>(entity, mesh);

    Material material{};
    material.shader = ShaderProgram::ForwardPhong;
    material.diffuse = Vector3(0.6f, 0.5f, 0.3f);

    m_registry.emplace<Material>(entity, material);

    Transform transform{};
    transform.startingTransform = Matrix::CreateScale(radius) * Matrix::CreateTranslation(pos);
    transform.currentTransform = Matrix::CreateScale(radius) * Matrix::CreateTranslation(pos);

    m_registry.emplace<Transform>(entity, transform);

    AABB aabb{};
    BoundingBox _boundingBox = AABBFromVertices(vertices);
    aabb.orginalBox = _boundingBox;
    _boundingBox.Transform(_boundingBox, transform.currentTransform);
    aabb.boundingBox = _boundingBox;
    aabb.isDrawAABB = false;
    aabb.UpdateBuffer(m_device);
    m_registry.emplace<AABB>(entity, aabb);

    Tag tag{};
    tag.name = "Sphere";
    m_registry.emplace<Tag>(entity, tag);

    return std::vector<entt::entity>{entity};
}

std::vector<entt::entity> ModelImporter::LoadLight(Light& lightData, LightMesh meshType, const float& scale)
{
    entt::entity light = m_registry.create();

    if (lightData.type == LightType::Directional)
    {
        const_cast<Light&>(lightData).position = -lightData.direction * 1e3;
    }
    m_registry.emplace<Light>(light, lightData);

    if (meshType == LightMesh::Quad)
    {
        float size = scale;
        std::vector<SimpleVertex> const vertices = {{{-0.5f * size, -0.5f * size, 0.0f}, {0.0f, 0.0f}},
                                                    {{0.5f * size, -0.5f * size, 0.0f}, {1.0f, 0.0f}},
                                                    {{0.5f * size, 0.5f * size, 0.0f}, {1.0f, 1.0f}},
                                                    {{-0.5f * size, 0.5f * size, 0.0f}, {0.0f, 1.0f}}};

        std::vector<uint16> indices{0, 2, 1, 2, 0, 3};

        Mesh mesh{};
        mesh.vertexBuffer =
            std::make_shared<DXBuffer>(m_device, VertexBufferDesc(vertices.size(), sizeof(SimpleVertex)), vertices.data());
        mesh.indexBuffer = std::make_shared<DXBuffer>(m_device, IndexBufferDesc(indices.size(), true), indices.data());
        mesh.indexCount = static_cast<uint32>(indices.size());
        m_registry.emplace<Mesh>(light, mesh);

        Material material{};
        material.diffuse = Vector3(lightData.color.x, lightData.color.y, lightData.color.z);

        if (lightData.type == LightType::Directional)
        {
            material.shader = ShaderProgram::UnKnown;
        }
        m_registry.emplace<Material>(light, material);

        Matrix translation = DirectX::XMMatrixTranslationFromVector(lightData.position);
        m_registry.emplace<Transform>(light, translation, translation);
    }
    else if (meshType == LightMesh::Cube)
    {
        std::vector<Vector3> positions;
        std::vector<Vector2> texcoords;

        // À­¸é
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // ¾Æ·§¸é
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // ¾Õ¸é
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // µÞ¸é
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // ¿ÞÂÊ
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // ¿À¸¥ÂÊ
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        std::vector<SimpleVertex> vertices;

        for (uint32 i = 0; i < positions.size(); ++i)
        {
            SimpleVertex v;
            v.position = positions[i];
            v.texcoord = texcoords[i];
            vertices.push_back(v);
        }

        std::vector<uint32> indices{0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9,  10, 8,  10, 11,
                                    12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};

        Mesh mesh{};
        mesh.vertexBuffer =
            std::make_shared<DXBuffer>(m_device, VertexBufferDesc(vertices.size(), sizeof(SimpleVertex)), vertices.data());
        mesh.indexBuffer = std::make_shared<DXBuffer>(m_device, IndexBufferDesc(indices.size(), false), indices.data());
        mesh.vertexCount = static_cast<uint32>(vertices.size());
        mesh.indexCount = static_cast<uint32>(indices.size());
        m_registry.emplace<Mesh>(light, mesh);

        Material material{};
        material.diffuse = Vector3(lightData.color.x, lightData.color.y, lightData.color.z) * lightData.energy;
        material.shader = ShaderProgram::Solid;
        m_registry.emplace<Material>(light, material);

        Matrix translation = DirectX::XMMatrixTranslationFromVector(lightData.position);
        m_registry.emplace<Transform>(light, translation, translation);
    }

    switch (lightData.type)
    {
    case LightType::Directional:
        m_registry.emplace<Tag>(light, "Directional Light");
        break;
    case LightType::Spot:
        m_registry.emplace<Tag>(light, "Spot Light");
        break;
    case LightType::Point:
        m_registry.emplace<Tag>(light, "Point Light");
        break;
    default:
        spdlog::error("Don't exist Light Type.");
        break;
    }

    return std::vector<entt::entity>{light};
}

std::vector<entt::entity> ModelImporter::LoadModel(std::string basePath, std::string filename, bool revertNormals)
{
    timer.Mark();

    std::vector<Model> model = ReadFromFile(basePath, filename, revertNormals);

    std::vector<Vertex> vertices{};
    std::vector<uint32> indices{};
    std::vector<entt::entity> entities{};
    std::unordered_map<std::string, std::vector<entt::entity>> meshNameToEntitiesMap;

    for (auto& data : model)
    {
        std::vector<entt::entity>& meshEntities = meshNameToEntitiesMap[data.meshData.name];
        assert(data.meshData.indices.size() >= 0);
        vertices = data.meshData.vertices;
        indices = data.meshData.indices;

        entt::entity e = m_registry.create();
        entities.push_back(e);
        meshEntities.push_back(e);

        Material material{};
        if (!data.materialData.albeodoTextureFilename.empty())
        {
            material.albedoTexture = g_TextureManager.LoadTexture(ToWideString(data.materialData.albeodoTextureFilename));
        }
        if (!data.materialData.normalTextureFilename.empty())
        {
            material.normalTexture = g_TextureManager.LoadTexture(ToWideString(data.materialData.normalTextureFilename));
        }
        if (!data.materialData.pbrMetallicRoughnessTextureFilename.empty())
        {
            material.metallicRoughnessTexture =
                g_TextureManager.LoadTexture(ToWideString(data.materialData.pbrMetallicRoughnessTextureFilename));
        }
        if (!data.materialData.emissiveTextureFilename.empty())
        {
            material.emissiveTexture = g_TextureManager.LoadTexture(ToWideString(data.materialData.emissiveTextureFilename));
        }

        material.shader = ShaderProgram::GBuffer;
        material.alphaCutoff = data.materialData.alphaCutoff;
        material.doubleSided = data.materialData.doubleSided;
        if (data.materialData.alphaMode == "OPAQUE")
        {
            material.alphaMode = MaterialAlphaMode::Opaque;
            material.shader = ShaderProgram::GBuffer;
        }
        // TODO : Add the AlphaMode(Blend, Mask)

        m_registry.emplace<Material>(e, material);

        Mesh meshComponent{};
        meshComponent.indexCount = static_cast<uint32>(indices.size());
        meshComponent.vertexCount = static_cast<uint32>(vertices.size());

        // TODO : Mesh가 많은 모델이 올 경우 어떻게 모델 로드를 설계해야할지 고민해야함.
        // meshComponent.startIndexLoc = static_cast<uint32>(indices.size());
        // meshComponent.baseVertexLoc = static_cast<uint32>(vertices.size());

        meshComponent.vertexBuffer =
            std::make_shared<DXBuffer>(m_device, VertexBufferDesc(vertices.size(), sizeof(Vertex)), vertices.data());
        meshComponent.indexBuffer = std::make_shared<DXBuffer>(m_device, IndexBufferDesc(indices.size(), false), indices.data());

        m_registry.emplace<Mesh>(e, meshComponent);

        // TODO : transform을 node 형태로 만들어, 모델의 각 부위의 matrix를 변환시킬 수 있도록 설정.
        Transform transform{};
        transform.currentTransform = Matrix::CreateScale(1.0f) * Matrix::CreateTranslation(Vector3(0.0f));
        transform.startingTransform = Matrix::CreateScale(1.0f) * Matrix::CreateTranslation(Vector3(0.0f));

        m_registry.emplace<Transform>(e, transform);

        AABB aabb{};
        BoundingBox _boundingBox = AABBFromVertices(vertices);
        aabb.orginalBox = _boundingBox;
        _boundingBox.Transform(_boundingBox, transform.currentTransform);
        aabb.boundingBox = _boundingBox;
        aabb.isDrawAABB = false;
        aabb.UpdateBuffer(m_device);
        m_registry.emplace<AABB>(e, aabb);
    }
    entt::entity root = m_registry.create();
    m_registry.emplace<Transform>(root);
    m_registry.emplace<Tag>(root, filename);

    Relationship relationship;
    relationship.childrenCount = static_cast<uint32>(entities.size());
    assert(relationship.childrenCount <= Relationship::MAX_CHILDREN);
    for (size_t i = 0; i < relationship.childrenCount; ++i)
    {
        relationship.children[i] = entities[i];
    }
    m_registry.emplace<Relationship>(root, relationship);

    size_t i = 0;
    for (entt::entity& e : entities)
    {
        m_registry.emplace<Tag>(e, filename + " submesh" + std::to_string(i++));
        m_registry.emplace<Relationship>(e, root);
    }

    RI_INFO("{:s} {:f}s sucessfully loaded!", filename, timer.MarkInSeconds());
    return entities;
}

} // namespace Riley