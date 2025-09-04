#pragma once
#include <cstdint>
#include <string>

/**
 * @brief Tracks the current animation state for smooth blending.
 */
struct AnimationStateComponent
{
    uint32_t currentAnimationId = 0;
    float transitionTime = 0.2f;
    bool isTransitioning = false;
};
