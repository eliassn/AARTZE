#pragma once
#include <array>

/**
 * @brief World-space position of an entity in 3D.
 */
struct PositionComponent
{
    std::array<float, 3> position = {0.0f, 0.0f, 0.0f};
};
