#include "StealthSystem.hpp"

#include <algorithm>

#include "Coordinator.hpp"
#include "SneakingComponent.hpp"

void StealthSystem::Update(float deltaTime)
{
    auto entities = gCoordinator.GetEntitiesWithComponents<SneakingComponent>();
    for (auto entity : entities)
    {
        auto& sneak = gCoordinator.GetComponent<SneakingComponent>(entity);
        if (sneak.isSneaking)
            sneak.noiseLevel = std::max(0.0f, sneak.noiseLevel - deltaTime);
        else
            sneak.noiseLevel = std::min(1.0f, sneak.noiseLevel + deltaTime);
    }
}
