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
    virtual void addEntity(Entity entity) = 0;
    virtual void removeEntity(Entity entity) = 0;

    std::unordered_map<Entity, std::size_t> mEntityToIndex;
private:
    bool hasEntity(Entity entity) const
    {
        return mEntityToIndex.find(entity) != std::end(mEntityToIndex);
    }

};

template<typename ...Ts>
class EntitySet : public BaseEntitySet
{
public:
    using ValueType = std::pair<Entity, std::array<ComponentId, sizeof...(Ts)>>;

    EntitySet(const EntityContainer* entities) : mEntities(entities)
    {

    }

    const std::vector<ValueType>& getEntities() const
    {
        return mManagedEntities;
    }

protected:
    virtual bool satisfyRequirements(Entity entity) override
    {
        // TODO: to improve: this is a duplicate of EntityManager::hasComponents
        auto& componentIds = mEntities->get(entity);
        return ((componentIds.find(Ts::type) != std::end(componentIds)) && ...);
    }

    virtual void addEntity(Entity entity) override
    {
        mEntityToIndex[entity] = mManagedEntities.size();
        const auto& componentsIds = mEntities->get(entity);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wnull-dereference"
        mManagedEntities.emplace_back(entity, std::array<ComponentId, sizeof...(Ts)>{componentsIds.find(Ts::type)->second...});
        #pragma GCC diagnostic pop
        // TODO: send event
    }

    virtual void removeEntity(Entity entity) override
    {
        // TODO: send event
        auto it = mEntityToIndex.find(entity);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wnull-dereference"
        auto index = it->second;
        #pragma GCC diagnostic pop
        mEntityToIndex[mManagedEntities.back().first] = index;
        mEntityToIndex.erase(it);
        mManagedEntities[index] = mManagedEntities.back();
        mManagedEntities.pop_back();
    }

private:
    std::vector<ValueType> mManagedEntities;
    const EntityContainer* mEntities = nullptr;
};

}