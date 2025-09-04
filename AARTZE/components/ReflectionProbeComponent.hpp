#pragma once
#include <cstdint>
#include <array>
#include <iostream>

/**
 * @brief Handles baked or real-time reflection probes for PBR rendering.
 */
struct ReflectionProbeComponent
{
    std::array<float, 3> position = {0.0f, 0.0f, 0.0f};
    float influenceRadius = 20.0f;
    uint32_t cubemapId = 0;
    bool isDynamic = false;  // Set to true for real-time reflections
};
