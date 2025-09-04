#pragma once
#include <cstdint>

struct MaterialComponent
{
    // Metallic-roughness PBR factors
    float baseColor[3] { 0.8f, 0.2f, 0.2f };
    float metallic { 0.0f };
    float roughness { 0.8f };
    uint32_t albedoTex { 0 }; // optional texture id
};

