#pragma once
#include "core/System.hpp"
#include "navigation/GridNav.hpp"

class NavigationSystem : public System
{
public:
    bool Initialize();
    void Shutdown() override {}
    void Update(float deltaTime) override;
    const char* GetName() const override { return "NavigationSystem"; }

private:
    GridNav m_grid;
};

