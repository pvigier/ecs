#pragma once

#include <functional>
#include <vector>
#include "EntityContainer.h"

namespace ecs
{

class EntityManager;

class System
{
public:
    virtual ~System() = default;

protected:
    template<typename ...Ts>
    void setRequirements()
    {
        mCheckRequirements = [this](Entity entity)
        {
            return mEntities->hasComponents<Ts...>(entity);
        };
    }

    const std::vector<Entity>& getManagedEntities() const
    {
        return mManagedEntities;
    }

    virtual void onManagedEntityAdded([[maybe_unused]] Entity entity)
    {

    }

    virtual void onManagedEntityRemoved([[maybe_unused]] Entity entity)
    {

    }

private:
    friend EntityManager;

    std::function<bool(Entity)> mCheckRequirements;
    std::size_t mType;
    std::vector<Entity> mManagedEntities;
    std::vector<Index>* mEntityToManagedEntity = nullptr;
    const EntityContainer* mEntities = nullptr;

    void setUp(std::size_t type, std::vector<Index>* entityToManagedEntity,
        const EntityContainer* entities)
    {
        mType = type;
        mEntityToManagedEntity = entityToManagedEntity;
        mEntities = entities;
    }

    void onEntityUpdated(Entity entity)
    {
        auto satisfied = mCheckRequirements(entity);
        auto managed = (*mEntityToManagedEntity)[entity] != InvalidIndex;
        if (satisfied && !managed)
            addEntity(entity);
        else if (!satisfied && managed)
            removeEntity(entity);
    }

    void onEntityRemoved(Entity entity)
    {
        if ((*mEntityToManagedEntity)[entity] != InvalidIndex)
            removeEntity(entity);
    }

    void addEntity(Entity entity)
    {
        (*mEntityToManagedEntity)[entity] = static_cast<Index>(mManagedEntities.size());
        mManagedEntities.emplace_back(entity);
        onManagedEntityAdded(entity);
    }

    void removeEntity(Entity entity)
    {
        onManagedEntityRemoved(entity);
        auto index = (*mEntityToManagedEntity)[entity];
        (*mEntityToManagedEntity)[mManagedEntities.back()] = index;
        (*mEntityToManagedEntity)[entity] = InvalidIndex;
        mManagedEntities[index] = mManagedEntities.back();
        mManagedEntities.pop_back();
    }
};

}