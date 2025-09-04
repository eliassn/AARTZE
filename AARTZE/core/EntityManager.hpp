#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <queue>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include "Entity.hpp"

// Support a larger number of components than the original 128.
// Use size_t so the constant can exceed the range of ComponentType.
constexpr std::size_t MAX_COMPONENTS = 256;

/**
 * @brief Signature defines the set of components attached to an entity.
 */
using Signature = std::bitset<MAX_COMPONENTS>;

/**
 * @brief Base class for all component arrays.
 */
class IComponentArray
{
   public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

/**
 * @brief Templated storage for a single component type.
 */
template <typename T>
class ComponentArray : public IComponentArray
{
   public:
    void insert(Entity entity, T component)
    {
        assert(entityToIndex.find(entity) == entityToIndex.end() && "Component added twice!");
        std::size_t index = size++;
        entityToIndex[entity] = index;
        indexToEntity[index] = entity;
        components[index] = component;
    }

    void remove(Entity entity)
    {
        assert(entityToIndex.find(entity) != entityToIndex.end() &&
               "Removing non-existent component!");

        std::size_t indexToRemove = entityToIndex[entity];
        std::size_t lastIndex = size - 1;

        components[indexToRemove] = components[lastIndex];

        Entity lastEntity = indexToEntity[lastIndex];
        entityToIndex[lastEntity] = indexToRemove;
        indexToEntity[indexToRemove] = lastEntity;

        entityToIndex.erase(entity);
        indexToEntity.erase(lastIndex);

        size--;
    }

    T& get(Entity entity)
    {
        assert(entityToIndex.find(entity) != entityToIndex.end() && "Accessing missing component!");
        return components[entityToIndex[entity]];
    }

    bool has(Entity entity)
    {
        return entityToIndex.find(entity) != entityToIndex.end();
    }

    void entityDestroyed(Entity entity) override
    {
        if (entityToIndex.find(entity) != entityToIndex.end())
        {
            remove(entity);
        }
    }

   private:
    std::array<T, MAX_ENTITIES> components{};
    std::unordered_map<Entity, std::size_t> entityToIndex{};
    std::unordered_map<std::size_t, Entity> indexToEntity{};
    std::size_t size = 0;
};

/**
 * @brief EntityManager handles entity IDs and all component registrations.
 */
class EntityManager
{
   public:
    EntityManager()
    {
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
        {
            availableEntities.push(entity);
        }
    }

    Entity createEntity()
    {
        assert(livingEntityCount < MAX_ENTITIES && "Too many entities in existence.");
        Entity id = availableEntities.front();
        availableEntities.pop();
        livingEntityCount++;
        return id;
    }

    void destroyEntity(Entity entity)
    {
        assert(entity < MAX_ENTITIES && "Invalid entity.");
        signatures[entity].reset();

        for (auto& [_, compArray] : componentArrays)
        {
            compArray->entityDestroyed(entity);
        }

        availableEntities.push(entity);
        livingEntityCount--;
    }

    template <typename T>
    void registerComponent()
    {
        ComponentKey typeIndex = std::type_index(typeid(T));

        assert(componentTypes.find(typeIndex) == componentTypes.end() &&
               "Component already registered.");

        componentTypes[typeIndex] = nextComponentType;
        componentArrays[typeIndex] = std::make_shared<ComponentArray<T>>();
        nextComponentType++;
    }

    template <typename T>
    void addComponent(Entity entity, T component)
    {
        getComponentArray<T>()->insert(entity, component);
        signatures[entity].set(getComponentType<T>(), true);
    }

    template <typename T>
    void removeComponent(Entity entity)
    {
        getComponentArray<T>()->remove(entity);
        signatures[entity].set(getComponentType<T>(), false);
    }

    template <typename T>
    T& getComponent(Entity entity)
    {
        return getComponentArray<T>()->get(entity);
    }

    template <typename T>
    bool hasComponent(Entity entity)
    {
        return getComponentArray<T>()->has(entity);
    }

    Signature getSignature(Entity entity)
    {
        return signatures[entity];
    }

   private:
    std::queue<Entity> availableEntities;
    std::array<Signature, MAX_ENTITIES> signatures;
    using ComponentKey = std::type_index;
    std::unordered_map<ComponentKey, std::shared_ptr<IComponentArray>> componentArrays;
    std::unordered_map<ComponentKey, std::uint8_t> componentTypes;

    std::uint32_t livingEntityCount = 0;
    std::uint8_t nextComponentType = 0;

    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray()
    {
        ComponentKey typeIndex = std::type_index(typeid(T));
        assert(componentTypes.find(typeIndex) != componentTypes.end() &&
               "Component not registered.");

        return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeIndex]);
    }

    template <typename T>
    std::uint8_t getComponentType()
    {
        ComponentKey typeIndex = std::type_index(typeid(T));
        assert(componentTypes.find(typeIndex) != componentTypes.end() &&
               "Component not registered.");

        return componentTypes[typeIndex];
    }
};
