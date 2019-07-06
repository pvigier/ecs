#pragma once

#include <memory>
#include "EntityContainer.h"
#include "ComponentContainer.h"
#include "System.h"

namespace ecs
{

template<typename T>
class Component;

template<std::size_t ComponentCount, std::size_t SystemCount>
class EntityManager
{
public:
    template<typename T>
    void registerComponent()
    {
        checkComponentType<T>();
        mComponentContainers[T::type] = std::make_unique<ComponentContainer<T, ComponentCount, SystemCount>>(
            mEntities.getEntityToComponent(T::type), mEntities.getEntityToBitset());
    }

    template<typename T, typename ...Args>
    T* createSystem(Args&& ...args)
    {
        auto type = mSystems.size();
        auto& system = mSystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        system->setUp(type, &mEntities.getEntityToManagedEntity(type));
        return static_cast<T*>(system.get());
    }

    void reserve(std::size_t size)
    {
        for (auto i = std::size_t(0); i < ComponentCount; ++i)
        {
            if (mComponentContainers[i])
                mComponentContainers[i]->reserve(size);
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
        for (auto i = std::size_t(0); i < ComponentCount; ++i)
        {
            if (mComponentContainers[i])
                mComponentContainers[i]->tryRemove(entity);
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
        return mEntities.getBitset(entity)[T::type];
    }

    template<typename ...Ts>
    bool hasComponents(Entity entity) const
    {
        checkComponentTypes<Ts...>();
        auto requirements = std::bitset<ComponentCount>();
        (requirements.set(Ts::type), ...);
        return (requirements & mEntities.getBitset(entity)) == requirements;
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

    template<typename T, typename... Args>
    void addComponent(Entity entity, Args&&... args)
    {
        checkComponentType<T>();
        getComponentContainer<T>()->add(entity, std::forward<Args>(args)...);
        // Send message to systems
        const auto& bitset = mEntities.getBitset(entity);
        for (auto& system : mSystems)
            system->onEntityUpdated(entity, bitset);
    }

    template<typename T>
    void removeComponent(Entity entity)
    {
        checkComponentType<T>();
        getComponentContainer<T>()->remove(entity);
        // Send message to systems
        const auto& bitset = mEntities.getBitset(entity);
        for (auto& system : mSystems)
            system->onEntityUpdated(entity, bitset);
    }

    template<typename T>
    Entity getOwner(const T& component) const
    {
        checkComponentType<T>();
        return getComponentContainer<T>()->getOwner(component);
    }

private:
    std::array<std::unique_ptr<BaseComponentContainer>, ComponentCount> mComponentContainers; // TODO: Benchmark against vector and runtime types
    EntityContainer<ComponentCount, SystemCount> mEntities;
    std::vector<std::unique_ptr<System<ComponentCount, SystemCount>>> mSystems;

    template<typename T>
    void checkComponentType() const
    {
        static_assert(std::is_base_of_v<Component<T>, T>);
    }

    template<typename ...Ts>
    void checkComponentTypes() const
    {
        (checkComponentType<Ts>(), ...);
    }

    template<typename T>
    auto getComponentContainer()
    {
        return static_cast<ComponentContainer<T, ComponentCount, SystemCount>*>(mComponentContainers[T::type].get());
    }

    template<typename T>
    auto getComponentContainer() const
    {
        return static_cast<const ComponentContainer<T, ComponentCount, SystemCount>*>(mComponentContainers[T::type].get());
    }
};

}