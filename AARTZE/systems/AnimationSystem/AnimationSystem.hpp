#pragma once
#include "core/System.hpp"

class AnimationSystem : public System
{
public:
    void Update(float deltaTime) override;
    const char* GetName() const override { return "AnimationSystem"; }
};

