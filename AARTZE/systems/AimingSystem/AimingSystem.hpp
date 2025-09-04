#pragma once
#include "System.hpp"

class AimingSystem : public System
{
   public:
    void Update(float deltaTime) override;
    const char* GetName() const override
    {
        return "AimingSystem";
    }
};
