#include "ModelLoader.h"
#include "../Utilities/FileUtil.h"
#include "../Utilities/StringUtil.h"

namespace Riley
{
void ModelLoader::Load(std::string basePath, std::string filename, bool revertNormals)
{
    if (GetExtension(filename) == ".gltf")
    {
        m_isGLTF = true;
        m_revertNormals = true;
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

// TODO
// TinyGlTf를 보면, vertex와 같은 것들을 vector로 만든뒤에 넘겨준다.
// 그리고 그 vector를 모델을 만들 때 재료로써 사용하게 된다.
// 각 면마다, vertices, indices, albedo, metallic, roughness, normal, emissive, height가 존재한다.
// 이것들을 구조체로 넘겨준뒤, modelImporter에서 작업하는게 맞는건가?
// 

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
        auto [newMesh, newMatarial] = this->ProcessMesh(mesh, scene);

        for (auto& v : newMesh.v)
    }
}

}