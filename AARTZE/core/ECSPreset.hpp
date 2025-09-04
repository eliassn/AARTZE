#pragma once
#include "Coordinator.hpp"

#include "components/TransformComponent.hpp"
#include "components/RenderableComponent.hpp"
#include "components/MaterialComponent.hpp"
// From legacy component set (included via include dirs)
#include "AimingComponent.hpp"
#include "DrivingComponent.hpp"
#include "SneakingComponent.hpp"
#include "components/AnimationComponent.hpp"
#include "components/animation/AnimationBlendComponent.hpp"
#include "components/physics/RigidBodyComponent.hpp"
#include "components/physics/BoxColliderComponent.hpp"
#include "components/physics/SphereColliderComponent.hpp"
#include "components/navigation/NavAgentComponent.hpp"

inline void RegisterBasicComponents()
{
    gCoordinator.RegisterComponent<TransformComponent>();
    gCoordinator.RegisterComponent<RenderableComponent>();
    gCoordinator.RegisterComponent<MaterialComponent>();
    gCoordinator.RegisterComponent<AimingComponent>();
    gCoordinator.RegisterComponent<DrivingComponent>();
    gCoordinator.RegisterComponent<SneakingComponent>();
    // Phase 2
    gCoordinator.RegisterComponent<AnimationComponent>();
    gCoordinator.RegisterComponent<AnimationBlendComponent>();
    gCoordinator.RegisterComponent<RigidBodyComponent>();
    gCoordinator.RegisterComponent<BoxColliderComponent>();
    gCoordinator.RegisterComponent<SphereColliderComponent>();
    gCoordinator.RegisterComponent<NavAgentComponent>();
}
