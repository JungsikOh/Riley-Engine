#pragma once
#include "../Core/Rendering.h"
#include "MathTypes.h"
#include <vector>

namespace Riley {
static void ComputeAndSetTangets(_In_ std::vector<uint32> const indices,
                                 _In_ std::vector<Vector3> const positions,
                                 _In_ std::vector<Vector3> const normals,
                                 _In_ std::vector<Vector2> const texcoords,
                                 _Out_ std::vector<Vector3>& tangents,
                                 _Out_ std::vector<Vector3>& bitangents) {
    std::vector<Vector3> _tangents(positions.size(), Vector3(0.0f));
    std::vector<Vector3> _bitangents(positions.size(), Vector3(0.0f));
    std::vector<float> _weights(positions.size(), 0.0f);

    for (uint32 i = 0; i <= indices.size() - 3; i += 3) {
        uint32 i0 = indices[i + 0];
        uint32 i1 = indices[i + 1];
        uint32 i2 = indices[i + 2];

        Vector3 p0 = positions[i0];
        Vector3 p1 = positions[i1];
        Vector3 p2 = positions[i2];

        Vector2 w0 = texcoords[i0];
        Vector2 w1 = texcoords[i1];
        Vector2 w2 = texcoords[i2];

        // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
        Vector3 e1(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z);
        Vector3 e2(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z);

        float x1 = w1.x - w0.x, x2 = w2.x - w0.x; // delta1
        float y1 = w1.y - w0.y, y2 = w2.y - w0.y; // delta2
        float r = 1.0f / (x1 * y2 - x2 * y1);

        Vector3 _tangent;
        _tangent.x = (y2 * e1.x - y1 * e2.x) * r;
        _tangent.y = (y2 * e1.y - y1 * e2.y) * r;
        _tangent.z = (y2 * e1.z - y1 * e2.z) * r;

        Vector3 _bitangent;
        _bitangent.x = (x2 * e1.x + x1 * e2.x) * r;
        _bitangent.y = (x2 * e1.y + x1 * e2.y) * r;
        _bitangent.z = (x2 * e1.z + x1 * e2.z) * r;

        _tangents[i0] += _tangent;
        _tangents[i1] += _tangent;
        _tangents[i2] += _tangent;
        _bitangents[i0] += _bitangent;
        _bitangents[i1] += _bitangent;
        _bitangents[i2] += _bitangent;
        _weights[i0] += 1.0f;
        _weights[i1] += 1.0f;
        _weights[i2] += 1.0f;
    }

    for (uint32 i = 0; i < positions.size(); ++i) {
        if (_weights[i] > 0.0f) {
            Vector3 _tangent = _tangents[i] / _weights[i];
            _tangent.Normalize();
            tangents.push_back(_tangent);

            Vector3 _bitangent = _bitangents[i] / _weights[i];
            _bitangent.Normalize();
            bitangents.push_back(_bitangent);
        }
    }
}

static void ComputeAndSetNormals(_In_ std::vector<uint32> const indices,
                                 _In_ std::vector<Vector3> const positions,
                                 _In_ std::vector<Vector2> const texcoords,
                                 _Out_ std::vector<Vector3>& normals) {
    std::vector<Vector3> _normals(positions.size(), Vector3(0.0f));
    std::vector<float> _weights(positions.size(), 0.0f);

    for (uint32 i = 0; i <= indices.size() - 3; i += 3) {
        uint32 i0 = indices[i + 0];
        uint32 i1 = indices[i + 1];
        uint32 i2 = indices[i + 2];

        Vector3 v0 = positions[i0];
        Vector3 v1 = positions[i1];
        Vector3 v2 = positions[i2];

        Vector3 faceNormal = (v1 - v0).Cross(v2 - v0);

        _normals[i0] += faceNormal;
        _normals[i1] += faceNormal;
        _normals[i2] += faceNormal;
        _weights[i0] += 1.0f;
        _weights[i1] += 1.0f;
        _weights[i2] += 1.0f;
    }

    for (uint32 i = 0; i < positions.size(); ++i) {
        if (_weights[i] > 0.0f) {
            Vector3 _normal = _normals[i] / _weights[i];
            _normal.Normalize();
            normals.push_back(_normal);
        }
    }
}
} // namespace Riley