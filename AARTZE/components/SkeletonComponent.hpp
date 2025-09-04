#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct BoneInfo
{
    std::string name;
    int parentIndex{-1};
    glm::mat4 offset{1.0f};
};

struct SkeletonComponent
{
    std::vector<BoneInfo> bones;
    std::vector<glm::mat4> poseMatrices;
};

