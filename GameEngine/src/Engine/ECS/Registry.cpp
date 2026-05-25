#include "ECS/Entity.h"
#include "ECS/Registry.h"

Entity Registry::Create()
{
	return Entity(*this, registry.create());
}