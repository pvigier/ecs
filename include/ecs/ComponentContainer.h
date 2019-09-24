#pragma once

#include "ComponentSparseSet.h"

namespace ecs
{

struct BaseComponent;

struct BaseComponentContainer
{
    virtual ~BaseComponentContainer() = default;

    virtual void reserve(std::size_t size) = 0;
    virtual BaseComponent& get(ComponentId componentId) = 0;
    virtual void remove(ComponentId componentId) = 0;
};

template<typename T>
struct ComponentContainer : public BaseComponentContainer
{
    ComponentSparseSet<T> components;

    virtual void reserve(std::size_t size) override
    {
        components.reserve(size);
    }

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