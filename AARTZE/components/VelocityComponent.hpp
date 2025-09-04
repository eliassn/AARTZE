#pragma once
#include <array>

/**
 * @brief Represents both linear and angular velocity for dynamic motion.
 */
struct VelocityComponent
{
    std::array<float, 3> linear = {0.0f, 0.0f, 0.0f};   // X, Y, Z velocity in m/s
    std::array<float, 3> angular = {0.0f, 0.0f, 0.0f};  // Pitch, Yaw, Roll in rad/s
};
