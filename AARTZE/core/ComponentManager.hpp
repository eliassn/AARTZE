#pragma once

#include "Coordinator.hpp"
#include "Entity.hpp"

/**
 * @brief Simple facade over the global Coordinator to manipulate components.
 */
class ComponentManager
{
public:
    template <typename T>
    static void Register()
    {
        gCoordinator.RegisterComponent<T>();
    }

    template <typename T>
    static void AddComponent(Entity entity, const T& component)
    {
        gCoordinator.AddComponent(entity, component);
    }

    template <typename T>
    static void RemoveComponent(Entity entity)
    {
        gCoordinator.RemoveComponent<T>(entity);
    }

    template <typename T>
    static T& GetComponent(Entity entity)
    {
        return gCoordinator.GetComponent<T>(entity);
    }

    template <typename T>
    static bool HasComponent(Entity entity)
    {
        return gCoordinator.HasComponent<T>(entity);
    }
};
