#include "AimingSystem.hpp"

#include "AimingComponent.hpp"
#include "Coordinator.hpp"

void AimingSystem::Update(float deltaTime)
{
    auto entities = gCoordinator.GetEntitiesWithComponents<AimingComponent>();
    for (auto entity : entities)
    {
        auto& aim = gCoordinator.GetComponent<AimingComponent>(entity);
        if (aim.isAiming)
        {
            aim.aimZoomLevel += (1.5f - aim.aimZoomLevel) * 0.1f;
            aim.aimSensitivity = 0.5f;
        }
        else
        {
            aim.aimZoomLevel += (1.0f - aim.aimZoomLevel) * 0.1f;
            aim.aimSensitivity = 0.75f;
        }
    }
}
