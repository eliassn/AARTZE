#pragma once
#include <array>
#include <string>
#include <vector>

#include "LearningDB.hpp"
#include "World/ZoneRegistry.hpp"
#include "World/MeshRegistry.hpp"
#include "World/TextureRegistry.hpp"

namespace AARTZE {

struct TerrainPiece {
    std::string meshPath;
    uint32_t meshId{0};
    uint32_t textureId{0};
};

struct Entity {
    std::string name;
    std::string meshPath;
    uint32_t meshId{0};
    std::array<float,3> position{0.f,0.f,0.f};
};

struct GeneratedWorld {
    std::vector<TerrainPiece> terrain;
    std::vector<Entity> entities;
};

GeneratedWorld GenerateWorld(const std::string& prompt);

} // namespace AARTZE
