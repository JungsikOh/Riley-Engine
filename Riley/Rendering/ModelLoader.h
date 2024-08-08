#pragma once

#include <assimp\Importer.hpp>
#include <assimp\pbrmaterial.h>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "Components.h"
#include "MeshData.h"

namespace Riley
{

struct Model;

class ModelLoader
{
  public:
    ModelLoader() = default;
    ~ModelLoader() = default;

    void Load(std::string basePath, std::string filename, bool revertNormals = false);
    void ProcessNode(aiNode* node, const aiScene* scene, Matrix tr);
    Model ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::string ReadFilename(aiMaterial* material, aiTextureType type);

  public:
    std::string basePath;
    std::vector<Model> model;
    bool m_isGLTF = false;
    bool m_revertNormals = false;
};

// Normalize Vertex coordinates
static std::vector<Model> ReadFromFile(std::string basePath, std::string filename, bool revertNormals)
{
    using namespace DirectX;

    ModelLoader modelLoader;
    modelLoader.Load(basePath, filename, revertNormals);
    std::vector<Model> model = modelLoader.model;

    Vector3 vmin(1000, 1000, 1000);
    Vector3 vmax(-1000, -1000, -1000);
    for (auto& data : model)
    {
        for (auto& v : data.meshData.vertices)
        {
            vmin.x = XMMin(vmin.x, v.position.x);
            vmin.y = XMMin(vmin.y, v.position.y);
            vmin.z = XMMin(vmin.z, v.position.z);
            vmax.x = XMMax(vmax.x, v.position.x);
            vmax.y = XMMax(vmax.y, v.position.y);
            vmax.z = XMMax(vmax.z, v.position.z);
        }
    }

    float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
    float dl = XMMax(XMMax(dx, dy), dz);
    float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f, cz = (vmax.z + vmin.z) * 0.5f;

    for (auto& data : model)
    {
        for (auto& v : data.meshData.vertices)
        {
            v.position.x = (v.position.x - cx) / dl;
            v.position.y = (v.position.y - cy) / dl;
            v.position.z = (v.position.z - cz) / dl;
        }
    }

    return model;
}

} // namespace Riley