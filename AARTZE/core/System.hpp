#pragma once

/**
 * @brief Base class for all systems in the ECS architecture.
 * Systems operate on entities with specific component sets.
 */
class System
{
   public:
    virtual ~System() = default;

    // Update logic per system — must be overridden
    virtual bool Initialize()
    {
        return true;
    }
    virtual void Shutdown() {}
    virtual void Update(float deltaTime) = 0;
    virtual const char* GetName() const
    {
        return "UnnamedSystem";
    }
};
