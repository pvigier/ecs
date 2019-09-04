#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>
#include "EntityContainer.h"
#include "EntitySet.h"
#include "ComponentContainer.h"
#include "hash.h"

namespace ecs
{

template<typename T>
class Component;

class EntityManager
{
public:
    EntityManager(std::size_t nbComponents, std::size_t nbSystems) :
        mEntities(nbComponents, nbSystems)
    {
        mComponentContainers.resize(nbComponents);
    }

    void reserve(std::size_t size)
    {
        for (auto& componentContainer : mComponentContainers)
        {
            if (componentContainer)
                componentContainer->reserve(size);
        }
        mEntities.reserve(size);
    }

    // Entities

    Entity createEntity()
    {
        return mEntities.create();
    }

    void removeEntity(Entity entity)
    {
        // Remove components
        for (auto& componentContainer : mComponentContainers)
        {
            if (componentContainer)
                componentContainer->tryRemove(entity);
        }
        // Send message to entity sets
        for (auto& [entitySetId, entitySet] : mEntitySets)
            entitySet->onEntityRemoved(entity);
        // Remove entity
        mEntities.remove(entity);
    }

    // Components

    template<typename T>
    void registerComponent()
    {
        checkComponentType<T>();
        mComponentContainers[T::type] = std::make_unique<ComponentContainer<T>>(
            mEntities.getEntityToComponent(T::type));
    }

    template<typename T>
    bool hasComponent(Entity entity) const
    {
        checkComponentType<T>();
        return mEntities.hasComponent<T>(entity);
    }

    template<typename ...Ts>
    bool hasComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        return mEntities.hasComponents<Ts...>(entity);
    }

    template<typename T>
    T& getComponent(Entity entity)
    {
        checkComponentType<T>();
        return getComponentContainer<T>()->get(entity);
    }

    template<typename T>
    const T& getComponent(Entity entity) const
    {
        checkComponentType<T>();
        return getComponentContainer<T>()->get(entity);
    }

    template<typename ...Ts>
    std::tuple<Ts&...> getComponents(Entity entity)
    {
        checkComponentTypes<Ts...>();
        return std::tie(getComponentContainer<Ts>()->get(entity)...);
    }

    template<typename ...Ts>
    std::tuple<const Ts&...> getComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        return std::tie(std::as_const(getComponentContainer<Ts>()->get(entity))...);
    }

    template<typename T, typename ...Args>
    void addComponent(Entity entity, Args&&... args)
    {
        checkComponentType<T>();
        getComponentContainer<T>()->add(entity, std::forward<Args>(args)...);
        // Send message to entity sets
        for (auto& [entitySetId, entitySet] : mEntitySets)
            entitySet->onEntityUpdated(entity);
    }

    template<typename T>
    void removeComponent(Entity entity)
    {
        checkComponentType<T>();
        getComponentContainer<T>()->remove(entity);
        // Send message to entity sets
        for (auto& [entitySetId, entitySet] : mEntitySets)
            entitySet->onEntityUpdated(entity);
    }

    template<typename T>
    Entity getOwner(const T& component) const
    {
        checkComponentType<T>();
        return getComponentContainer<T>()->getOwner(component);
    }

    // Entity sets

    template<typename ...Ts>
    void registerEntitySet()
    {
        checkComponentTypes<Ts...>();
        // TODO: add assertion concerning the number of entity sets
        mEntitySets[EntitySetId{Ts::type...}] = std::make_unique<EntitySet<Ts...>>(
            &mEntities, &mEntities.getEntityToManagedEntity(mEntitySets.size()));
    }

    template<typename ...Ts>
    const std::vector<Entity>& getEntitySet()
    {
        checkComponentTypes<Ts...>();
        assert(mEntitySets.find(EntitySetId{Ts::type...}) != std::end(mEntitySets));
        return mEntitySets[EntitySetId{Ts::type...}]->getEntities();
    }

private:
    std::vector<std::unique_ptr<BaseComponentContainer>> mComponentContainers;
    EntityContainer mEntities;
    std::unordered_map<EntitySetId, std::unique_ptr<BaseEntitySet>, HashEntitySetId> mEntitySets;

    template<typename T>
    constexpr void checkComponentType() const
    {
        static_assert(std::is_base_of_v<Component<T>, T>);
    }

    template<typename ...Ts>
    constexpr void checkComponentTypes() const
    {
        (checkComponentType<Ts>(), ...);
    }

    template<typename T>
    auto getComponentContainer()
    {
        return static_cast<ComponentContainer<T>*>(mComponentContainers[T::type].get());
    }

    template<typename T>
    auto getComponentContainer() const
    {
        return static_cast<const ComponentContainer<T>*>(mComponentContainers[T::type].get());
    }
};

}