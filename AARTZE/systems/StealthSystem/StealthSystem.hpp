#pragma once
#include "System.hpp"

class StealthSystem : public System
{
   public:
    void Update(float deltaTime) override;
    const char* GetName() const override
    {
        return "StealthSystem";
    }
};
