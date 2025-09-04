// File: AARTZE/core/SystemManager.hpp
#pragma once

#include <future>
#include <memory>
#include <vector>

#include "core/Coordinator.hpp"

// Keep SystemManager minimal and consistent with available systems
#include "../../AARTZE/systems/AimingSystem/AimingSystem.hpp"
#include "../../AARTZE/systems/DecalSystem/DecalSystem.hpp"
#include "../../AARTZE/systems/StealthSystem/StealthSystem.hpp"
#include "../../AARTZE/systems/TrailEffectSystem/TrailEffectSystem.hpp"
#include "../../AARTZE/systems/VehicleSystem/VehicleSystem.hpp"
#include "../../AARTZE/systems/WeatherSystem/WeatherSystem.hpp"
#include "../../AARTZE/systems/TextRenderingSystem/TextRenderingSystem.hpp"
#include "../../AARTZE/systems/PhysicsSystem/PhysicsSystem.hpp"
#include "../../AARTZE/systems/NavigationSystem/NavigationSystem.hpp"
#include "../../AARTZE/systems/AnimationSystem/AnimationSystem.hpp"
#include "../../AARTZE/systems/StreamingSystem/StreamingSystem.hpp"
#include "core/Profiler.hpp"

/*
 * Thread Safety Design Notes
 * --------------------------
 * The ECS coordinator (`gCoordinator`) protects entity creation and component
 * registration with a mutex, but `GetComponent` accesses are unsynchronized.
 * Running system updates in parallel previously caused data races when multiple
 * systems modified the same components. See docs/ThreadSafetyAudit.md for the
 * components each system touches.
 *
 * To avoid corruption we now execute systems serially inside `Update()`.
 * Systems can be grouped into parallel batches in the future once their
 * interactions are verified to be read‑only or otherwise safe.
 */

class SystemManager
{
   public:
    // Minimal set of available systems
    StealthSystem stealthSystem;
    AimingSystem aimingSystem;
    DecalSystem decalSystem;
    TrailEffectSystem trailEffectSystem;
    VehicleSystem vehicleSystem;
    WeatherSystem weatherSystem;
    TextRenderingSystem textRenderingSystem; // rendered separately via Application
    PhysicsSystem physicsSystem;
    NavigationSystem navigationSystem;
    AnimationSystem animationSystem;
    StreamingSystem streamingSystem;

    void Update(float deltaTime)
    {
        // Serial updates to avoid data races; profile each
        { ProfileScope _p(gProfiler, "Streaming");  streamingSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "TrailEffect"); trailEffectSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "Stealth");     stealthSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "Aiming");      aimingSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "Decal");       decalSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "Vehicle");     vehicleSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "Weather");     weatherSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "Animation");   animationSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "Navigation");  navigationSystem.Update(deltaTime); }
        { ProfileScope _p(gProfiler, "Physics");     physicsSystem.Update(deltaTime); }
        // textRenderingSystem: drawn in Application after render pass
    }
};

extern std::unique_ptr<SystemManager> gSystemManager;
