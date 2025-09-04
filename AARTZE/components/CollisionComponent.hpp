#pragma once
#include <array>

/**
 * @brief AABB box for collision detection.
 */
struct CollisionComponent
{
    std::array<float, 3> boundingBoxMin;
    std::array<float, 3> boundingBoxMax;
    bool isCollidable = true;
};
