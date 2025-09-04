#pragma once
#include <cstdint>

// Simple renderable descriptor for a mesh + material
struct RenderableComponent
{
    uint32_t meshId = 0;
    uint32_t materialId = 0; // reserved for future registry
    int lodLevel = 0;
    bool castShadows = true;
    bool isVisible = true;
};

