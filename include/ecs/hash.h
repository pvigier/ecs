#pragma once

#include <functional>
#include "EntitySet.h"

namespace ecs
{

template<typename T>
void hash_combine(std::size_t& seed, const T& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct HashEntitySetId
{
    std::size_t operator()(const EntitySetId& id) const
    {
        auto result = std::hash<std::size_t>()(id.size());
        for (const auto& type : id)
            hash_combine(result, type);
        return result;
    }
};

}