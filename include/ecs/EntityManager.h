#pragma once

#include "EntitySet.h"
#include "Visitor.h"

namespace ecs
{

template<typename T>
class Component;

class EntityManager
{
public:
    static constexpr auto UndefinedEntity = static_cast<Entity>(std::numeric_limits<std::underlying_type_t<Entity>>::max());

    EntityManager()
    {
        auto nbComponents = BaseComponent::getComponentCount();
        // Component containers
        mComponentContainers.resize(nbComponents);
        for (auto type = std::size_t(0); type < mComponentContainers.size(); ++type)
            mComponentContainers[type] = BaseComponent::createComponentContainer(type);
        // Entity sets
        mComponentToEntitySets.resize(nbComponents);
        mEntitySets.resize(BaseEntitySet::getEntitySetCount());
        for (auto type = std::size_t(0); type < mEntitySets.size(); ++type)
            mEntitySets[type] = BaseEntitySet::createEntitySet(type, mEntities, mComponentContainers, mComponentToEntitySets);
    }

    void reserve(std::size_t size)
    {
        mEntities.reserve(size);
    }

    // Entities

    bool hasEntity(Entity entity) const
    {
        return mEntities.has(entity);
    }

    Entity createEntity()
    {
        return mEntities.emplace().first;
    }

    void removeEntity(Entity entity)
    {
        const auto& entityData = mEntities.get(entity);
        // Remove components
        for (auto& [componentType, componentId] : entityData.getComponents())
            mComponentContainers[componentType]->remove(componentId);
        // Send message to entity sets
        for (auto entitySetType : entityData.getEntitySets())
            mEntitySets[entitySetType]->onEntityRemoved(entity);
        // Remove entity
        mEntities.erase(entity);
    }

    void visitEntity(Entity entity, const Visitor& visitor)
    {
        for (const auto& [componentType, componentId] : mEntities.get(entity).getComponents())
            visitor.handle(componentType, mComponentContainers[componentType]->get(componentId));
    }

    // Components

    template<typename T>
    bool hasComponent(Entity entity) const
    {
        checkComponentType<T>();
        return mEntities.get(entity).hasComponent<T>();
    }

    template<typename ...Ts>
    bool hasComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        return mEntities.get(entity).hasComponents<Ts...>();
    }

    template<typename T>
    T& getComponent(Entity entity)
    {
        checkComponentType<T>();
        return getComponentSparseSet<T>().get(mEntities.get(entity).getComponent<T>());
    }

    template<typename T>
    const T& getComponent(Entity entity) const
    {
        checkComponentType<T>();
        return getComponentSparseSet<T>().get(mEntities.get(entity).getComponent<T>());
    }

    template<typename ...Ts>
    std::tuple<Ts&...> getComponents(Entity entity)
    {
        checkComponentTypes<Ts...>();
        auto& entityData = mEntities.get(entity);
        return std::tie(getComponentSparseSet<Ts>().get(entityData.getComponent<Ts>())...);
    }

    template<typename ...Ts>
    std::tuple<const Ts&...> getComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        auto& entityData = mEntities.get(entity);
        return std::tie(std::as_const(getComponentSparseSet<Ts>().get(entityData.getComponent<Ts>()))...);
    }

    template<typename T, typename ...Args>
    T& addComponent(Entity entity, Args&&... args)
    {
        checkComponentType<T>();
        auto [componentId, component] = getComponentSparseSet<T>().emplace(std::forward<Args>(args)...);
        mEntities.get(entity).addComponent<T>(componentId);
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
        // Remove component from entity and component container
        getComponentSparseSet<T>().erase(mEntities.get(entity).removeComponent<T>());
        // Send message to entity sets
        for (auto entitySet : mComponentToEntitySets[T::Type])
            entitySet->onEntityUpdated(entity);
    }

    // Entity sets

    template<typename ...Ts>
    EntitySet<Ts...>& getEntitySet()
    {
        checkComponentTypes<Ts...>();
        return *static_cast<EntitySet<Ts...>*>(mEntitySets[EntitySet<Ts...>::Type].get());
    }

    template<typename ...Ts>
    const EntitySet<Ts...>& getEntitySet() const
    {
        checkComponentTypes<Ts...>();
        return *static_cast<EntitySet<Ts...>*>(mEntitySets[EntitySet<Ts...>::Type].get());
    }

private:
    std::vector<std::unique_ptr<BaseComponentContainer>> mComponentContainers;
    EntityContainer mEntities;
    std::vector<std::unique_ptr<BaseEntitySet>> mEntitySets;
    std::vector<std::vector<BaseEntitySet*>> mComponentToEntitySets;


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