#pragma once
#include <cstdint>
#include <iostream>

/**
 * @brief Represents a skeletal mesh with animation support.
 */
struct SkeletalMeshComponent
{
    uint32_t meshId = 0;
    uint32_t skeletonId = 0;
    uint32_t animationStateId = 0;
    bool isPlaying = true;
    bool loop = true;
    float playbackSpeed = 1.0f;
};
