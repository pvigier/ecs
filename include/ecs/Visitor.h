#pragma once

#include <functional>
#include "Component.h"

namespace ecs
{

class Visitor
{
public:
    Visitor()
    {
        mHandlers.resize(BaseComponent::getComponentCount());
    }

    template<typename T, typename Callable>
    void setHandler(Callable&& callable)
    {
        checkComponentType<T>();
        mHandlers[T::Type] = [callable = std::forward<Callable>(callable)](BaseComponent& component)
        {
            callable(static_cast<T&>(component));
        };
    }

    void handle(ComponentType componentType, BaseComponent& component) const
    {
        if (mHandlers[componentType])
            mHandlers[componentType](component);
    }

private:
    std::vector<std::function<void(BaseComponent&)>> mHandlers;
};

}