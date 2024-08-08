#pragma once
#include "Components.h"

namespace Riley
{

struct MeshData
{
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;

    MeshData() = default;
    ~MeshData() = default;
    MeshData(const MeshData&) = default;
    MeshData(MeshData&&) = default;
    MeshData& operator=(const MeshData&) = default;
    MeshData& operator=(MeshData&&) = default;
    bool operator==(const MeshData&) const = default;
};

struct MaterialData
{
    std::string name;
    std::string alphaMode = "OPAQUE";
    float alphaCutoff = 0.5f;
    bool doubleSided = false;

    std::string albeodoTextureFilename;
    std::string normalTextureFilename;
    std::string emissiveTextureFilename;
    std::string occlusionTextureFilename;
    std::string pbrMetallicRoughnessTextureFilename;
    std::string pbrClearcoatTextureFilename;
    std::string pbrSheenTextureFilename;

    MaterialData() = default;
    ~MaterialData() = default;
    MaterialData(const MaterialData&) = default;
    MaterialData(MaterialData&&) = default;
    MaterialData& operator=(const MaterialData&) = default;
    MaterialData& operator=(MaterialData&&) = default;
    bool operator==(const MaterialData&) const = default;
};

/*
 * @brief It's available from C++20. Because of '= default'
 */
struct Model
{
    MeshData meshData;
    MaterialData materialData;
};

} // namespace Riley