#include "ModelImporter.h"
#include "../Math/ComputeVectors.h"
#include "spdlog\spdlog.h"

namespace Riley {
Mesh ModelImporter::LoadSquare(ID3D11Device& device, const float& scale,
                               const Vector2& texScale) {

    std::vector<Vertex> vertices;
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

    for (int i = 0; i < positions.size(); ++i) {
        Vertex v;
        v.position = positions[i];
        v.normal = normals[i];
        v.texcoord = texcoords[i];
        v.tangent = tangents[i];
        v.bitangent = bitangents[i];

        vertices.push_back(v);
    }

    Mesh m;
    DXBuffer* vb = new DXBuffer(
        device, VertexBufferDesc(vertices.size(), (uint32)sizeof(Vertex)),
        vertices.data());
    DXBuffer* ib = new DXBuffer(device, IndexBufferDesc(indices.size(), false),
                                indices.data());

    m.vertexBuffer = vb;
    m.indexBuffer = ib;
    m.vertexCount = (uint32)vertices.size();
    m.indexCount = (uint32)indices.size();

    return m;
}
} // namespace Riley