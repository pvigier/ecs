#pragma once

#include <array>
#include <tuple>
#include <utility>
#include "ComponentSparseSet.h"
#include "Entity.h"

namespace ecs
{

template<typename Iterator, typename ...Ts>
class EntitySetIterator
{
    using ComponentContainers = std::tuple<ComponentSparseSet<std::remove_const_t<Ts>>&...>;

public:
    explicit EntitySetIterator(Iterator it, const ComponentContainers& componentContainers) :
        mIt(it), mComponentContainers(componentContainers)
    {

    }

    bool operator!=(const EntitySetIterator<Iterator, Ts...>& it)
    {
        return mIt != it.mIt;
    }

    std::pair<Entity, std::tuple<Ts&...>> operator*()
    {
        return std::pair(mIt->first, getComponentsByIds(mIt->second, std::index_sequence_for<Ts...>{}));
    }

    EntitySetIterator<Iterator, Ts...>& operator++()
    {
        ++mIt;
        return *this;
    }

private:
    Iterator mIt;
    const ComponentContainers& mComponentContainers; // MAYBE: just copy the references

    template<std::size_t ...Is>
    std::tuple<Ts&...> getComponentsByIds(const std::array<ComponentId, sizeof...(Ts)>& componentIds, std::index_sequence<Is...>)
    {
        return std::tie(std::get<Is>(mComponentContainers).get(componentIds[Is])...);
    }

    template<std::size_t ...Is>
    std::tuple<const Ts&...> getComponentsByIds(const std::array<ComponentId, sizeof...(Ts)>& componentIds, std::index_sequence<Is...>) const
    {
        return std::tie(std::as_const(std::get<Is>(mComponentContainers).get(componentIds[Is]))...);
    }
};

}