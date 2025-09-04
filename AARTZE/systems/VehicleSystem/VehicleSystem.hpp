#pragma once
#include "System.hpp"

class VehicleSystem : public System
{
   public:
    void Update(float deltaTime) override;
    const char* GetName() const override
    {
        return "VehicleSystem";
    }
};
