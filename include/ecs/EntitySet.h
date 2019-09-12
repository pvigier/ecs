#pragma once

#include "EntityContainer.h"
#include "EntitySetIterator.h"

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
    using ValueType = std::pair<Entity, std::array<ComponentId, sizeof...(Ts)>>;
    using UIterator = typename std::vector<ValueType>::iterator; // Underlying iterator
    using UConstIterator = typename std::vector<ValueType>::const_iterator; // Underlying const iterator
    using ComponentContainers = std::tuple<ComponentSparseSet<Ts>&...>;

public:
    using Iterator = EntitySetIterator<UIterator, Ts...>;
    using ConstIterator = EntitySetIterator<UConstIterator, const Ts...>;
    using ListenerId = uint32_t;
    using EntityAddedListener = std::function<void(Entity)>;
    using EntityRemovedListener = std::function<void(Entity)>;

    EntitySet(const EntityContainer* entities, const ComponentContainers& componentContainers) :
        mEntities(entities), mComponentContainers(componentContainers)
    {

    }

    std::size_t getSize() const
    {
        return mManagedEntities.size();
    }

    Iterator begin()
    {
        return Iterator(mManagedEntities.begin(), mComponentContainers);
    }

    ConstIterator begin() const
    {
        return ConstIterator(mManagedEntities.begin(), mComponentContainers);
    }

    Iterator end()
    {
        return Iterator(mManagedEntities.end(), mComponentContainers);
    }

    ConstIterator end() const
    {
        return ConstIterator(mManagedEntities.end(), mComponentContainers);
    }

    // Listeners

    ListenerId addEntityAddedListener(EntityAddedListener listener)
    {
        return mEntityAddedListeners.emplace(std::move(listener)).first;
    }

    void removeEntityAddedListener(ListenerId listenerId)
    {
        mEntityAddedListeners.erase(listenerId);
    }

    ListenerId addEntityRemovedListener(EntityRemovedListener listener)
    {
        return mEntityRemovedListeners.emplace(std::move(listener)).first;
    }

    void removeEntityRemovedListener(ListenerId listenerId)
    {
        mEntityRemovedListeners.erase(listenerId);
    }

protected:
    virtual bool satisfyRequirements(Entity entity) override
    {
        // TODO: to improve: this is a duplicate of EntityManager::hasComponents
        auto& componentIds = mEntities->get(entity);
        return ((componentIds.find(Ts::Type) != std::end(componentIds)) && ...);
    }

    virtual void addEntity(Entity entity) override
    {
        mEntityToIndex[entity] = mManagedEntities.size();
        const auto& componentsIds = mEntities->get(entity);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wnull-dereference"
        mManagedEntities.emplace_back(entity, std::array<ComponentId, sizeof...(Ts)>{componentsIds.find(Ts::Type)->second...});
        #pragma GCC diagnostic pop
        // Call listeners
        for (const auto& listener : mEntityAddedListeners.getObjects())
            listener(entity);
    }

    virtual void removeEntity(Entity entity) override
    {
        // Call listeners
        for (const auto& listener : mEntityRemovedListeners.getObjects())
            listener(entity);
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
    ComponentContainers mComponentContainers;
    SparseSet<ListenerId, EntityAddedListener> mEntityAddedListeners;
    SparseSet<ListenerId, EntityRemovedListener> mEntityRemovedListeners;
};

}