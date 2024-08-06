#pragma once

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "Components.h"

namespace Riley
{

    struct 

class ModelLoader
{
  public:
    ModelLoader() = default;
    ~ModelLoader() = default;

    void Load(std::string basePath, std::string filename, bool revertNormals = false);
    void ProcessNode(aiNode* node, const aiScene* scene, Matrix tr);

    std::pair<Mesh, Material> ProcessMesh(aiMesh* mesh, const aiScene* scene);

    std::string ReadFilename(aiMaterial* material, aiTextureType type);

  public:
    std::string basePath;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    bool m_isGLTF = false;
    bool m_revertNormals = false;
};
} // namespace Riley