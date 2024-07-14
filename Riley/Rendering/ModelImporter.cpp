#include "ModelImporter.h"
#include "../Math/ComputeVectors.h"
#include "spdlog\spdlog.h"

namespace Riley {
Model ModelImporter::LoadSquare(ID3D11Device* device,
                                            const float& scale,
                                            const Vector2& texScale) {
    Mesh mesh;

    std::vector<Vertex> vertices;
    Vertex v;
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

    std::vector<uint32> indices{0, 1, 2, 0, 2, 3};

    ComputeAndSetTangets(indices, positions, normals, texcoords, tangents,
                         bitangents);

    for (uint32 i = 0; i < positions.size(); ++i) {
        Vertex v;
        v.position = positions[i];
        v.normal = normals[i];
        v.texcoord = texcoords[i];
        v.tangent = tangents[i];
        v.bitangent = bitangents[i];

        vertices.push_back(v);
    }

    std::vector<Mesh> meshes;
    Mesh m;
    DXBuffer vb(device, VertexBufferDesc(vertices.size(), sizeof(vertices)),
                vertices.data());
    DXBuffer ib(device, IndexBufferDesc(indices.size(), false), indices.data());

    for (uint32 i = 0; i < vertices.size(); ++i) {
        m.vertexBuffer = &vb;
        m.indexBuffer = &ib;
        m.vertexCount = (uint32)vertices.size();
        m.indexCount = (uint32)indices.size();
        meshes.push_back(m);
    }

    ObjectConsts obj;
    obj.worldRow = Matrix::CreateTranslation(Vector3(1.0f)).Transpose();
    obj.worldInvTransposeRow = obj.worldRow.Invert();

    MaterialConsts materialConst;
    materialConst.ambient = Vector3(0.2f, 0.2f, 0.8f);

    Model model(device, "Squere", obj, materialConst);

    return model;
}
} // namespace Riley