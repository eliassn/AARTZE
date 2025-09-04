#pragma once

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <array>
#include <assimp/Importer.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef MESHUTILS_NO_REGISTRY
#include "World/TextureRegistry.hpp"
#include "utils/AssetLoader.hpp"
#endif

struct MeshData
{
    std::vector<float> vertices;       // x,y,z
    std::vector<float> normals;        // nx,ny,nz
    std::vector<float> colors;         // r,g,b
    std::vector<float> texCoords;      // u,v
    std::vector<int> boneIndices;      // 4 per vertex
    std::vector<float> boneWeights;    // 4 per vertex
    std::vector<uint32_t> textureIds;  // diffuse texture IDs per material
};
// Normalize vertices to fit in [-1, 1] box
inline void NormalizeVertices(std::vector<float>& vertices)
{
    if (vertices.empty()) return;

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        float x = vertices[i];
        float y = vertices[i + 1];
        float z = vertices[i + 2];
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        minZ = std::min(minZ, z);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
        maxZ = std::max(maxZ, z);
    }

    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;

    float maxExtent = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    if (maxExtent == 0.0f) return;

    float scale = 2.0f / maxExtent;
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        vertices[i] = (vertices[i] - centerX) * scale;
        vertices[i + 1] = (vertices[i + 1] - centerY) * scale;
        vertices[i + 2] = (vertices[i + 2] - centerZ) * scale;
    }
}

// Load OBJ vertices only (positions only)
inline std::vector<float> LoadObjVertices(const std::string& path)
{
    std::vector<float> vertices;
    std::ifstream file(path);
    if (!file.is_open()) return vertices;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.rfind("v ", 0) == 0)
        {
            std::istringstream iss(line.substr(2));
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    NormalizeVertices(vertices);
    return vertices;
}

// Load FBX vertices only (positions only, for fast loading)
inline std::vector<float> LoadFbxVertices(const std::string& path, bool normalize = true,
                                          float scaleFactor = 1.0f)
{
    std::vector<float> vertices;
    if (!std::filesystem::exists(path))
    {
        std::cerr << "Mesh file not found: " << path << std::endl;
        return vertices;
    }
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->HasMeshes())
    {
        std::cerr << "Assimp failed to load " << path << ": "
                  << importer.GetErrorString() << std::endl;
        return vertices;
    }

    const aiMesh* mesh = scene->mMeshes[0];
    vertices.reserve(mesh->mNumFaces * 9);

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < 3; ++j)
        {
            unsigned int index = face.mIndices[j];
            const auto& v = mesh->mVertices[index];
            vertices.push_back(v.x);
            vertices.push_back(v.y);
            vertices.push_back(v.z);

            minX = std::min(minX, v.x);
            minY = std::min(minY, v.y);
            minZ = std::min(minZ, v.z);
            maxX = std::max(maxX, v.x);
            maxY = std::max(maxY, v.y);
            maxZ = std::max(maxZ, v.z);
        }
    }

    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;
    float maxExtent = std::max({maxX - minX, maxY - minY, maxZ - minZ});

    if (normalize && maxExtent > 0.0f)
    {
        float scale = 2.0f / maxExtent * scaleFactor;
        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            vertices[i] = (vertices[i] - centerX) * scale;
            vertices[i + 1] = (vertices[i + 1] - centerY) * scale;
            vertices[i + 2] = (vertices[i + 2] - centerZ) * scale;
        }
    }
    else
    {
        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            vertices[i] *= scaleFactor;
            vertices[i + 1] *= scaleFactor;
            vertices[i + 2] *= scaleFactor;
        }
    }

    return vertices;
}

