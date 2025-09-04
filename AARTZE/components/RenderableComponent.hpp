#pragma once
#include <cstdint>
#include <iostream>

/**
 * @brief Data to render an entity (mesh + material + LOD support).
 */
struct RenderableComponent
{
    uint32_t meshId = 0;      // Mesh identifier (linked to GPU buffer)
    uint32_t materialId = 0;  // Material/shader ID
    int lodLevel = 0;         // Level of detail (0 = highest)
    bool castShadows = true;
    bool isVisible = true;
};
