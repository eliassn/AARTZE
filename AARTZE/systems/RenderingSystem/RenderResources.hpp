#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

struct MeshData; // from utils/MeshUtils.hpp

struct MeshGPU
{
    unsigned vao{0};
    unsigned vbo{0};
    int vertexCount{0};
    int stride{0};
};

namespace RenderResources
{
// Create or fetch a GPU mesh for given id using provided data
void UploadMesh(uint32_t meshId, const MeshData& data);
const MeshGPU* GetMesh(uint32_t meshId);
void Clear();
}

