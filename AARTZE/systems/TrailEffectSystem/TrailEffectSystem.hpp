#pragma once
#include "System.hpp"

class TrailEffectSystem : public System
{
   public:
    void Update(float deltaTime) override;
    const char* GetName() const override
    {
        return "TrailEffectSystem";
    }
};
