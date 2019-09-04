#include <benchmark/benchmark.h>
#include "ecs/Component.h"
#include "ecs/EntityManager.h"

using namespace ecs;

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

struct Mass : public Component<Mass>
{
    Mass(float Value = 0.0) : value(Value)
    {

    }

    float value;
};

template<typename ...Components>
class DummySystem
{
public:
    DummySystem(EntityManager& entityManager) : mEntityManager(entityManager)
    {

    }

    void update()
    {
        for (const auto& entity : mEntityManager.getEntitySet<Components...>())
            benchmark::DoNotOptimize(mEntityManager.getComponents<Components...>(entity));
    }

private:
    EntityManager& mEntityManager;
};

constexpr auto MinNbEntities = 100000;
constexpr auto MaxNbEntities = 100000;

template<std::size_t ComponentCount, std::size_t SystemCount, bool Reserve, typename ...Components>
void createEntities(benchmark::State& state)
{
    for (auto _ : state)
    {
        auto manager = EntityManager(ComponentCount, SystemCount);
        (manager.registerComponent<Components>(), ...);
        manager.registerEntitySet<Components...>();
        auto system = DummySystem<Components...>(manager);
        if constexpr (Reserve)
            manager.reserve(static_cast<std::size_t>(state.range()));
        for (auto i = 0; i < state.range(); ++i)
        {
            auto entity = manager.createEntity();
            (manager.addComponent<Components>(entity), ...);
        }
    }
    auto nbItems = static_cast<int>(state.iterations()) * state.range();
    state.SetItemsProcessed(static_cast<std::size_t>(nbItems));
    state.SetComplexityN(state.range());
}
BENCHMARK_TEMPLATE(createEntities, 512, 32, true, Position)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createEntities, 512, 32, true, Position, Velocity)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createEntities, 512, 32, true, Position, Velocity, Mass)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createEntities, 512, 32, false, Position)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createEntities, 512, 32, false, Position, Velocity)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createEntities, 512, 32, false, Position, Velocity, Mass)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);

template<std::size_t ComponentCount, std::size_t SystemCount, bool Reserve, typename ...Components>
void iterateEntities(benchmark::State& state)
{
    auto manager = EntityManager(ComponentCount, SystemCount);
    (manager.registerComponent<Components>(), ...);
    manager.registerEntitySet<Components...>();
    auto system = DummySystem<Components...>(manager);
    if constexpr (Reserve)
        manager.reserve(static_cast<std::size_t>(state.range()));
    for (auto i = 0; i < state.range(); ++i)
    {
        auto entity = manager.createEntity();
        (manager.addComponent<Components>(entity), ...);
    }
    for (auto _ : state)
        system.update();
    auto nbItems = static_cast<int>(state.iterations()) * state.range();
    state.SetItemsProcessed(static_cast<std::size_t>(nbItems));
    state.SetComplexityN(state.range());
}
BENCHMARK_TEMPLATE(iterateEntities, 512, 32, true, Position)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(iterateEntities, 512, 32, true, Position, Velocity)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(iterateEntities, 512, 32, true, Position, Velocity, Mass)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(iterateEntities, 512, 32, false, Position)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(iterateEntities, 512, 32, false, Position, Velocity)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(iterateEntities, 512, 32, false, Position, Velocity, Mass)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);

template<std::size_t ComponentCount, std::size_t SystemCount, bool Reserve, std::size_t K, typename ...Components>
void createThenRemoveEntities(benchmark::State& state)
{
    for (auto _ : state)
    {
        auto manager = EntityManager(ComponentCount, SystemCount);
        (manager.registerComponent<Components>(), ...);
        manager.registerEntitySet<Components...>();
        auto system = DummySystem<Components...>(manager);
        if constexpr (Reserve)
            manager.reserve(static_cast<std::size_t>(state.range()));
        for (auto k = std::size_t(0); k < K; ++k)
        {
            for (auto i = 0; i < state.range(); ++i)
            {
                auto entity = manager.createEntity();
                (manager.addComponent<Components>(entity), ...);
            }
            for (auto i = state.range() - 1; i >= 0; --i)
                manager.removeEntity(static_cast<Entity>(i));
        }
    }
    auto nbItems = static_cast<int>(K * state.iterations()) * state.range();
    state.SetItemsProcessed(static_cast<std::size_t>(nbItems));
    state.SetComplexityN(state.range());
}
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, true, 1, Position)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, true, 1, Position, Velocity)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, true, 1, Position, Velocity, Mass)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, false, 1, Position)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, false, 1, Position, Velocity)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, false, 1, Position, Velocity, Mass)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, true, 10, Position)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, true, 10, Position, Velocity)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, true, 10, Position, Velocity, Mass)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, false, 10, Position)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, false, 10, Position, Velocity)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);
BENCHMARK_TEMPLATE(createThenRemoveEntities, 512, 32, false, 10, Position, Velocity, Mass)->RangeMultiplier(10)->Range(MinNbEntities, MaxNbEntities);

BENCHMARK_MAIN()