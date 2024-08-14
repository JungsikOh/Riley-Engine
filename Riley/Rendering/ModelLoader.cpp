#include "ModelLoader.h"
#include "../Math/ComputeVectors.h"
#include "../Utilities/FileUtil.h"
#include "../Utilities/StringUtil.h"

namespace Riley
{

void ModelLoader::Load(std::string basePath, std::string filename, bool revertNormals)
{
    if (GetExtension(filename) == ".gltf")
    {
        m_isGLTF = true;
        m_revertNormals = revertNormals;
    }

    this->basePath = basePath;

    Assimp::Importer importer;
    // aiProcess_Triangulate : 모델이 삼각형으로 이루져잇지 않다면, 모든 Primitive 도형들을 삼각형으로 변환시킨다.
    // aiProcess_ConvertToLeftHanded : DirectX는 왼손좌표계이므로, 해당 옵션을 설정.
    const aiScene* pScene = importer.ReadFile(this->basePath + filename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!pScene)
    {
        assert(false && "Assimp failed to Read File");
    }
    else
    {
        Matrix tr = Matrix::Identity;
        ProcessNode(pScene->mRootNode, pScene, tr);
    }
}

void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene, Matrix tr)
{
    Matrix m;
    ai_real* temp = &node->mTransformation.a1;
    float* mTemp = &m._11;
    for (int t = 0; t < 16; ++t)
    {
        mTemp[t] = float(temp[t]);
    }
    m = m.Transpose() * tr;

    for (uint32 i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto newModelData = this->ProcessMesh(mesh, scene);

        for (auto& v : newModelData.meshData.vertices)
        {
            v.position = Vector3::Transform(v.position, m);
        }

        model.push_back(newModelData);

        for (uint32 i = 0; i < node->mNumChildren; ++i)
        {
            this->ProcessNode(node->mChildren[i], scene, m);
        }
    }
}

Model ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;

    for (uint32 i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32 j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    for (uint32 i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        if (m_isGLTF)
        {
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }
        else
        {
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        if (m_revertNormals)
        {
            vertex.normal *= -1.0f;
        }
        vertex.normal.Normalize();

        if (mesh->mTextureCoords[0])
        {
            vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
            vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
        }

        if (mesh->mTangents)
        {
            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;
        }
        vertex.tangent.Normalize();

        if (mesh->mBitangents)
        {
            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;
        }

        vertices.push_back(vertex);
    }

    Model newModelData;
    newModelData.meshData.vertices = vertices;
    newModelData.meshData.indices = indices;
    newModelData.meshData.name = mesh->mName.C_Str();

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        newModelData.materialData.name = material->GetName().C_Str();
        float alphaCutoff = 0.5f;
        if (AI_SUCCESS == material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff))
        {
            newModelData.materialData.alphaCutoff = alphaCutoff; // Default: alphaCutoff = 0.5f
        }
        std::string alphaMode;
        if (AI_SUCCESS == material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode))
        {
            newModelData.materialData.alphaMode = alphaMode;
        }

        // https://github.com/assimp/assimp/blob/master/include/assimp/material.h
        newModelData.materialData.albeodoTextureFilename = ReadFilename(material, aiTextureType_BASE_COLOR);
        if (newModelData.materialData.albeodoTextureFilename.empty())
        {
            newModelData.materialData.albeodoTextureFilename = ReadFilename(material, aiTextureType_DIFFUSE);
        }
        newModelData.materialData.normalTextureFilename = ReadFilename(material, aiTextureType_NORMALS);
        newModelData.materialData.pbrMetallicRoughnessTextureFilename = ReadFilename(material, aiTextureType_METALNESS);
        if (newModelData.materialData.pbrMetallicRoughnessTextureFilename.empty())
        {
            newModelData.materialData.pbrMetallicRoughnessTextureFilename = ReadFilename(material, aiTextureType_DIFFUSE_ROUGHNESS);
        }
        newModelData.materialData.emissiveTextureFilename = ReadFilename(material, aiTextureType_EMISSIVE);
        newModelData.materialData.pbrClearcoatTextureFilename = ReadFilename(material, aiTextureType_CLEARCOAT);
        newModelData.materialData.pbrSheenTextureFilename = ReadFilename(material, aiTextureType_SHEEN);
    }

    return newModelData;
}

std::string ModelLoader::ReadFilename(aiMaterial* material, aiTextureType type)
{
    if (material->GetTextureCount(type) > 0)
    {
        aiString filepath;
        material->GetTexture(type, 0, &filepath);

        std::string fullPath = this->basePath + GetFilename(filepath.C_Str());
        return fullPath;
    }
    else
    {
        return "";
    }
}

} // namespace Riley