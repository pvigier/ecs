#pragma once

#include <memory>
#include "EntityContainer.h"
#include "ComponentContainer.h"
#include "System.h"

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

    template<typename T>
    void registerComponent()
    {
        checkComponentType<T>();
        mComponentContainers[T::type] = std::make_unique<ComponentContainer<T>>(
            mEntities.getEntityToComponent(T::type));
    }

    template<typename T, typename ...Args>
    T* createSystem(Args&& ...args)
    {
        auto type = mSystems.size();
        auto& system = mSystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        system->setUp(type, &mEntities.getEntityToManagedEntity(type), &mEntities);
        return static_cast<T*>(system.get());
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
        // Send message to systems
        for (auto& system : mSystems)
            system->onEntityRemoved(entity);
        // Remove entity
        mEntities.remove(entity);
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
        // Send message to systems
        for (auto& system : mSystems)
            system->onEntityUpdated(entity);
    }

    template<typename T>
    void removeComponent(Entity entity)
    {
        checkComponentType<T>();
        getComponentContainer<T>()->remove(entity);
        // Send message to systems
        for (auto& system : mSystems)
            system->onEntityUpdated(entity);
    }

    template<typename T>
    Entity getOwner(const T& component) const
    {
        checkComponentType<T>();
        return getComponentContainer<T>()->getOwner(component);
    }

private:
    std::vector<std::unique_ptr<BaseComponentContainer>> mComponentContainers;
    EntityContainer mEntities;
    std::vector<std::unique_ptr<System>> mSystems;

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