#pragma once

#include <functional>
#include <vector>
#include "EntityContainer.h"

namespace ecs
{

using EntitySetId = std::vector<std::size_t>;

class BaseEntitySet
{
public:
    BaseEntitySet()
    {

    }

    virtual ~BaseEntitySet() = default;

    const std::vector<Entity>& getEntities() const
    {
        return mManagedEntities;
    }

    void onEntityUpdated(Entity entity)
    {
        auto satisfied = satisfyRequirements(entity);
        auto managed = hasEntity(entity);
        if (satisfied && !managed)
            addEntity(entity);
        else if (!satisfied && managed)
            removeEntity(entity);
    }

    void onEntityRemoved(Entity entity)
    {
        if (hasEntity(entity))
            removeEntity(entity);
    }

protected:
    virtual bool satisfyRequirements(Entity entity) = 0;

private:
    const EntitySetId mId;
    std::vector<Entity> mManagedEntities;
    std::unordered_map<Entity, std::size_t> mEntityToIndex;

    bool hasEntity(Entity entity) const
    {
        return mEntityToIndex.find(entity) != std::end(mEntityToIndex);
    }

    void addEntity(Entity entity)
    {
        mEntityToIndex[entity] = mManagedEntities.size();
        mManagedEntities.emplace_back(entity);
        // TODO: send event
    }

    void removeEntity(Entity entity)
    {
        // TODO: send event
        auto it = mEntityToIndex.find(entity);
        auto index = it->second;
        mEntityToIndex[mManagedEntities.back()] = index;
        mEntityToIndex.erase(it);
        mManagedEntities[index] = mManagedEntities.back();
        mManagedEntities.pop_back();
    }
};

template<typename ...Ts>
class EntitySet : public BaseEntitySet
{
public:
    EntitySet(const EntityContainer* entities) : mEntities(entities)
    {

    }

protected:
    virtual bool satisfyRequirements(Entity entity) override
    {
        // TODO: to improve: this is a duplicate of EntityManager::hasComponents
        auto& componentIds = mEntities->get(entity);
        return ((componentIds.find(Ts::type) != std::end(componentIds)) && ...);
    }

private:
    const EntityContainer* mEntities = nullptr;
};

}