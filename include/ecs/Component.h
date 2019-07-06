#pragma once

#include <cstddef>

namespace ecs
{

template<typename T>
class Component
{
public:
    static const std::size_t type;
};

std::size_t generateComponentType()
{
    static auto counter = std::size_t(0);
    return counter++;
}

template<typename T>
const std::size_t Component<T>::type = generateComponentType();

}