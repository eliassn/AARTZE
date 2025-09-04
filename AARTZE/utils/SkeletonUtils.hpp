#pragma once
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <vector>

#include "SkeletonComponent.hpp"
#include "components/AnimationComponent.hpp"

void PopulateBoneHierarchy(const aiNode* node, int parentIndex,
                           std::unordered_map<std::string, int>& boneMap,
                           SkeletonComponent& skel);

inline SkeletonComponent LoadFbxSkeleton(const std::string& path)
{
    SkeletonComponent skel;
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->HasMeshes()) return skel;
    std::unordered_map<std::string, int> boneMap;
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            const aiBone* bone = mesh->mBones[b];
            if (boneMap.find(bone->mName.C_Str()) == boneMap.end())
            {
                int index = static_cast<int>(skel.bones.size());
                BoneInfo info;
                info.name = bone->mName.C_Str();
                info.offset = glm::transpose(glm::make_mat4(&bone->mOffsetMatrix.a1));
                skel.bones.push_back(info);
                boneMap[info.name] = index;
            }
        }
    }
    skel.poseMatrices.resize(skel.bones.size(), glm::mat4(1.0f));
    PopulateBoneHierarchy(scene->mRootNode, -1, boneMap, skel);
    return skel;
}
inline void PopulateBoneHierarchy(const aiNode* node, int parentIndex,
                                   std::unordered_map<std::string, int>& boneMap,
                                   SkeletonComponent& skel)
{
    auto it = boneMap.find(node->mName.C_Str());
    int currentIndex = parentIndex;
    if (it != boneMap.end())
    {
        currentIndex = it->second;
        if (currentIndex < (int)skel.bones.size())
            skel.bones[currentIndex].parentIndex = parentIndex;
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        PopulateBoneHierarchy(node->mChildren[i], currentIndex, boneMap, skel);
    }
}

inline std::vector<AnimationClip> LoadFbxAnimations(const std::string& path,
                                                    const SkeletonComponent& skel)
{
    std::vector<AnimationClip> clips;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->HasAnimations()) return clips;

    std::unordered_map<std::string, int> boneMap;
    for (size_t i = 0; i < skel.bones.size(); ++i) boneMap[skel.bones[i].name] = i;

    for (unsigned int a = 0; a < scene->mNumAnimations; ++a)
    {
        const aiAnimation* anim = scene->mAnimations[a];
        AnimationClip clip;
        clip.name = anim->mName.C_Str();
        if (clip.name.empty()) clip.name = "Anim_" + std::to_string(a);
        clip.duration = static_cast<float>(anim->mDuration / (anim->mTicksPerSecond ? anim->mTicksPerSecond : 25.0));
        clip.channels.resize(skel.bones.size());

        for (unsigned int c = 0; c < anim->mNumChannels; ++c)
        {
            const aiNodeAnim* channel = anim->mChannels[c];
            auto it = boneMap.find(channel->mNodeName.C_Str());
            if (it == boneMap.end()) continue;
            int boneIndex = it->second;
            BoneChannel& bc = clip.channels[boneIndex];
            for (unsigned int i = 0; i < channel->mNumPositionKeys; ++i)
            {
                const auto& k = channel->mPositionKeys[i];
                bc.positions.push_back({static_cast<float>(k.mTime / (anim->mTicksPerSecond ? anim->mTicksPerSecond : 25.0)),
                                        glm::vec3(k.mValue.x, k.mValue.y, k.mValue.z)});
            }
            for (unsigned int i = 0; i < channel->mNumRotationKeys; ++i)
            {
                const auto& k = channel->mRotationKeys[i];
                bc.rotations.push_back({static_cast<float>(k.mTime / (anim->mTicksPerSecond ? anim->mTicksPerSecond : 25.0)),
                                        glm::quat(k.mValue.w, k.mValue.x, k.mValue.y, k.mValue.z)});
            }
            for (unsigned int i = 0; i < channel->mNumScalingKeys; ++i)
            {
                const auto& k = channel->mScalingKeys[i];
                bc.scalings.push_back({static_cast<float>(k.mTime / (anim->mTicksPerSecond ? anim->mTicksPerSecond : 25.0)),
                                       glm::vec3(k.mValue.x, k.mValue.y, k.mValue.z)});
            }
        }
        clips.push_back(std::move(clip));
    }
    return clips;
}
