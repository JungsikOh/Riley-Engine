#pragma once
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXShaderProgram.h"
#include "../Rendering/ShaderManager.h"
#include "../Rendering/TextureManager.h"

#define COMPONENT

namespace Riley
{

static constexpr float pie = 3.141592654f;

struct Vertex
{
   Vector3 position;
   Vector3 normal;
   Vector2 texcoord;
   Vector3 tangent;
   Vector3 bitangent;
};

struct SimpleVertex
{
   Vector3 position;
   Vector2 texcoord;
};

struct COMPONENT Transform
{
   Matrix startingTransform = Matrix::Identity;
   Matrix currentTransform = Matrix::Identity;
};

struct COMPONENT Material
{
    TextureHandle albedoTexture = INVALID_TEXTURE_HANDLE;
    TextureHandle normalTexture = INVALID_TEXTURE_HANDLE;
    TextureHandle metallicRoughnessTexture = INVALID_TEXTURE_HANDLE;
    TextureHandle emissiveTexture = INVALID_TEXTURE_HANDLE;

   float albedoFactor = 1.0f;
   float metallicFactor = 0.001f;
   float roughnessFactor = 1.0f;
   float emissiveFactor = 1.0f;

   MaterialAlphaMode alphaMode;
   float alphaCutoff = 0.5f;
   bool doubleSided = false;
   bool useNormalMap = true;

   Vector3 diffuse = Vector3(1, 1, 1);
   ShaderProgram shader = ShaderProgram::UnKnown;
};

struct COMPONENT Mesh
{
   std::shared_ptr<DXBuffer> vertexBuffer = nullptr;
   std::shared_ptr<DXBuffer> indexBuffer = nullptr;
   std::shared_ptr<DXBuffer> instanceBuffer = nullptr;

   // vertex buffer and index buffer
   uint32 vertexCount = 0;
   uint32 startVertexLoc = 0; // Index of the first vertex

   uint32 indexCount = 0;
   // the location of the first index read by the GPU from ib
   uint32 startIndexLoc = 0;
   // A value added to each index before reading a vertex from the vb
   int32 baseVertexLoc = 0;

   // instancing
   uint32 instanceCount = 1;
   // A value added to each idx before reading per-instance data from a vb
   uint32 startInstanceLoc = 0;

   D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

   void Draw(ID3D11DeviceContext* _context) const;
   void Draw(ID3D11DeviceContext* _context, D3D11_PRIMITIVE_TOPOLOGY override_topology) const;
};

struct COMPONENT Light
{
   Vector4 position = Vector4(0, 10, 0, 1);
   Vector4 direction = Vector4(0, -1, 0, 0);
   Vector4 color = Vector4(1, 1, 1, 1);
   float energy = 1.0f;
   float range = 100.0f;
   LightType type = LightType::Directional;
   float outer_cosine = cos(pie / 4);
   float inner_cosine = cos(pie / 8);
   float radius = 0.0f;
   float haloStrength = 1.0f;
   bool castShadows = true;
   bool useCascades = true;
   bool active = true;
   /*bool volumetric = false;
   float volumetric_strength = 0.03f;
   bool screen_space_contact_shadows = false;
   float sscs_thickness = 0.5f;
   float sscs_max_ray_distance = 0.05f;
   float sscs_max_depth_distance = 200.0f;
   bool lens_flare = false;
   bool god_rays = false;
   float godrays_decay = 0.825f;
   float godrays_weight = 0.25f;
   float godrays_density = 0.975f;
   float godrays_exposure = 2.0f;*/
};

struct COMPONENT AABB
{
   DirectX::BoundingBox boundingBox;
   DirectX::BoundingBox orginalBox;
   bool isCameraVisible = true;
   bool isLightVisible = false;
   bool isDrawAABB = false;
   std::shared_ptr<DXBuffer> aabbVertexBuffer = nullptr;
   std::shared_ptr<DXBuffer> aabbIndexBuffer = nullptr;

   void UpdateBuffer(ID3D11Device* device)
   {
      Vector3 corners[8];
      boundingBox.GetCorners(corners);
      SimpleVertex vertices[] = {SimpleVertex{corners[0]}, SimpleVertex{corners[1]}, SimpleVertex{corners[2]},
                                 SimpleVertex{corners[3]}, SimpleVertex{corners[4]}, SimpleVertex{corners[5]},
                                 SimpleVertex{corners[6]}, SimpleVertex{corners[7]}};
      aabbVertexBuffer = std::make_shared<DXBuffer>(device, VertexBufferDesc(ARRAYSIZE(vertices), sizeof(SimpleVertex)), vertices);
   }

   void SetAABBIndexBuffer(ID3D11Device* device)
   {
      const uint16 aabb_indices[] = {0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 5, 6, 6, 7, 7, 4};
      aabbIndexBuffer = std::make_shared<DXBuffer>(device, IndexBufferDesc(ARRAYSIZE(aabb_indices), true), aabb_indices);
   }
};

struct COMPONENT Relationship
{
    static constexpr size_t MAX_CHILDREN = 2048;
    entt::entity parent = entt::null;
    uint32 childrenCount = 0;
    entt::entity children[MAX_CHILDREN] = {entt::null};
};

struct COMPONENT Tag
{
   std::string name = "default";
};

} // namespace Riley