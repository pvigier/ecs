#pragma once

#include "ComponentSparseSet.h"

namespace ecs
{

struct BaseComponentContainer
{
    virtual ~BaseComponentContainer() = default;

    virtual void reserve(std::size_t size) = 0;
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

    virtual void remove(ComponentId componentId)
    {
        components.erase(componentId);
    }
};

}