#pragma once

#include "ComponentId.h"
#include "SparseSet.h"

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
    SparseSet<ComponentId, T> components;

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