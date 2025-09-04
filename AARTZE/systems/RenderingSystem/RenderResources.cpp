#include "RenderResources.hpp"
#include <glad/glad.h>
#include <cstring>

#include "utils/MeshUtils.hpp"

namespace
{
std::unordered_map<uint32_t, MeshGPU> gMeshes;
}

namespace RenderResources
{
void UploadMesh(uint32_t meshId, const MeshData& data)
{
    if (data.vertices.empty()) return;
    MeshGPU& gpu = gMeshes[meshId];
    if (gpu.vao == 0)
    {
        glGenVertexArrays(1, &gpu.vao);
        glGenBuffers(1, &gpu.vbo);
    }
    glBindVertexArray(gpu.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gpu.vbo);

    // Interleave attributes: pos(3), normal(3), color(3), uv(2)
    const size_t vcount = data.vertices.size() / 3;
    std::vector<float> interleaved;
    interleaved.reserve(vcount * 11);
    for (size_t i = 0; i < vcount; ++i)
    {
        // position
        interleaved.push_back(data.vertices[i * 3 + 0]);
        interleaved.push_back(data.vertices[i * 3 + 1]);
        interleaved.push_back(data.vertices[i * 3 + 2]);
        // normal
        if (data.normals.size() >= (i + 1) * 3)
        {
            interleaved.push_back(data.normals[i * 3 + 0]);
            interleaved.push_back(data.normals[i * 3 + 1]);
            interleaved.push_back(data.normals[i * 3 + 2]);
        }
        else
        {
            interleaved.push_back(0.0f);
            interleaved.push_back(0.0f);
            interleaved.push_back(1.0f);
        }
        // color
        if (data.colors.size() >= (i + 1) * 3)
        {
            interleaved.push_back(data.colors[i * 3 + 0]);
            interleaved.push_back(data.colors[i * 3 + 1]);
            interleaved.push_back(data.colors[i * 3 + 2]);
        }
        else
        {
            interleaved.push_back(1.0f);
            interleaved.push_back(1.0f);
            interleaved.push_back(1.0f);
        }
        // uv
        if (data.texCoords.size() >= (i + 1) * 2)
        {
            interleaved.push_back(data.texCoords[i * 2 + 0]);
            interleaved.push_back(data.texCoords[i * 2 + 1]);
        }
        else
        {
            interleaved.push_back(0.0f);
            interleaved.push_back(0.0f);
        }
    }
    gpu.stride = 11 * sizeof(float);
    gpu.vertexCount = static_cast<int>(vcount);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, gpu.stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, gpu.stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, gpu.stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, gpu.stride, (void*)(9 * sizeof(float)));

    glBindVertexArray(0);
}

const MeshGPU* GetMesh(uint32_t meshId)
{
    auto it = gMeshes.find(meshId);
    if (it == gMeshes.end()) return nullptr;
    return &it->second;
}

void Clear()
{
    for (auto& [id, m] : gMeshes)
    {
        if (m.vbo) glDeleteBuffers(1, &m.vbo);
        if (m.vao) glDeleteVertexArrays(1, &m.vao);
    }
    gMeshes.clear();
}
}

