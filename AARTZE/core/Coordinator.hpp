#pragma once

#include <bitset>
#include <cassert>
#include <memory>
#include <mutex>
#include <queue>
#include <cstddef>
#include <typeindex>
#include <unordered_map>

#include "ComponentArray.hpp"
#include "Entity.hpp"
#include "MemoryManager.hpp"
#include "ThreadPool.hpp"

using Signature = std::bitset<MAX_COMPONENTS>;

class Coordinator
{
   public:
    Coordinator()
    {
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
            availableEntities.push(entity);

        // Initialize memory manager
        MemoryManager::Initialize();
    }

    ~Coordinator()
    {
        // Ensure components are destroyed before releasing arena memory
        componentArrays.clear();
        entitySignatures.clear();
        // Shutdown memory manager
        MemoryManager::Shutdown();
    }

    Entity CreateEntity()
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        assert(livingEntityCount < MAX_ENTITIES && "Too many entities in existence.");
        Entity id = availableEntities.front();
        availableEntities.pop();
        ++livingEntityCount;
        return id;
    }

    void DestroyEntity(Entity entity)
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        assert(entity < MAX_ENTITIES && "Entity out of range.");
        entitySignatures.erase(entity);
        for (auto const& pair : componentArrays)
            pair.second->EntityDestroyed(entity);
        availableEntities.push(entity);
        --livingEntityCount;
    }

    template <typename T>
    void RegisterComponent()
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        ComponentKey typeIndex = std::type_index(typeid(T));
        assert(componentTypes.find(typeIndex) == componentTypes.end() &&
               "Component already registered.");

        void* mem = MemoryManager::Arena().Allocate(sizeof(ComponentArray<T>));
        auto* array = new (mem) ComponentArray<T>();
        componentTypes[typeIndex] = nextComponentType++;
        componentArrays[typeIndex] =
            std::shared_ptr<IComponentArray>(array, [](IComponentArray* ptr) {
                ptr->~IComponentArray();
            });
    }

    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        GetComponentArray<T>()->InsertData(entity, component);
        entitySignatures[entity].set(GetComponentType<T>(), true);
    }

    template <typename T>
    void RemoveComponent(Entity entity)
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        GetComponentArray<T>()->RemoveData(entity);
        entitySignatures[entity].set(GetComponentType<T>(), false);
    }

    template <typename T>
    T& GetComponent(Entity entity)
    {
        return GetComponentArray<T>()->GetData(entity);
    }

    template <typename T>
    ComponentType GetComponentType()
    {
        ComponentKey typeIndex = std::type_index(typeid(T));
        assert(componentTypes.find(typeIndex) != componentTypes.end() &&
               "Component not registered.");
        return componentTypes[typeIndex];
    }

    Signature GetSignature(Entity entity)
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        return entitySignatures[entity];
    }

    template <typename... Components>
    std::vector<Entity> GetEntitiesWithComponents()
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        std::vector<Entity> result;
        Signature required = GetCombinedSignature<Components...>();

        for (const auto& [entity, signature] : entitySignatures)
        {
            if ((signature & required) == required)
                result.push_back(entity);
        }

        return result;
    }

    bool IsEntityAlive(Entity entity) const
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        return entitySignatures.find(entity) != entitySignatures.end();
    }

    template <typename T>
    bool HasComponent(Entity entity) const
    {
        std::lock_guard<std::mutex> lock(ecsMutex);
        ComponentKey typeIndex = std::type_index(typeid(T));
        auto it = componentTypes.find(typeIndex);
        if (it == componentTypes.end()) return false;

        ComponentType type = it->second;
        auto sigIt = entitySignatures.find(entity);
        if (sigIt == entitySignatures.end()) return false;

        return sigIt->second.test(type);
    }

   private:
    std::queue<Entity> availableEntities;
    using ComponentKey = std::type_index;
    std::unordered_map<ComponentKey, ComponentType> componentTypes;
    std::unordered_map<ComponentKey, std::shared_ptr<IComponentArray>> componentArrays;
    std::unordered_map<Entity, Signature> entitySignatures;

    ComponentType nextComponentType = 0;
    uint32_t livingEntityCount = 0;
    mutable std::mutex ecsMutex;

    template <typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray()
    {
        ComponentKey typeIndex = std::type_index(typeid(T));
        assert(componentArrays.find(typeIndex) != componentArrays.end() &&
               "Component not registered.");
        return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeIndex]);
    }

    template <typename... Components>
    Signature GetCombinedSignature()
    {
        Signature sig;
        (sig.set(GetComponentType<Components>()), ...);
        return sig;
    }
};

// === Global Instances ===
inline Coordinator gCoordinator;
inline ThreadPool gThreadPool(std::max<unsigned>(1, std::thread::hardware_concurrency()));
