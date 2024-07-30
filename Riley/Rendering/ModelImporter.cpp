#include "ModelImporter.h"
#include "../Math/ComputeVectors.h"
#include "../Math/BoundingVolume.h"

namespace Riley
{

ModelImporter::ModelImporter(ID3D11Device* device, entt::registry& reg)
    : m_device(device)
    , m_registry(reg)
{}

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
   material.shader = ShaderProgram::ForwardPhong;

   m_registry.emplace<Material>(entity, material);

   Transform transform{};
   transform.startingTransform =
     Matrix::CreateScale(scale) * Matrix::CreateRotationX(DirectX::XMConvertToRadians(rotate)) * Matrix::CreateTranslation(pos);
   transform.currentTransform =
     Matrix::CreateScale(scale) * Matrix::CreateRotationX(DirectX::XMConvertToRadians(rotate)) * Matrix::CreateTranslation(pos);

   m_registry.emplace<Transform>(entity, transform);

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

   BoundingBox _boundingBox = AABBFromVertices(vertices);
   _boundingBox.Transform(_boundingBox, transform.currentTransform);

   AABB aabb{};
   aabb.boundingBox = _boundingBox;
   aabb.isDrawAABB = true;
   aabb.UpdateBuffer(m_device);
   m_registry.emplace<AABB>(entity, aabb);

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

         std::vector<uint32> indices{
           0,  1,  2,  0,  2,  3,  // À­¸é
           4,  5,  6,  4,  6,  7,  // ¾Æ·§
           8,  9,  10, 8,  10, 11, // ¾Õ¸é
           12, 13, 14, 12, 14, 15, // µÞ¸é
           16, 17, 18, 16, 18, 19, // ¿ÞÂÊ
           20, 21, 22, 20, 22, 23  // ¿À¸¥
         };

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

} // namespace Riley