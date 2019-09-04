#pragma once

#include "ComponentType.h"

namespace ecs
{

template<typename T>
class Component
{
public:
    static const ComponentType type;
};

ComponentType generateComponentType()
{
    static auto counter = ComponentType(0);
    return counter++;
}

template<typename T>
const ComponentType Component<T>::type = generateComponentType();

}