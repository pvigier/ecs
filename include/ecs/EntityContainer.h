#pragma once

#include <unordered_map>
#include <unordered_set>
#include "ComponentId.h"
#include "ComponentType.h"
#include "Entity.h"
#include "EntitySetType.h"
#include "SparseSet.h"

namespace ecs
{

class EntityData
{
    using ComponentIdContainer = std::unordered_map<ComponentType, ComponentId>;
    using EntitySetTypeContainer = std::unordered_set<EntitySetType>;

public:
    // Components

    template<typename T>
    bool hasComponent() const
    {
        return findComponent<T>() != std::end(mComponentIds);
    }

    template<typename ...Ts>
    bool hasComponents() const
    {
        return (hasComponent<Ts>() && ...);
    }

    template<typename T>
    ComponentId getComponent() const
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wnull-dereference"
        return findComponent<T>()->second;
        #pragma GCC diagnostic pop
    }

    const ComponentIdContainer& getComponents() const
    {
        return mComponentIds;
    }

    template<typename T>
    void addComponent(ComponentId componentId)
    {
        mComponentIds[T::Type] = componentId;
    }

    template<typename T>
    ComponentId removeComponent()
    {
        auto it = findComponent<T>();
        auto componentId = it->second;
        mComponentIds.erase(it);
        return componentId;
    }

    // Types

    const EntitySetTypeContainer& getEntitySets() const
    {
        return mEntitySetTypes;
    }

    void addEntitySet(EntitySetType entitySetType)
    {
        mEntitySetTypes.insert(entitySetType);
    }

    void removeEntitySet(EntitySetType entitySetType)
    {
        mEntitySetTypes.erase(entitySetType);
    }

private:
    ComponentIdContainer mComponentIds;
    EntitySetTypeContainer mEntitySetTypes;

    template<typename T>
    ComponentIdContainer::const_iterator findComponent() const
    {
        return mComponentIds.find(T::Type);
    }
};

using EntityContainer = SparseSet<Entity, EntityData>;

}