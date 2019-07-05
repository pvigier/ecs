#pragma once

#include <bitset>
#include <map>
#include <vector>
#include "Entity.h"

namespace ecs
{

class BaseComponentContainer
{
public:
    virtual ~BaseComponentContainer() = default;

    virtual void reserve(std::size_t size) = 0;
    virtual bool tryRemove(Entity entity) = 0;
};

template<typename T, std::size_t ComponentCount, std::size_t SystemCount>
class ComponentContainer : public BaseComponentContainer
{
public:
    ComponentContainer(std::vector<std::bitset<ComponentCount>>& entityToBitset) :
        mEntityToBitset(entityToBitset)
    {

    }

    virtual void reserve(std::size_t size) override
    {
        mComponents.reserve(size);
        mComponentToEntity.reserve(size);
    }

    T& get(Entity entity)
    {
        return mComponents[mEntityToComponent[entity]];
    }

    const T& get(Entity entity) const
    {
        return mComponents[mEntityToComponent.find(entity)->second];
    }

    template<typename... Args>
    void add(Entity entity, Args&&... args)
    {
        auto index = static_cast<Index>(mComponents.size());
        mComponents.emplace_back(std::forward<Args>(args)...);
        mComponentToEntity.emplace_back(entity);
        mEntityToComponent[entity] = index;
        mEntityToBitset[entity][T::type] = true;
    }

    void remove(Entity entity)
    {
        mEntityToBitset[entity][T::type] = false;
        auto index = mEntityToComponent[entity];
        // Update mComponents
        mComponents[index] = std::move(mComponents.back());
        mComponents.pop_back();
        // Update mEntityToComponent
        mEntityToComponent[mComponentToEntity.back()] = index;
        mEntityToComponent.erase(entity);
        // Update mComponentToEntity
        mComponentToEntity[index] = mComponentToEntity.back();
        mComponentToEntity.pop_back();
    }

    virtual bool tryRemove(Entity entity) override
    {
        if (mEntityToBitset[entity][T::type])
        {
            remove(entity);
            return true;
        }
        return false;
    }

    Entity getOwner(const T& component) const
    {
        auto begin = mComponents.data();
        auto index = static_cast<std::size_t>(&component - begin);
        return mComponentToEntity[index];
    }

private:
    std::vector<T> mComponents;
    std::vector<Entity> mComponentToEntity;
    std::map<Entity, Index> mEntityToComponent;
    std::vector<std::bitset<ComponentCount>>& mEntityToBitset;
};

}