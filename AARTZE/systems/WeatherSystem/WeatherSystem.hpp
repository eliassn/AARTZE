#pragma once
#include "System.hpp"

class WeatherSystem : public System
{
   public:
    bool isRaining = true;
    void Update(float deltaTime) override;
    const char* GetName() const override
    {
        return "WeatherSystem";
    }
};
