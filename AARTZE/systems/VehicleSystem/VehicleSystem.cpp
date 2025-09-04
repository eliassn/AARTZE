#include "VehicleSystem.hpp"

#include <algorithm>

#include "Coordinator.hpp"
#include "DrivingComponent.hpp"
#include "environment/TireSplashComponent.hpp"

void VehicleSystem::Update(float deltaTime)
{
    auto drivingEntities = gCoordinator.GetEntitiesWithComponents<DrivingComponent>();
    for (auto entity : drivingEntities)
    {
        auto& drive = gCoordinator.GetComponent<DrivingComponent>(entity);
        if (drive.isDriving)
            drive.acceleration = std::min(1.0f, drive.acceleration + deltaTime);
        else
            drive.acceleration = 0.0f;
    }

    auto splashEntities =
        gCoordinator.GetEntitiesWithComponents<DrivingComponent, TireSplashComponent>();
    for (auto entity : splashEntities)
    {
        auto& drive = gCoordinator.GetComponent<DrivingComponent>(entity);
        auto& splash = gCoordinator.GetComponent<TireSplashComponent>(entity);
        if (drive.isDriving)
        {
            splash.splashIntensity =
                std::min(1.0f, splash.splashIntensity + drive.acceleration * deltaTime);
            splash.lastSplashTime += deltaTime;
        }
        else
        {
            splash.splashIntensity = 0.0f;
            splash.lastSplashTime = 0.0f;
        }
    }
}
