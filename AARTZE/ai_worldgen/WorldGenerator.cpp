#include "WorldGenerator.hpp"

namespace AARTZE {

GeneratedWorld GenerateWorld(const std::string& prompt) {
    GeneratedWorld world;
    auto learned = LoadLearningData();

    // Basic terrain piece using registry templates
    TerrainPiece ground;
    ground.meshPath = "assets/terrain/ground.obj";
    ground.meshId = RegisterMesh(ground.meshPath);
    ground.textureId = RegisterTexture("assets/terrain/ground.png");
    world.terrain.push_back(ground);

    // Create entities at known zones
    for (const auto& [zone, pos] : ZonePositions) {
        Entity entity;
        entity.name = zone;
        entity.meshPath = "assets/entities/default.obj";
        entity.meshId = RegisterMesh(entity.meshPath);
        entity.position = pos;
        world.entities.push_back(entity);
    }

    // Record the prompt as a custom part to refine future generations
    SaveCustomPart({prompt, "generated"});
    return world;
}

} // namespace AARTZE
