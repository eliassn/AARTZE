#pragma once
#include <cstdint>

enum class RigidBodyType { Static, Dynamic, Kinematic };

struct RigidBodyComponent
{
    RigidBodyType type{RigidBodyType::Dynamic};
    float mass{1.0f};
    float friction{0.5f};
    float restitution{0.0f};
    // Runtime handle (btRigidBody*) stored as uintptr_t to avoid header exposure
    std::uintptr_t native{0};
};