// Load full FBX model (positions, texcoords, colors)
#ifndef MESHUTILS_NO_REGISTRY
inline MeshData LoadFbxModel(const std::string& path, bool normalize = true,
                             float scaleFactor = 1.0f)
{
    MeshData data;
    if (!std::filesystem::exists(path))
    {
        std::cerr << "Mesh file not found: " << path << std::endl;
        return data;
    }
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                                    aiProcess_PreTransformVertices);
    if (!scene || !scene->HasMeshes())
    {
        std::cerr << "Assimp failed to load " << path << ": "
                  << importer.GetErrorString() << std::endl;
        return data;
    }

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    std::string directory;
    size_t slash = path.find_last_of("/\\");
    if (slash != std::string::npos) directory = path.substr(0, slash + 1);

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < 3; ++j)
            {
                unsigned int index = face.mIndices[j];
                const auto& v = mesh->mVertices[index];
                data.vertices.push_back(v.x);
                data.vertices.push_back(v.y);
                data.vertices.push_back(v.z);
                aiVector3D n(0.0f, 0.0f, 1.0f);
                if (mesh->HasNormals())
                {
                    n = mesh->mNormals[index];
                }
                data.normals.push_back(n.x);
                data.normals.push_back(n.y);
                data.normals.push_back(n.z);

                // Texcoords
                aiVector3D uv(0.0f, 0.0f, 0.0f);
                if (mesh->HasTextureCoords(0))
                {
                    uv = mesh->mTextureCoords[0][index];
                }
                data.texCoords.push_back(uv.x);
                data.texCoords.push_back(uv.y);

                // Colors
                aiColor3D color(1.0f, 1.0f, 1.0f);
                if (mesh->HasVertexColors(0))
                {
                    const auto& c = mesh->mColors[0][index];
                    color.r = c.r;
                    color.g = c.g;
                    color.b = c.b;
                }
                else if (mesh->mMaterialIndex < scene->mNumMaterials)
                {
                    scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
                }
                data.colors.push_back(color.r);
                data.colors.push_back(color.g);
                data.colors.push_back(color.b);
                for (int bw = 0; bw < 4; ++bw)
                {
                    data.boneIndices.push_back(0);
                    data.boneWeights.push_back(0.0f);
                }

                // AABB tracking
                minX = std::min(minX, v.x);
                minY = std::min(minY, v.y);
                minZ = std::min(minZ, v.z);
                maxX = std::max(maxX, v.x);
                maxY = std::max(maxY, v.y);
                maxZ = std::max(maxZ, v.z);
            }
        }

        if (mesh->mMaterialIndex < scene->mNumMaterials)
        {
            const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
            aiString texPath;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
            {
                std::string full = directory + texPath.C_Str();
                auto future = AssetLoader::LoadTextureAsync(full);
                data.textureIds.push_back(future.get());
            }
            else
            {
                data.textureIds.push_back(0);
            }
        }
    }

    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;
    float maxExtent = std::max({maxX - minX, maxY - minY, maxZ - minZ});

    if (normalize && maxExtent > 0.0f)
    {
        float scale = 2.0f / maxExtent * scaleFactor;
        for (size_t i = 0; i < data.vertices.size(); i += 3)
        {
            data.vertices[i] = (data.vertices[i] - centerX) * scale;
            data.vertices[i + 1] = (data.vertices[i + 1] - centerY) * scale;
            data.vertices[i + 2] = (data.vertices[i + 2] - centerZ) * scale;
        }
    }
    else
    {
        for (size_t i = 0; i < data.vertices.size(); i += 3)
        {
            data.vertices[i] *= scaleFactor;
            data.vertices[i + 1] *= scaleFactor;
            data.vertices[i + 2] *= scaleFactor;
        }
    }

    return data;
}
#endif // MESHUTILS_NO_REGISTRY

