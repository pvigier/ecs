#pragma once

#include "ComponentType.h"

namespace ecs
{

class BaseComponent
{
public:
    static std::size_t getComponentCount()
    {
        return sCounter;
    }

protected:
    static ComponentType generateComponentType()
    {
        return static_cast<ComponentType>(sCounter++);
    }

private:
    static std::size_t sCounter;
};

std::size_t BaseComponent::sCounter = 0;

template<typename T>
class Component : private BaseComponent
{
public:
    static const ComponentType Type;
};

template<typename T>
const ComponentType Component<T>::Type = BaseComponent::generateComponentType();

}