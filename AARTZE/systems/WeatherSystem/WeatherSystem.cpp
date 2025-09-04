#include "WeatherSystem.hpp"

#include <algorithm>

#include "Coordinator.hpp"
#include "environment/PuddleComponent.hpp"
#include "environment/RainAudioComponent.hpp"
#include "environment/RainOcclusionComponent.hpp"
#include "environment/RainRipplesComponent.hpp"
#include "environment/ScreenWetnessEffectComponent.hpp"

void WeatherSystem::Update(float deltaTime)
{
    auto rippleEntities = gCoordinator.GetEntitiesWithComponents<RainRipplesComponent>();
    for (auto entity : rippleEntities)
    {
        auto& ripple = gCoordinator.GetComponent<RainRipplesComponent>(entity);
        bool occluded = gCoordinator.HasComponent<RainOcclusionComponent>(entity);
        ripple.active = isRaining && !occluded;
    }

    auto screenEntities = gCoordinator.GetEntitiesWithComponents<ScreenWetnessEffectComponent>();
    for (auto entity : screenEntities)
    {
        auto& screen = gCoordinator.GetComponent<ScreenWetnessEffectComponent>(entity);
        if (isRaining)
            screen.wetnessAmount = std::min(1.0f, screen.wetnessAmount + deltaTime);
        else
            screen.wetnessAmount = std::max(0.0f, screen.wetnessAmount - deltaTime);
    }

    auto puddleEntities = gCoordinator.GetEntitiesWithComponents<PuddleComponent>();
    for (auto entity : puddleEntities)
    {
        auto& puddle = gCoordinator.GetComponent<PuddleComponent>(entity);
        if (isRaining && puddle.isDynamic)
            puddle.depth = std::min(1.0f, puddle.depth + deltaTime);
        else
            puddle.depth = std::max(0.0f, puddle.depth - deltaTime);
    }

    auto audioEntities = gCoordinator.GetEntitiesWithComponents<RainAudioComponent>();
    for (auto entity : audioEntities)
    {
        auto& audio = gCoordinator.GetComponent<RainAudioComponent>(entity);
        audio.volume = isRaining ? 1.0f : 0.0f;
    }
}