// Load FBX model with bones/weights (no vertex pre-transform)
#ifndef MESHUTILS_NO_REGISTRY
inline MeshData LoadFbxSkinnedModel(const std::string& path)
{
    MeshData data;
    if (!std::filesystem::exists(path))
    {
        std::cerr << "Mesh file not found: " << path << std::endl;
        return data;
    }
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->HasMeshes())
    {
        std::cerr << "Assimp failed to load " << path << ": "
                  << importer.GetErrorString() << std::endl;
        return data;
    }

    size_t vertexOffset = 0;
    std::unordered_map<std::string, int> boneMap;
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
        {
            const auto& vert = mesh->mVertices[v];
            data.vertices.push_back(vert.x);
            data.vertices.push_back(vert.y);
            data.vertices.push_back(vert.z);
            aiVector3D n(0.0f, 0.0f, 1.0f);
            if (mesh->HasNormals()) n = mesh->mNormals[v];
            data.normals.push_back(n.x);
            data.normals.push_back(n.y);
            data.normals.push_back(n.z);
            aiVector3D uv(0.0f, 0.0f, 0.0f);
            if (mesh->HasTextureCoords(0)) uv = mesh->mTextureCoords[0][v];
            data.texCoords.push_back(uv.x);
            data.texCoords.push_back(uv.y);
            data.colors.push_back(1.0f);
            data.colors.push_back(1.0f);
            data.colors.push_back(1.0f);
            for (int bw = 0; bw < 4; ++bw)
            {
                data.boneIndices.push_back(0);
                data.boneWeights.push_back(0.0f);
            }
        }

        std::vector<std::array<int, 4>> tmpIndices(mesh->mNumVertices);
        std::vector<std::array<float, 4>> tmpWeights(mesh->mNumVertices);
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            const aiBone* bone = mesh->mBones[b];
            int boneId;
            auto it = boneMap.find(bone->mName.C_Str());
            if (it == boneMap.end())
            {
                boneId = boneMap.size();
                boneMap[bone->mName.C_Str()] = boneId;
            }
            else
                boneId = it->second;

            for (unsigned int w = 0; w < bone->mNumWeights; ++w)
            {
                unsigned int vid = bone->mWeights[w].mVertexId;
                float weight = bone->mWeights[w].mWeight;
                for (int i = 0; i < 4; ++i)
                {
                    if (tmpWeights[vid][i] == 0.0f)
                    {
                        tmpIndices[vid][i] = boneId;
                        tmpWeights[vid][i] = weight;
                        break;
                    }
                }
            }
        }
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
        {
            size_t base = vertexOffset + v;
            for (int i = 0; i < 4; ++i)
            {
                data.boneIndices[base * 4 + i] = tmpIndices[v][i];
                data.boneWeights[base * 4 + i] = tmpWeights[v][i];
            }
        }
        vertexOffset += mesh->mNumVertices;

        if (mesh->mMaterialIndex < scene->mNumMaterials)
        {
            const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
            aiString texPath;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
            {
                std::string full = std::string(texPath.C_Str());
                auto future = AssetLoader::LoadTextureAsync(full);
                data.textureIds.push_back(future.get());
            }
            else
            {
                data.textureIds.push_back(0);
            }
        }
    }
    return data;
}
#endif // MESHUTILS_NO_REGISTRY

// Load glTF model (positions, texcoords, colors)
#ifndef MESHUTILS_NO_REGISTRY
inline MeshData LoadGltfModel(const std::string& path, bool normalize = true,
                              float scaleFactor = 1.0f)
{
    MeshData data;
    if (!std::filesystem::exists(path))
    {
        std::cerr << "Mesh file not found: " << path << std::endl;
        return data;
    }
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                                    aiProcess_PreTransformVertices);
    if (!scene || !scene->HasMeshes())
    {
        std::cerr << "Assimp failed to load " << path << ": "
                  << importer.GetErrorString() << std::endl;
        return data;
    }

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    std::string directory;
    size_t slash = path.find_last_of("/\\");
    if (slash != std::string::npos) directory = path.substr(0, slash + 1);

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < 3; ++j)
            {
                unsigned int index = face.mIndices[j];
                const auto& v = mesh->mVertices[index];
                data.vertices.push_back(v.x);
                data.vertices.push_back(v.y);
                data.vertices.push_back(v.z);
                aiVector3D n(0.0f, 0.0f, 1.0f);
                if (mesh->HasNormals())
                {
                    n = mesh->mNormals[index];
                }
                data.normals.push_back(n.x);
                data.normals.push_back(n.y);
                data.normals.push_back(n.z);

                aiVector3D uv(0.0f, 0.0f, 0.0f);
                if (mesh->HasTextureCoords(0))
                {
                    uv = mesh->mTextureCoords[0][index];
                }
                data.texCoords.push_back(uv.x);
                data.texCoords.push_back(uv.y);

                aiColor3D color(1.0f, 1.0f, 1.0f);
                if (mesh->HasVertexColors(0))
                {
                    const auto& c = mesh->mColors[0][index];
                    color.r = c.r;
                    color.g = c.g;
                    color.b = c.b;
                }
                else if (mesh->mMaterialIndex < scene->mNumMaterials)
                {
                    scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
                }
                data.colors.push_back(color.r);
                data.colors.push_back(color.g);
                data.colors.push_back(color.b);
                for (int bw = 0; bw < 4; ++bw)
                {
                    data.boneIndices.push_back(0);
                    data.boneWeights.push_back(0.0f);
                }

                minX = std::min(minX, v.x);
                minY = std::min(minY, v.y);
                minZ = std::min(minZ, v.z);
                maxX = std::max(maxX, v.x);
                maxY = std::max(maxY, v.y);
                maxZ = std::max(maxZ, v.z);
            }
        }

        if (mesh->mMaterialIndex < scene->mNumMaterials)
        {
            const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
            aiString texPath;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
            {
                std::string full = directory + texPath.C_Str();
                uint32_t texId = RegisterTexture(full);
                data.textureIds.push_back(texId);
            }
            else
            {
                data.textureIds.push_back(0);
            }
        }
    }

    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;
    float maxExtent = std::max({maxX - minX, maxY - minY, maxZ - minZ});

    if (normalize && maxExtent > 0.0f)
    {
        float scale = 2.0f / maxExtent * scaleFactor;
        for (size_t i = 0; i < data.vertices.size(); i += 3)
        {
            data.vertices[i] = (data.vertices[i] - centerX) * scale;
            data.vertices[i + 1] = (data.vertices[i + 1] - centerY) * scale;
            data.vertices[i + 2] = (data.vertices[i + 2] - centerZ) * scale;
        }
    }
    else
    {
        for (size_t i = 0; i < data.vertices.size(); i += 3)
        {
            data.vertices[i] *= scaleFactor;
            data.vertices[i + 1] *= scaleFactor;
            data.vertices[i + 2] *= scaleFactor;
        }
    }

    return data;
}
#endif // MESHUTILS_NO_REGISTRY

