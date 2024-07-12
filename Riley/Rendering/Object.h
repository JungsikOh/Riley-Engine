#pragma once
#include "../Core/ECS.h"
#include <memory>

namespace Riley {
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
    Vector3 tangent;
};

struct Material {
    float albedoFactor = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    float emissiveFactor = 1.0f;

    float alphaCutoff = 0.5f;
    bool doubleSided = false;

    Vector3 diffuse = Vector3(1, 1, 1);
};

struct Mesh {
    std::shared_ptr<ID3D11Buffer*> vertexBuffer = nullptr;
    std::shared_ptr<ID3D11Buffer*> indexBuffer = nullptr;
    std::shared_ptr<ID3D11Buffer*> instanceBuffer = nullptr;

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
    void Draw(ID3D11DeviceContext* _context,
              D3D11_PRIMITIVE_TOPOLOGY override_topology) const;
};

} // namespace Riley