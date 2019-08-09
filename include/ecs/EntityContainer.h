#pragma once

#include <array>
#include <vector>
#include <numeric>
#include "Entity.h"

namespace ecs
{

template<std::size_t ComponentCount, std::size_t SystemCount>
class EntityContainer
{
public:
    // Should only be called once before creating entities
    void reserve(std::size_t size)
    {
        mFreeEntities.resize(size);
        std::iota(std::begin(mFreeEntities), std::end(mFreeEntities), 0);
        for (auto& entityToComponent : mEntityToComponents)
            entityToComponent.resize(size);
        for (auto& entityToManagedEntity : mEntityToManagedEntities)
            entityToManagedEntity.resize(size);
        mNextEntity = static_cast<Entity>(size);
    }

    std::vector<Index>& getEntityToComponent(std::size_t type)
    {
        return mEntityToComponents[type];
    }

    std::vector<Index>& getEntityToManagedEntity(std::size_t type)
    {
        return mEntityToManagedEntities[type];
    }

    Entity create()
    {
        auto entity = Entity();
        if (mFreeEntities.empty())
        {
            entity = mNextEntity;
            ++mNextEntity;
            for (auto& entityToComponent : mEntityToComponents)
                entityToComponent.emplace_back(InvalidIndex);
            for (auto& entityToManagedEntity : mEntityToManagedEntities)
                entityToManagedEntity.emplace_back(InvalidIndex);
        }
        else
        {
            entity = mFreeEntities.back();
            mFreeEntities.pop_back();
            for (auto& entityToComponent : mEntityToComponents)
                entityToComponent[entity] = InvalidIndex;
            for (auto& entityToManagedEntity : mEntityToManagedEntities)
                entityToManagedEntity[entity] = InvalidIndex;
        }
        return entity;
    }

    void remove(Entity entity)
    {
        mFreeEntities.push_back(entity);
    }

    template<typename T>
    bool hasComponent(Entity entity) const
    {
        return mEntityToComponents[T::type][entity] != InvalidIndex;
    }

    template<typename ...Ts>
    bool hasComponents(Entity entity) const
    {
        return (hasComponent<Ts>(entity) && ...);
    }

private:
    Entity mNextEntity = 0;
    std::array<std::vector<Index>, ComponentCount> mEntityToComponents;
    std::array<std::vector<Index>, SystemCount> mEntityToManagedEntities;
    std::vector<Entity> mFreeEntities;
};

}