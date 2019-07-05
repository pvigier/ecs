#pragma once

#include <array>
#include <bitset>
#include <vector>
#include <numeric>
#include "Entity.h"

namespace ecs
{

template<std::size_t ComponentCount, std::size_t SystemCount>
class EntityContainer
{
public:
    void reserve(std::size_t size)
    {
        mFreeEntities.resize(size);
        std::iota(std::begin(mFreeEntities), std::end(mFreeEntities), 0);
        mEntityToBitset.resize(size);
        for (auto& entityToComponent : mEntityToComponents)
            entityToComponent.resize(size);
        for (auto& entityToManagedEntity : mEntityToManagedEntities)
            entityToManagedEntity.resize(size);
    }

    std::vector<std::bitset<ComponentCount>>& getEntityToBitset()
    {
        return mEntityToBitset;
    }

    const std::bitset<ComponentCount>& getBitset(Entity entity) const
    {
        return mEntityToBitset[entity];
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
            entity = static_cast<Entity>(mEntityToBitset.size());
            mEntityToBitset.emplace_back();
            for (auto& entityToComponent : mEntityToComponents)
                entityToComponent.emplace_back();
            for (auto& entityToManagedEntity : mEntityToManagedEntities)
                entityToManagedEntity.emplace_back(InvalidIndex);
        }
        else
        {
            entity = mFreeEntities.back();
            mFreeEntities.pop_back();
            mEntityToBitset[entity].reset();
            for (auto& entityToManagedEntity : mEntityToManagedEntities)
                entityToManagedEntity[entity] = InvalidIndex;
        }
        return entity;
    }

    void remove(Entity entity)
    {
        mFreeEntities.push_back(entity);
    }

private:
    std::vector<std::bitset<ComponentCount>> mEntityToBitset;
    std::array<std::vector<Index>, ComponentCount> mEntityToComponents;
    std::array<std::vector<Index>, SystemCount> mEntityToManagedEntities;
    std::vector<Entity> mFreeEntities;
};

}