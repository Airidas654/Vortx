#include "ECS/Entity.h"
#include "ECS/Registry.h"

#include <iostream>

namespace Vortx
{

	Entity Registry::Create()
	{
		return Entity(this, registry.create());
	}
}