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
    BaseEntitySet(const EntitySetId& id, std::vector<Index>* entityToManagedEntity) :
        mId(id), mEntityToManagedEntity(entityToManagedEntity)
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

    const EntitySetId& getId() const
    {
        return mId;
    }

protected:
    virtual bool satisfyRequirements(Entity entity) = 0;

private:
    const EntitySetId mId;
    std::vector<Entity> mManagedEntities;
    std::vector<Index>* mEntityToManagedEntity = nullptr;

    void addEntity(Entity entity)
    {
        (*mEntityToManagedEntity)[entity] = static_cast<Index>(mManagedEntities.size());
        mManagedEntities.emplace_back(entity);
        // TODO: send event
    }

    void removeEntity(Entity entity)
    {
        // TODO: send event
        auto index = (*mEntityToManagedEntity)[entity];
        (*mEntityToManagedEntity)[mManagedEntities.back()] = index;
        (*mEntityToManagedEntity)[entity] = InvalidIndex;
        mManagedEntities[index] = mManagedEntities.back();
        mManagedEntities.pop_back();
    }
};

template<typename ...Ts>
class EntitySet : public BaseEntitySet
{
public:
    EntitySet(const EntityContainer* entities, std::vector<Index>* entityToManagedEntity) :
        BaseEntitySet({Ts::type...}, entityToManagedEntity), mEntities(entities)
    {

    }

protected:
    virtual bool satisfyRequirements(Entity entity) override
    {
        return mEntities->hasComponents<Ts...>(entity);
    }

private:
    const EntityContainer* mEntities = nullptr;
};

}