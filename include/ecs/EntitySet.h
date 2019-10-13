#pragma once

#include <functional>
#include <memory>
#include "ComponentContainer.h"
#include "EntitySetIterator.h"
#include "EntitySetType.h"
#include "EntityContainer.h"

namespace ecs
{

template<typename ...Ts>
class EntitySet;

class BaseEntitySet
{
public:
    static std::size_t getEntitySetCount()
    {
        return sFactories.size();
    }

    static std::unique_ptr<BaseEntitySet> createEntitySet(std::size_t type,
        EntityContainer& entities,
        const std::vector<std::unique_ptr<BaseComponentContainer>>& componentContainers,
        std::vector<std::vector<BaseEntitySet*>>& componentToEntitySets)
    {
        return sFactories[type](entities, componentContainers, componentToEntitySets);
    }

    BaseEntitySet()
    {

    }

    virtual ~BaseEntitySet() = default;

    bool hasEntity(Entity entity) const
    {
        return mEntityToIndex.find(entity) != std::end(mEntityToIndex);
    }

    void onEntityUpdated(Entity entity)
    {
        auto satisfied = satisfyRequirements(entity);
        auto managed = hasEntity(entity);
        if (satisfied && !managed)
            addEntity(entity);
        else if (!satisfied && managed)
            removeEntity(entity, true);
    }

    void onEntityRemoved(Entity entity)
    {
        removeEntity(entity, false);
    }

protected:
    virtual bool satisfyRequirements(Entity entity) = 0;
    virtual void addEntity(Entity entity) = 0;
    virtual void removeEntity(Entity entity, bool updateEntity) = 0;

    std::unordered_map<Entity, std::size_t> mEntityToIndex;

    template<typename ...Ts>
    static EntitySetType generateEntitySetType()
    {
        sFactories.push_back([](EntityContainer& entities,
            const std::vector<std::unique_ptr<BaseComponentContainer>>& componentContainers,
            std::vector<std::vector<BaseEntitySet*>>& componentToEntitySets)
            -> std::unique_ptr<BaseEntitySet>
        {
            auto entitySet = std::make_unique<EntitySet<Ts...>>(entities,
                std::tie(static_cast<ComponentContainer<Ts>*>(componentContainers[Ts::Type].get())->components...));
            (componentToEntitySets[Ts::Type].push_back(entitySet.get()), ...);
            return std::move(entitySet);
        });
        return sFactories.size() - 1;
    }

private:
    using EntitySetFactory = std::unique_ptr<BaseEntitySet>(*)(
        EntityContainer&,
        const std::vector<std::unique_ptr<BaseComponentContainer>>&,
        std::vector<std::vector<BaseEntitySet*>>&);

    static std::vector<EntitySetFactory> sFactories;
};

inline std::vector<BaseEntitySet::EntitySetFactory> BaseEntitySet::sFactories;

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

    static const EntitySetType Type;

    EntitySet(EntityContainer& entities, const ComponentContainers& componentContainers) :
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
    bool satisfyRequirements(Entity entity) override
    {
        return mEntities.get(entity).template hasComponents<Ts...>();
    }

    void addEntity(Entity entity) override
    {
        mEntityToIndex[entity] = mManagedEntities.size();
        auto& entityData = mEntities.get(entity);
        entityData.addEntitySet(Type);
        mManagedEntities.emplace_back(entity, std::array<ComponentId, sizeof...(Ts)>{entityData.template getComponent<Ts>()...});
        // Call listeners
        for (const auto& listener : mEntityAddedListeners.getObjects())
            listener(entity);
    }

    void removeEntity(Entity entity, bool updateEntity) override
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
        if (updateEntity)
            mEntities.get(entity).removeEntitySet(Type);
    }

private:
    std::vector<ValueType> mManagedEntities;
    EntityContainer& mEntities;
    ComponentContainers mComponentContainers;
    SparseSet<ListenerId, EntityAddedListener> mEntityAddedListeners;
    SparseSet<ListenerId, EntityRemovedListener> mEntityRemovedListeners;
};

template<typename ...Ts>
const EntitySetType EntitySet<Ts...>::Type = BaseEntitySet::generateEntitySetType<Ts...>();

}