#include "DecalSystem.hpp"

#include "Coordinator.hpp"
#include "DecalComponent.hpp"

void DecalSystem::Update(float deltaTime)
{
    auto entities = gCoordinator.GetEntitiesWithComponents<DecalComponent>();
    for (auto entity : entities)
    {
        auto& decal = gCoordinator.GetComponent<DecalComponent>(entity);
        if (decal.fadeOverTime)
        {
            decal.lifetime -= deltaTime;
            if (decal.lifetime <= 0.0f)
            {
                gCoordinator.RemoveComponent<DecalComponent>(entity);
            }
        }
    }
}
