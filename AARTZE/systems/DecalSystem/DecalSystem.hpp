#pragma once
#include "System.hpp"

class DecalSystem : public System
{
   public:
    void Update(float deltaTime) override;
    const char* GetName() const override
    {
        return "DecalSystem";
    }
};
