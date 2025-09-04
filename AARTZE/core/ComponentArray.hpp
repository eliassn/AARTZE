// File: AARTZE/core/ComponentArray.hpp
#pragma once

#include <array>
#include <cassert>
#include <queue>
#include <unordered_map>
#include <mutex>

#include "Entity.hpp"
#include "MemoryManager.hpp"  // Include the memory system

/**
 * @brief Interface for component arrays (used internally).
 */
class IComponentArray
{
   public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};

/**
 * @brief Stores components of a specific type T.
 */
template <typename T>
class ComponentArray : public IComponentArray
{
   public:
    ComponentArray()
    {
        arrayData =
            static_cast<T*>(MemoryManager::Arena().Allocate(sizeof(T) * MAX_ENTITIES, alignof(T)));
        assert(arrayData && "Failed to allocate memory for ComponentArray");
    }

    void InsertData(Entity entity, T component)
    {
        std::lock_guard<std::mutex> lock(arrayMutex);
        assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
               "Component added to same entity more than once.");

        size_t newIndex = size;
        entityToIndexMap[entity] = newIndex;
        indexToEntityMap[newIndex] = entity;

        new (&arrayData[newIndex]) T(component);  // Placement new
        ++size;
    }

    void RemoveData(Entity entity)
    {
        std::lock_guard<std::mutex> lock(arrayMutex);
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() &&
               "Removing non-existent component.");

        size_t indexOfRemoved = entityToIndexMap[entity];
        size_t indexOfLast = size - 1;

        if (indexOfRemoved != indexOfLast)
        {
            arrayData[indexOfRemoved] = arrayData[indexOfLast];

            Entity entityOfLast = indexToEntityMap[indexOfLast];
            entityToIndexMap[entityOfLast] = indexOfRemoved;
            indexToEntityMap[indexOfRemoved] = entityOfLast;
        }

        entityToIndexMap.erase(entity);
        indexToEntityMap.erase(indexOfLast);
        --size;
    }

    T& GetData(Entity entity)
    {
        std::lock_guard<std::mutex> lock(arrayMutex);
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() &&
               "Retrieving non-existent component.");
        return arrayData[entityToIndexMap[entity]];
    }

    void EntityDestroyed(Entity entity) override
    {
        if (entityToIndexMap.find(entity) != entityToIndexMap.end())
        {
            RemoveData(entity);
        }
    }

   private:
    T* arrayData;
    std::unordered_map<Entity, size_t> entityToIndexMap;
    std::unordered_map<size_t, Entity> indexToEntityMap;
    size_t size = 0;
    std::mutex arrayMutex;
};
