#pragma once

#include "ComponentId.h"
#include "SparseSet.h"

namespace ecs
{

template<typename T>
using ComponentSparseSet = SparseSet<ComponentId, T>;

}