#pragma once
#include <array>

/**
 * @brief Basic 3D position, rotation, and scale component.
 */
struct TransformComponent
{
    std::array<float, 3> position{0.0f, 0.0f, 0.0f};
    std::array<float, 3> rotation{0.0f, 0.0f, 0.0f};  // Pitch, Yaw, Roll
    std::array<float, 3> scale{1.0f, 1.0f, 1.0f};
};