// Generic Assimp-based loader for various formats (fbx, obj, dae, etc.)
#ifndef MESHUTILS_NO_REGISTRY
inline MeshData LoadMeshAny(const std::string& path, bool normalize = true, float scaleFactor = 1.0f)
{
    MeshData data;
    if (!std::filesystem::exists(path)) { std::cerr << "Mesh file not found: " << path << std::endl; return data; }
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);
    if (!scene || !scene->HasMeshes()) { std::cerr << "Assimp failed to load " << path << ": " << importer.GetErrorString() << std::endl; return data; }

    float minX=std::numeric_limits<float>::max(), minY=minX, minZ=minX;
    float maxX=std::numeric_limits<float>::lowest(), maxY=maxX, maxZ=maxX;

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < 3; ++j)
            {
                unsigned int index = face.mIndices[j];
                const auto& v = mesh->mVertices[index];
                data.vertices.push_back(v.x); data.vertices.push_back(v.y); data.vertices.push_back(v.z);
                aiVector3D n(0.0f,0.0f,1.0f); if (mesh->HasNormals()) n = mesh->mNormals[index];
                data.normals.push_back(n.x); data.normals.push_back(n.y); data.normals.push_back(n.z);
                aiVector3D uv(0.0f,0.0f,0.0f); if (mesh->HasTextureCoords(0)) uv = mesh->mTextureCoords[0][index];
                data.texCoords.push_back(uv.x); data.texCoords.push_back(uv.y);
                aiColor3D color(1.0f,1.0f,1.0f); if (mesh->HasVertexColors(0)) { const auto& c = mesh->mColors[0][index]; color = aiColor3D(c.r,c.g,c.b); }
                data.colors.push_back(color.r); data.colors.push_back(color.g); data.colors.push_back(color.b);
                for (int bw=0; bw<4; ++bw) { data.boneIndices.push_back(0); data.boneWeights.push_back(0.0f); }
                minX=std::min(minX,v.x); minY=std::min(minY,v.y); minZ=std::min(minZ,v.z);
                maxX=std::max(maxX,v.x); maxY=std::max(maxY,v.y); maxZ=std::max(maxZ,v.z);
            }
        }
    }

    float centerX=(minX+maxX)*0.5f, centerY=(minY+maxY)*0.5f, centerZ=(minZ+maxZ)*0.5f;
    float maxExtent = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    if (normalize && maxExtent > 0.0f)
    {
        float scale = 2.0f/maxExtent * scaleFactor;
        for (size_t i=0;i<data.vertices.size();i+=3)
        {
            data.vertices[i]   = (data.vertices[i]   - centerX) * scale;
            data.vertices[i+1] = (data.vertices[i+1] - centerY) * scale;
            data.vertices[i+2] = (data.vertices[i+2] - centerZ) * scale;
        }
    }
    else
    {
        for (size_t i=0;i<data.vertices.size();i+=3)
        {
            data.vertices[i]   *= scaleFactor;
            data.vertices[i+1] *= scaleFactor;
            data.vertices[i+2] *= scaleFactor;
        }
    }
    return data;
}
#endif // MESHUTILS_NO_REGISTRY
