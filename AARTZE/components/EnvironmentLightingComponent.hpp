#pragma once
#include <cstdint>
#include <iostream>

/**
 * @brief Component for global environment lighting settings like skybox, ambient light, etc.
 */
struct EnvironmentLightingComponent
{
    uint32_t skyboxTextureId = 0;   // ID of the cubemap skybox
    float ambientIntensity = 1.0f;  // Ambient light multiplier
    bool enableFog = true;
    float fogDensity = 0.02f;
    float fogStart = 10.0f;
    float fogEnd = 100.0f;
};
