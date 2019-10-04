#pragma once

#include "ComponentSparseSet.h"

namespace ecs
{

struct BaseComponent;

struct BaseComponentContainer
{
    virtual ~BaseComponentContainer() = default;

    virtual BaseComponent& get(ComponentId componentId) = 0;
    virtual void remove(ComponentId componentId) = 0;
};

template<typename T>
struct ComponentContainer : public BaseComponentContainer
{
    ComponentSparseSet<T> components;

    virtual BaseComponent& get(ComponentId componentId) override
    {
        return components.get(componentId);
    }

    virtual void remove(ComponentId componentId)
    {
        components.erase(componentId);
    }
};

}