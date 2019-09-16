#pragma once

#include <cassert>
#include <memory>
#include "ComponentContainer.h"
#include "hash.h"

namespace ecs
{

template<typename T>
class Component;

class EntityManager
{
public:
    EntityManager()
    {
        auto nbComponents = BaseComponent::getComponentCount();
        // Component containers
        mComponentContainers.resize(nbComponents);
        for (auto type = std::size_t(0); type < mComponentContainers.size(); ++type)
            mComponentContainers[type] = BaseComponent::createComponentContainer(type);
        // Entity sets
        mComponentToEntitySets.resize(nbComponents);
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
        return mEntities.emplace().first;
    }

    void removeEntity(Entity entity)
    {
        // Remove components
        for (auto& [componentType, componentId] : mEntities.get(entity))
            mComponentContainers[componentType]->remove(componentId);
        // Send message to entity sets
        for (auto& [entitySetId, entitySet] : mEntitySets)
            entitySet->onEntityRemoved(entity);
        // Remove entity
        mEntities.erase(entity);
    }

    // Components

    template<typename T>
    bool hasComponent(Entity entity) const
    {
        checkComponentType<T>();
        auto& componentIds = mEntities.get(entity);
        return componentIds.find(T::Type) != std::end(componentIds);
    }

    template<typename ...Ts>
    bool hasComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        auto& componentIds = mEntities.get(entity);
        return ((componentIds.find(Ts::Type) != std::end(componentIds)) && ...);
    }

    template<typename T>
    T& getComponent(Entity entity)
    {
        checkComponentType<T>();
        auto componentId = mEntities.get(entity)[T::Type];
        return getComponentSparseSet<T>().get(componentId);
    }

    template<typename T>
    const T& getComponent(Entity entity) const
    {
        checkComponentType<T>();
        auto componentId = mEntities.get(entity).find(T::Type)->second;
        return getComponentSparseSet<T>().get(componentId);
    }

    template<typename ...Ts>
    std::tuple<Ts&...> getComponents(Entity entity)
    {
        checkComponentTypes<Ts...>();
        auto& componentIds = mEntities.get(entity);
        return std::tie(getComponentSparseSet<Ts>().get(componentIds[Ts::Type])...);
    }

    template<typename ...Ts>
    std::tuple<const Ts&...> getComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        auto& componentIds = mEntities.get(entity);
        return std::tie(std::as_const(getComponentSparseSet<Ts>().get(componentIds.find(Ts::Type)->second))...);
    }

    template<typename T, typename ...Args>
    T& addComponent(Entity entity, Args&&... args)
    {
        checkComponentType<T>();
        auto [componentId, component] = getComponentSparseSet<T>().emplace(std::forward<Args>(args)...);
        mEntities.get(entity)[T::Type] = componentId;
        // Send message to entity sets
        for (auto entitySet : mComponentToEntitySets[T::Type])
            entitySet->onEntityUpdated(entity);
        // Return the created component
        return component;
    }

    template<typename T>
    void removeComponent(Entity entity)
    {
        checkComponentType<T>();
        auto& componentIds = mEntities.get(entity);
        auto it = componentIds.find(T::Type);
        getComponentSparseSet<T>().erase(it->second);
        componentIds.erase(it);
        // Send message to entity sets
        for (auto entitySet : mComponentToEntitySets[T::Type])
            entitySet->onEntityUpdated(entity);
    }

    // Entity sets

    template<typename ...Ts>
    void registerEntitySet()
    {
        checkComponentTypes<Ts...>();
        if (mEntitySets.find(EntitySetId{Ts::Type...}) == std::end(mEntitySets))
        {
            auto entitySet = std::make_unique<EntitySet<Ts...>>(&mEntities,
                std::tie(getComponentSparseSet<Ts>()...));
            (mComponentToEntitySets[Ts::Type].push_back(entitySet.get()), ...);
            mEntitySets[EntitySetId{Ts::Type...}] = std::move(entitySet);
        }
    }

    template<typename ...Ts>
    EntitySet<Ts...>& getEntitySet()
    {
        checkComponentTypes<Ts...>();
        assert(mEntitySets.find(EntitySetId{Ts::Type...}) != std::end(mEntitySets) &&
            "An entity set must be registered before using it");
        return *static_cast<EntitySet<Ts...>*>(mEntitySets[EntitySetId{Ts::Type...}].get());
    }

    template<typename ...Ts>
    const EntitySet<Ts...>& getEntitySet() const
    {
        checkComponentTypes<Ts...>();
        assert(mEntitySets.find(EntitySetId{Ts::Type...}) != std::end(mEntitySets) &&
            "An entity set must be registered before using it");
        return *static_cast<EntitySet<Ts...>*>(mEntitySets.find(EntitySetId{Ts::Type...})->second.get());
    }

private:
    std::vector<std::unique_ptr<BaseComponentContainer>> mComponentContainers;
    EntityContainer mEntities;
    std::unordered_map<EntitySetId, std::unique_ptr<BaseEntitySet>, HashEntitySetId> mEntitySets;
    std::vector<std::vector<BaseEntitySet*>> mComponentToEntitySets;

    template<typename T>
    constexpr void checkComponentType() const
    {
        static_assert(std::is_base_of_v<Component<T>, T>, "A component of type T must inherit Component<T>");
    }

    template<typename ...Ts>
    constexpr void checkComponentTypes() const
    {
        (checkComponentType<Ts>(), ...);
    }

    template<typename T>
    ComponentSparseSet<T>& getComponentSparseSet()
    {
        return static_cast<ComponentContainer<T>*>(mComponentContainers[T::Type].get())->components;
    }

    template<typename T>
    const ComponentSparseSet<T>& getComponentSparseSet() const
    {
        return static_cast<const ComponentContainer<T>*>(mComponentContainers[T::Type].get())->components;
    }
};

}