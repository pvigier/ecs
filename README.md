# ecs

[![Build Status](https://travis-ci.org/pvigier/ecs.svg?branch=master)](https://travis-ci.org/pvigier/ecs)
[![codecov](https://codecov.io/gh/pvigier/ecs/branch/master/graph/badge.svg)](https://codecov.io/gh/pvigier/ecs)

`ecs` is the [entity-component-system](https://en.wikipedia.org/wiki/Entity_component_system) library I have created for my game [Vagabond](https://pvigier.github.io/tag/vagabond).

`ecs` aims to be:

* simple
* lightweight
* easy to use
* fast
* header only
* implemented with modern C++ features (C++17)

## Example

Firstly, you should fix your number of components and systems:

```cpp
constexpr auto ComponentCount = 32;
constexpr auto SystemCount = 8;
```

Then, you can define some components:

```cpp
struct Position : public Component<Position>
{
    float x;
    float y;
};

struct Velocity : public Component<Velocity>
{
    float x;
    float y;
};
```

Now, let us define a system that will operate on the components:

```cpp
class PhysicsSystem : public System<ComponentCount, SystemCount>
{
public:
    PhysicsSystem(EntityManager<ComponentCount, SystemCount>& entityManager) : mEntityManager(entityManager)
    {
        setRequirements<Position, Velocity>();
    }

    void update(float dt)
    {
        for (const auto& entity : getManagedEntities())
        {
            auto [position, velocity] = mEntityManager.getComponents<Position, Velocity>(entity);
            position.x += velocity.x * dt;
            position.y += velocity.y * dt;
        }
    }

private:
    EntityManager<ComponentCount, SystemCount>& mEntityManager;
};
```

Finally, we create an entity manager, register the components, create a system and some entities:

```cpp
auto manager = EntityManager<ComponentCount, SystemCount>();
manager.registerComponent<Position>();
manager.registerComponent<Velocity>();
auto system = manager.createSystem<PhysicsSystem>(manager);
for (auto i = 0; i < 10; ++i)
{
    auto entity = manager.createEntity();
    manager.addComponent<Position>(entity);
    manager.addComponent<Velocity>(entity);
}
auto dt = 1.0f / 60.0f;
while (true)
    system->update(dt);
```

It is that easy!

If you want more examples, look at the [examples](https://github.com/pvigier/ecs/tree/master/examples) folder.

## Documentation

I have written several articles on my blog describing the design of the library. They are available [here](https://pvigier.github.io/2019/07/07/entity-component-system-part1.html).

Otherwise, just look at the [EntityManager.h](https://github.com/pvigier/ecs/blob/master/include/ecs/EntityManager.h) file, it is simple to understand.

## License

Distributed under the MIT License.