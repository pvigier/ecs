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
    }

    std::vector<std::bitset<ComponentCount>>& getEntityToBitset()
    {
        return mEntityToBitset;
    }

    const std::bitset<ComponentCount>& getBitset(Entity entity) const
    {
        return mEntityToBitset[entity];
    }

    Entity create()
    {
        auto entity = Entity();
        if (mFreeEntities.empty())
        {
            entity = static_cast<Entity>(mEntityToBitset.size());
            mEntityToBitset.emplace_back();
        }
        else
        {
            entity = mFreeEntities.back();
            mFreeEntities.pop_back();
            mEntityToBitset[entity].reset();
        }
        return entity;
    }

    void remove(Entity entity)
    {
        mFreeEntities.push_back(entity);
    }

private:
    std::vector<std::bitset<ComponentCount>> mEntityToBitset;
    std::vector<Entity> mFreeEntities;
};

}