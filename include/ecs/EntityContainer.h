#pragma once

#include <unordered_map>
#include "ComponentId.h"
#include "ComponentType.h"
#include "Entity.h"
#include "SparseSet.h"


namespace ecs
{

using EntityContainer = SparseSet<Entity, std::unordered_map<ComponentType, ComponentId>>;

}