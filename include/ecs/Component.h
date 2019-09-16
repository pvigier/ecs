#pragma once

#include <memory>
#include "ComponentContainer.h"
#include "ComponentType.h"

namespace ecs
{

class BaseComponent
{
public:
    static std::size_t getComponentCount()
    {
        return sFactories.size();
    }

    static std::unique_ptr<BaseComponentContainer> createComponentContainer(std::size_t type)
    {
        return sFactories[type]();
    }

protected:
    template<typename T>
    static ComponentType generateComponentType()
    {
        sFactories.push_back([]() -> std::unique_ptr<BaseComponentContainer>
        {
            return std::make_unique<ComponentContainer<T>>();
        });
        return static_cast<ComponentType>(sFactories.size() - 1);
    }

private:
    using ComponentContainerFactory = std::unique_ptr<BaseComponentContainer>(*)();

    static std::vector<ComponentContainerFactory> sFactories;
};

inline std::vector<BaseComponent::ComponentContainerFactory> BaseComponent::sFactories;

template<typename T>
class Component : private BaseComponent
{
public:
    static const ComponentType Type;
};

template<typename T>
const ComponentType Component<T>::Type = BaseComponent::generateComponentType<T>();

}