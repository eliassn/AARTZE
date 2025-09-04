#include "TrailEffectSystem.hpp"

#include "Coordinator.hpp"
#include "TrailEffectComponent.hpp"

void TrailEffectSystem::Update(float deltaTime)
{
    auto entities = gCoordinator.GetEntitiesWithComponents<TrailEffectComponent>();
    for (auto entity : entities)
    {
        auto& trail = gCoordinator.GetComponent<TrailEffectComponent>(entity);
        trail.lifetime -= deltaTime;
        if (trail.lifetime <= 0.0f) gCoordinator.RemoveComponent<TrailEffectComponent>(entity);
    }
}
