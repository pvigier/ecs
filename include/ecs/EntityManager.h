#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>
#include "ComponentContainer.h"
#include "EntityContainer.h"
#include "EntitySet.h"
#include "hash.h"

namespace ecs
{

template<typename T>
class Component;

class EntityManager
{
public:
    EntityManager(std::size_t nbComponents)
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
        return mEntities.emplace();
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
    void registerComponent()
    {
        checkComponentType<T>();
        mComponentContainers[T::type] = std::make_unique<ComponentContainer<T>>();
    }

    template<typename T>
    bool hasComponent(Entity entity) const
    {
        checkComponentType<T>();
        auto& componentIds = mEntities.get(entity);
        return componentIds.find(T::type) != std::end(componentIds);
    }

    template<typename ...Ts>
    bool hasComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        auto& componentIds = mEntities.get(entity);
        return ((componentIds.find(Ts::type) != std::end(componentIds)) && ...);
    }

    template<typename T>
    T& getComponent(Entity entity)
    {
        checkComponentType<T>();
        auto componentId = mEntities.get(entity)[T::type];
        return getComponentContainer<T>().get(componentId);
    }

    template<typename T>
    const T& getComponent(Entity entity) const
    {
        checkComponentType<T>();
        auto componentId = mEntities.get(entity).find(T::type)->second;
        return getComponentContainer<T>().get(componentId);
    }

    template<typename ...Ts>
    std::tuple<Ts&...> getComponents(Entity entity)
    {
        checkComponentTypes<Ts...>();
        auto& componentIds = mEntities.get(entity);
        return std::tie(getComponentContainer<Ts>().get(componentIds[Ts::type])...);
    }

    template<typename ...Ts>
    std::tuple<const Ts&...> getComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        auto& componentIds = mEntities.get(entity);
        return std::tie(std::as_const(getComponentContainer<Ts>().get(componentIds.find(Ts::type)->second))...);
    }

    template<typename T>
    T& getComponentById(ComponentId componentId)
    {
        checkComponentType<T>();
        return getComponentContainer<T>().get(componentId);
    }

    template<typename T>
    const T& getComponentById(ComponentId componentId) const
    {
        checkComponentType<T>();
        return getComponentContainer<T>().get(componentId);
    }

    template<typename ...Ts>
    std::tuple<Ts&...> getComponentsByIds(const std::array<ComponentId, sizeof...(Ts)>& componentIds)
    {
        checkComponentTypes<Ts...>();
        return getComponentsByIds<Ts...>(componentIds, std::index_sequence_for<Ts...>{});
    }

    template<typename ...Ts>
    std::tuple<const Ts&...> getComponentsByIds(const std::array<ComponentId, sizeof...(Ts)>& componentIds) const
    {
        checkComponentTypes<Ts...>();
        return getComponentsByIds<Ts...>(componentIds, std::index_sequence_for<Ts...>{});
    }

    template<typename T, typename ...Args>
    void addComponent(Entity entity, Args&&... args)
    {
        checkComponentType<T>();
        auto componentId = getComponentContainer<T>().emplace(std::forward<Args>(args)...);
        mEntities.get(entity)[T::type] = componentId;
        // Send message to entity sets
        for (auto& [entitySetId, entitySet] : mEntitySets)
            entitySet->onEntityUpdated(entity);
    }

    template<typename T>
    void removeComponent(Entity entity)
    {
        checkComponentType<T>();
        auto& componentIds = mEntities.get(entity);
        auto it = componentIds.find(T::type);
        getComponentContainer<T>().erase(it->second);
        componentIds.erase(it);
        // Send message to entity sets
        for (auto& [entitySetId, entitySet] : mEntitySets)
            entitySet->onEntityUpdated(entity);
    }

    // Entity sets

    template<typename ...Ts>
    void registerEntitySet()
    {
        checkComponentTypes<Ts...>();
        mEntitySets[EntitySetId{Ts::type...}] = std::make_unique<EntitySet<Ts...>>(&mEntities);
    }

    template<typename ...Ts>
    const auto& getEntitySet()
    {
        checkComponentTypes<Ts...>();
        assert(mEntitySets.find(EntitySetId{Ts::type...}) != std::end(mEntitySets));
        return static_cast<EntitySet<Ts...>*>(mEntitySets[EntitySetId{Ts::type...}].get())->getEntities();
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
    auto& getComponentContainer()
    {
        return static_cast<ComponentContainer<T>*>(mComponentContainers[T::type].get())->components;
    }

    template<typename T>
    const auto& getComponentContainer() const
    {
        return static_cast<const ComponentContainer<T>*>(mComponentContainers[T::type].get())->components;
    }

    template<typename ...Ts, std::size_t ...Is>
    std::tuple<Ts&...> getComponentsByIds(const std::array<ComponentId, sizeof...(Ts)>& componentIds, std::index_sequence<Is...>)
    {
        checkComponentTypes<Ts...>();
        return std::tie(getComponentContainer<Ts>().get(componentIds[Is])...);
    }

    template<typename ...Ts, std::size_t ...Is>
    std::tuple<const Ts&...> getComponentsByIds(const std::array<ComponentId, sizeof...(Ts)>& componentIds, std::index_sequence<Is...>) const
    {
        checkComponentTypes<Ts...>();
        return std::tie(std::as_const(getComponentContainer<Ts>().get(componentIds[Is]))...);
    }
};

}