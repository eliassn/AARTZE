#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

template <typename T>
struct Keyframe
{
    float time{0.f};
    T value{};
};

struct BoneChannel
{
    std::vector<Keyframe<glm::vec3>> positions;
    std::vector<Keyframe<glm::quat>> rotations;
    std::vector<Keyframe<glm::vec3>> scalings;
};

struct AnimationClip
{
    std::string name;
    float duration{0.f};
    std::vector<BoneChannel> channels;
};

struct AnimationComponent
{
    std::vector<AnimationClip> clips;
    int currentClip{-1};
    float currentTime{0.f};
    bool playing{false};
};

