#include <chrono>
#include "ecs/Component.h"
#include "ecs/EntityManager.h"

using namespace ecs;

constexpr auto ComponentCount = 512;

struct Position : public Component<Position>
{
    Position(float X = 0.0, float Y = 0.0) : x(X), y(Y)
    {

    }

    float x;
    float y;
};

struct Velocity : public Component<Velocity>
{
    Velocity(float X = 0.0, float Y = 0.0) : x(X), y(Y)
    {

    }

    float x;
    float y;
};

class PhysicsSystem
{
public:
    PhysicsSystem(EntityManager& entityManager) : mEntityManager(entityManager)
    {

    }

    void update(float dt)
    {
        for (auto [entity, components] : mEntityManager.getEntitySet<Position, Velocity>())
        {
            auto [position, velocity] = components;
            position.x += velocity.x * dt;
            position.y += velocity.y * dt;
        }
    }

private:
    EntityManager& mEntityManager;
};

int main()
{
    constexpr auto nbEntities = std::size_t(10000);
    constexpr auto nbUpdates = std::size_t(100);
    auto manager = EntityManager(ComponentCount);
    manager.registerComponent<Position>();
    manager.registerComponent<Velocity>();
    manager.registerEntitySet<Position, Velocity>();
    auto system = PhysicsSystem(manager);
    manager.reserve(nbEntities);
    for (auto i = std::size_t(0); i < nbEntities; ++i)
    {
        auto entity = manager.createEntity();
        manager.addComponent<Position>(entity, 0.0f, 0.0f);
        manager.addComponent<Velocity>(entity, 1.0f, 1.0f);
    }
    auto prevTime = std::chrono::system_clock::now();
    for (auto i = std::size_t(0); i < nbUpdates; ++i)
    {
        auto time = std::chrono::system_clock::now();
        auto dt = std::chrono::duration<float>(time - prevTime).count();
        prevTime = time;
        system.update(dt);
    }
    return 0;
}
