#pragma once
#include <array>
#include <cstdint>

/**
 * @brief AI behavior tree placeholder and state info.
 */
struct AIComponent
{
    enum class Behavior
    {
        Idle,
        Driving,
        Walking,
        Jogging,
        Patrol,
        Chase,
        Dialogue,
        Flee,
        Attack
    };

    Behavior currentBehavior = Behavior::Idle;
    std::array<float, 3> targetPosition = {0.0f, 0.0f, 0.0f};
    uint32_t targetEntity = 0;
    bool engaged = false;
};
