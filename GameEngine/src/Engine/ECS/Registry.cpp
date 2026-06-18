#include "ECS/Entity.h"
#include "ECS/Registry.h"

#include <iostream>

Entity Registry::Create()
{
	return Entity(this, registry.create());
}