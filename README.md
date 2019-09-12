# ecs

[![Build Status](https://travis-ci.org/pvigier/ecs.svg?branch=master)](https://travis-ci.org/pvigier/ecs)
[![codecov](https://codecov.io/gh/pvigier/ecs/branch/master/graph/badge.svg)](https://codecov.io/gh/pvigier/ecs)

`ecs` is the [entity-component-system](https://en.wikipedia.org/wiki/Entity_component_system) library I have created for my game [Vagabond](https://pvigier.github.io/tag/vagabond).

`ecs` aims to be:

* easy to use
* fast
* lightweight
* header only
* implemented with modern C++ features (C++17)

## Example

Firstly, you should define some components:

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

A component of type `T` must inherit `Component<T>`.

Now, let us create an entity manager and register the components:

```cpp
constexpr auto NbComponents = 2;
auto manager = EntityManager(NbComponents);
manager.registerComponent<Position>();
manager.registerComponent<Velocity>();
```

Next, we can create an entity set. An entity set tracks all entities that fullfill a certain requirement on components. For instance, let us create an entity set that will track all entities that have `Position` and `Velocity` components:

```cpp
manager.registerEntitySet<Position, Velocity>();
```

Let us create an entity with both components:

```cpp
auto entity = manager.createEntity();
manager.addComponent<Position>(entity);
manager.addComponent<Velocity>(entity);
```

Finally, we can use the entity set we created to find all entities that have both components and update their positions:

```cpp
auto dt = 1.0f / 60.0f;
for (auto [entity, components] : manager.getEntitySet<Position, Velocity>())
{
    auto [position, velocity] = components;
    // Update the position
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
}
```

It is that easy!

If you want more examples, look at the [examples](https://github.com/pvigier/ecs/tree/master/examples) folder.

## Documentation

I have written several articles on my blog describing the design of the library. They are available [here](https://pvigier.github.io/2019/07/07/entity-component-system-part1.html).

Otherwise, just look at the [EntityManager.h](https://github.com/pvigier/ecs/blob/master/include/ecs/EntityManager.h) file, it is simple to understand.

## License

Distributed under the MIT License.