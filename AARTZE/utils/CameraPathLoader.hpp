#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <array>
#include <string>
#include <vector>
#include <iostream>

inline std::vector<std::array<float, 3>> LoadCameraPath(const std::string& file)
{
    std::vector<std::array<float, 3>> points;
    if (!std::filesystem::exists(file))
    {
        std::cerr << "Camera path file not found: " << file << std::endl;
        return points;
    }
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(file, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (!scene)
    {
        std::cerr << "Assimp failed to load " << file << ": "
                  << importer.GetErrorString() << std::endl;
        return points;
    }

    const aiCamera* camera = nullptr;
    if (scene->HasCameras()) camera = scene->mCameras[0];
    if (!camera || !scene->HasAnimations()) return points;

    const aiAnimation* anim = scene->mAnimations[0];
    const aiNodeAnim* camChannel = nullptr;
    for (unsigned int c = 0; c < anim->mNumChannels; ++c)
    {
        const aiNodeAnim* channel = anim->mChannels[c];
        if (channel->mNodeName == camera->mName)
        {
            camChannel = channel;
            break;
        }
    }
    if (!camChannel) return points;

    for (unsigned int i = 0; i < camChannel->mNumPositionKeys; ++i)
    {
        const aiVector3D& v = camChannel->mPositionKeys[i].mValue;
        points.push_back({v.x, v.y, v.z});
    }
    return points;
}
